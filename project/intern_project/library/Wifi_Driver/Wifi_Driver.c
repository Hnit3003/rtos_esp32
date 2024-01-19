#include "Wifi_Driver.h"

#define WIFI_MAXIMUM_RETRY_CONNECT 1
static uint8_t retry_connect_count = 0;
enum WIFI_ID{ID1, ID2, ID3, CONNECTED};
enum WIFI_ID wifi_num = ID1;

/*---WIFI CONFIG---*/
#define WIFI_SSID1      "TinhChau"
#define WIFI_PASSWORD1  "30032003"
#define WIFI_SSID2      "TP-Link_1F00"
#define WIFI_PASSWORD2  "43286669"
#define WIFI_SSID3      "PIF_CLUB"
#define WIFI_PASSWORD3  "chinsochin"
    
/*---EVENTGROUP BIT---*/
#define WIFI_CONNECTED_BIT  BIT0
#define WIFI_FAIL_BIT       BIT1
static EventGroupHandle_t   wifi_eventgroup;

static const char *TAG = "wi-fi station";

static void wifi_event_handler(void *arg, esp_event_base_t event_base, 
                                int32_t event_id, void* event_data)
{
    if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        esp_wifi_connect();
    }else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if(retry_connect_count < WIFI_MAXIMUM_RETRY_CONNECT)
        {
            esp_wifi_connect();
            retry_connect_count++;
            ESP_LOGI(TAG, "retry to connect to the Access Point");
        }else
        {
            retry_connect_count = 0;
            xEventGroupSetBits(wifi_eventgroup, WIFI_FAIL_BIT);
            xEventGroupClearBits(wifi_eventgroup, WIFI_FAIL_BIT);
        }

    }else if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        retry_connect_count = 0;
        xEventGroupSetBits(wifi_eventgroup, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_station(void)
{
    wifi_eventgroup = xEventGroupCreate();

    // s1.1: The main task calls esp_netif_init() to create an LwIP core task and initialize LwIP-related work.
    esp_netif_init();

    // s1.2: The main task calls esp_event_loop_create() to create a system Event task and initialize an application event’s callback function.
    // In the scenario above, the application event’s callback function does nothing but relaying the event to the application task.
    esp_event_loop_create_default();

    // s1.3: The main task calls esp_netif_create_default_wifi_ap() or esp_netif_create_default_wifi_sta() 
    // to create default network interface instance binding station or AP with TCP/IP stack.
    // esp_netif_create_default_wifi_ap();
    esp_netif_create_default_wifi_sta();

    // s1.4: The main task calls esp_wifi_init() to create the Wi-Fi driver task and initialize the Wi-Fi driver.
    wifi_init_config_t wifi_init_conf = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_init_conf);

    /*---Wi-fi eventgroup---*/
    // Register wi-fi event and ip event
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    esp_event_handler_instance_register(WIFI_EVENT, 
                                        ESP_EVENT_ANY_ID, 
                                        &wifi_event_handler, 
                                        NULL, 
                                        &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT, 
                                        IP_EVENT_STA_GOT_IP, 
                                        &wifi_event_handler, 
                                        NULL, 
                                        &instance_got_ip);

    /*---Wi-Fi Configuration Phase---*/
    // Configure the Wi-Fi mode as station
    esp_wifi_set_mode(WIFI_MODE_STA);

    wifi_config_t wifi_conf = {
        // Config Station
        .sta = {
            .ssid = WIFI_SSID1,
            .password = WIFI_PASSWORD1,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            // Configuration for Protected Management Frame. Will be advertised in RSN Capabilities in RSN IE.
            .pmf_cfg = {
                .capable = true,
                .required = false
            }
        }
    };

    esp_wifi_set_config(WIFI_IF_STA, &wifi_conf);
    esp_wifi_start();
    ESP_LOGI(TAG, "wifi_init_station finished.");

    while(1)
    {
        /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
        * number of re-tries (WIFI_FAIL_BIT). The bits are set by wifi_event_handler() */
        EventBits_t bits = xEventGroupWaitBits(wifi_eventgroup,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);
        if(bits & WIFI_CONNECTED_BIT) 
        {
            switch (wifi_num)
            {
            case ID1:
                ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                WIFI_SSID1, WIFI_PASSWORD1);
                break;

            case ID2:
                ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                WIFI_SSID2, WIFI_PASSWORD2);
                break;
            
            case ID3:
                ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                WIFI_SSID3, WIFI_PASSWORD3);
                break;
            
            default:
                break;
            }

            wifi_num = CONNECTED;

        }else if(bits & WIFI_FAIL_BIT) 
        {
            switch (wifi_num)
            {
            case ID1:
                ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                WIFI_SSID1, WIFI_PASSWORD1);
                wifi_num = ID2;

                // wifi_conf.sta.ssid = WIFI_SSID2;
                // wifi_conf.sta.password = WIFI_PASSWORD2;

                wifi_config_t wifi_conf2 = {
                // Config Station
                    .sta = {
                        .ssid = WIFI_SSID2,
                        .password = WIFI_PASSWORD2,
                            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
                        // Configuration for Protected Management Frame. Will be advertised in RSN Capabilities in RSN IE.
                        .pmf_cfg = {
                            .capable = true,
                            .required = false
                        }
                    }
                };

                esp_wifi_set_config(WIFI_IF_STA, &wifi_conf2);
                esp_wifi_connect();

                break;

            case ID2:
                ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                WIFI_SSID2, WIFI_PASSWORD2);
                wifi_num = ID3;

                // wifi_conf.sta.ssid =  WIFI_SSID3;
                // wifi_conf.sta.password = WIFI_PASSWORD3;
                
                wifi_config_t wifi_conf3 = {
                // Config Station
                    .sta = {
                        .ssid = WIFI_SSID3,
                        .password = WIFI_PASSWORD3,
                        .threshold.authmode = WIFI_AUTH_WPA2_PSK,
                        // Configuration for Protected Management Frame. Will be advertised in RSN Capabilities in RSN IE.
                        .pmf_cfg = {
                            .capable = true,
                            .required = false
                        }
                    }
                };

                esp_wifi_set_config(WIFI_IF_STA, &wifi_conf3);
                esp_wifi_connect();

                break;

            case ID3:
                ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                WIFI_SSID3, WIFI_PASSWORD3);
                wifi_num = ID1;

                break;
            
            default:
                ESP_LOGE(TAG, "UNEXPECTED EVENT");
                break;
            }
        } else 
        {
            ESP_LOGE(TAG, "UNEXPECTED EVENT");
        }

        if(wifi_num == CONNECTED) 
        {
            break;
        }
    }
}
