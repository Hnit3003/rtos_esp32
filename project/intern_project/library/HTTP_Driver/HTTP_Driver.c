#include "HTTP_Driver.h"

#define RELAY1 GPIO_NUM_4
#define RELAY2 GPIO_NUM_17
#define RELAY3 GPIO_NUM_21

#define SW_OFF 1
#define SW_ON 0

static const char *TAG = "HTTP_Server";
static httpd_handle_t http_server = NULL;

extern uint32_t voltage;

static xQueueHandle xQueue_SwitchPost;

enum SwitchID_State{SW1_ON, SW1_OFF, SW2_ON, SW2_OFF, SW3_ON, SW3_OFF};
enum SwitchID_State SWID_St;

void http_switch_post_task(void *pvParameter)
{
    enum SwitchID_State SWID_St_Task;

    while(1)
    {
        xQueueReceive(xQueue_SwitchPost, &SWID_St_Task, portMAX_DELAY);
        switch (SWID_St_Task)
        {
        case SW1_OFF:
            printf("SW1 OFF\n");
            gpio_set_level(RELAY1, SW_OFF);
            break;

        case SW1_ON:
            printf("SW1 ON\n");
            gpio_set_level(RELAY1, SW_ON);
            break;
        
        case SW2_OFF:
            printf("SW2 OFF\n");
            gpio_set_level(RELAY2, SW_OFF);
            break;

        case SW2_ON:
            printf("SW2 ON\n");
            gpio_set_level(RELAY2, SW_ON);
            break;

        case SW3_OFF:
            printf("SW3 OFF\n");
            gpio_set_level(RELAY3, SW_OFF);
            break;

        case SW3_ON:
            printf("SW3 ON\n");
            gpio_set_level(RELAY3, SW_ON);
            break;

        default:
            break;
        }
    }
}

/*---404 NOT FOUND---*/
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    if(strcmp("/dashboard", req->uri) == 0)
    {
        httpd_resp_send_err (req, HTTPD_404_NOT_FOUND, "DASHBOARD URI IS NOT AVAILABLE");
        /* Return ESP_OK to keep underlying socket open */
        return ESP_OK;
    }
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "404 NOT FOUND");
    return ESP_FAIL;
}

/*---DASHBOARD---*/
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[]  asm("_binary_index_html_end");

static esp_err_t dashboard_display(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);
    return ESP_OK;
}

static const httpd_uri_t get_dashboard_display = {
    .uri = "/dashboard",
    .method = HTTP_GET,
    .handler = dashboard_display,
    .user_ctx = NULL
};

/*---SWITCH---*/
// Switch 1
static esp_err_t switch1_handler(httpd_req_t *req)
{
    char buf[5];
    httpd_req_recv(req, buf, req->content_len);
    printf("SW1 change state: %s\n", buf);
    if (buf[0] == '1')
    {
        SWID_St = SW1_ON;
        xQueueSend(xQueue_SwitchPost, &SWID_St, portMAX_DELAY);
    }else if (buf[0] == '0')
    {
        SWID_St = SW1_OFF;
        xQueueSend(xQueue_SwitchPost, &SWID_St, portMAX_DELAY);
    }
    
    return ESP_OK;
}

static const httpd_uri_t post_switch1 = {
    .uri = "/switch1",
    .method = HTTP_POST,
    .handler = switch1_handler,
    .user_ctx = NULL
};

// Switch 2
static esp_err_t switch2_handler(httpd_req_t *req)
{
    char buf[5];
    httpd_req_recv(req, buf, req->content_len);
    printf("SW2 change state: %s\n", buf);

    if (buf[0] == '1')
    {
        SWID_St = SW2_ON;
        xQueueSend(xQueue_SwitchPost, &SWID_St, portMAX_DELAY);
    }else if (buf[0] == '0')
    {
        SWID_St = SW2_OFF;
        xQueueSend(xQueue_SwitchPost, &SWID_St, portMAX_DELAY);
    }

    return ESP_OK;
}

static const httpd_uri_t post_switch2 = {
    .uri = "/switch2",
    .method = HTTP_POST,
    .handler = switch2_handler,
    .user_ctx = NULL
};

// Switch 3
static esp_err_t switch3_handler(httpd_req_t *req)
{
    char buf[5];
    httpd_req_recv(req, buf, req->content_len);
    printf("SW3 change state: %s\n", buf);

    if (buf[0] == '1')
    {
        SWID_St = SW3_ON;
        xQueueSend(xQueue_SwitchPost, &SWID_St, portMAX_DELAY);
    }else if (buf[0] == '0')
    {
        SWID_St = SW3_OFF;
        xQueueSend(xQueue_SwitchPost, &SWID_St, portMAX_DELAY);
    }

    return ESP_OK;
}

static const httpd_uri_t post_switch3 = {
    .uri = "/switch3",
    .method = HTTP_POST,
    .handler = switch3_handler,
    .user_ctx = NULL
};

/*---ADC---*/

static esp_err_t adc_handler(httpd_req_t *req)
{
    uint32_t current = (voltage - 1250) * 20;
    char json_keyvalue[100];
    sprintf(json_keyvalue, "{\"voltage_value\": \"%d\", \"current_value\": \"%d\", \"current_number\": %d}", voltage, current, current);
    const char* resp_str = (const char*) json_keyvalue;
    httpd_resp_send(req, resp_str, strlen(resp_str));

    return ESP_OK;
}

static const httpd_uri_t get_adc = {
    .uri = "/adc",
    .method = HTTP_GET,
    .handler = adc_handler,
    .user_ctx = NULL
};

void http_start_webserver(void)
{
    httpd_config_t http_conf = HTTPD_DEFAULT_CONFIG();
    // allow the http server to close our socket if needed
    http_conf.lru_purge_enable = true;

    ESP_LOGI(TAG, "Starting server on port: '%d'", http_conf.server_port);
    if(httpd_start(&http_server, &http_conf) == ESP_OK)
    {
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_err_handler(http_server, HTTPD_404_NOT_FOUND, http_404_error_handler);
        httpd_register_uri_handler(http_server, &get_dashboard_display);
        httpd_register_uri_handler(http_server, &post_switch1);
        httpd_register_uri_handler(http_server, &post_switch2);
        httpd_register_uri_handler(http_server, &post_switch3);
        httpd_register_uri_handler(http_server, &get_adc);
    }else {
        ESP_LOGI(TAG, "Error starting server!");
    }
    xQueue_SwitchPost = xQueueCreate(10, sizeof(uint32_t));

    TaskHandle_t xHTTP_SWITCH = NULL;
    xTaskCreate(http_switch_post_task, "http_switch_post_task", 2048, NULL, 3, &xHTTP_SWITCH);

    gpio_set_output(RELAY1);
    gpio_set_level(RELAY1, 1);
    gpio_set_output(RELAY2);
    gpio_set_level(RELAY2, 1);
    gpio_set_output(RELAY3);
    gpio_set_level(RELAY3, 1);

}

void http_stop_webserver(void)
{
    //Stop the http server
    httpd_stop(http_server);
}