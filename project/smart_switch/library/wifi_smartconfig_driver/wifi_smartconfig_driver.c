#include "wifi_smartconfig_driver.h"

wifi_config_t wifi_config_current;

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;
static const char *TAG = "smartconfig_driver";

/* Smartconfig initial task prototype */
static void smartconfig_task(void* param);

void smartconfig_start()
{
    esp_smartconfig_set_type(SC_TYPE_ESPTOUCH);
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    esp_smartconfig_start(&cfg);
}

/* Wifi Smartconfig event handler */
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    /* After WIFI start -> Create smartconfig task */
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        xTaskCreate(smartconfig_task, "smartconfig_task", 4096, NULL, 1, NULL);
        smartconfig_start();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        http_stop_webserver();
        /* Re-Connect wifi if disconnected */
        ESP_LOGI(TAG, "Wifi disconnected");
        esp_smartconfig_stop();
        xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        /* Got IP Address succsesfully */
        xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
        const ip_event_got_ip_t * event = (const ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG, "my sta ip: " IPSTR ", my mask: " IPSTR ", my gw: " IPSTR,
            IP2STR(&event->ip_info.ip),
            IP2STR(&event->ip_info.netmask),
            IP2STR(&event->ip_info.gw));

        SSID_display();

        char IP_address[20] = {0};
        sprintf(IP_address, " "IPSTR, IP2STR(&event->ip_info.ip));
        USRIP_display((char *)IP_address);

        http_start_webserver();

    } else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE) 
    {
        /* Smartconfig scan channel done */
        ESP_LOGI(TAG, "Scan done");
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL) 
    {
        /* Smartconfig found channel */
        ESP_LOGI(TAG, "Found channel");
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) 
    {
        /* Smartconfig got SSID and PSWD */
        ESP_LOGI(TAG, "Got SSID and password");

        /* Init wifi_config structure */
        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t wifi_config;

        /* Copy ssid and password to wifi_config */
        bzero(&wifi_config, sizeof(wifi_config_t));
        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
        
        /* Check bssid whether or not */
        wifi_config.sta.bssid_set = evt->bssid_set;
        if (wifi_config.sta.bssid_set == true)
        {
            memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
        }

        /* Print SSID and PASSWORD into monitor */
        uint8_t ssid[33] = { 0 };   
        uint8_t password[65] = { 0 };
        uint8_t rvd_data[33] = { 0 };
        memcpy(ssid, evt->ssid, sizeof(evt->ssid));
        memcpy(password, evt->password, sizeof(evt->password));
        ESP_LOGI(TAG, "SSID:%s", ssid);
        ESP_LOGI(TAG, "PASSWORD:%s", password);
        
        /* Check ESPTOUCH_V2 whether or not */
        if (evt->type == SC_TYPE_ESPTOUCH_V2) 
        {
            ESP_ERROR_CHECK( esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)) );
            ESP_LOGI(TAG, "RVD_DATA:");
            for (int i=0; i<33; i++) 
            {
                printf("%02x ", rvd_data[i]);
            }
            printf("\n");
        }

        /* Re-Connect wifi with SSID and PASSWORD received by smartconfig */
        esp_wifi_disconnect();
        esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
        esp_wifi_connect();

        wifi_config_current = wifi_config;
        // smartconfig_display();
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) 
    {
        /* Smart config done */
        xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
    }
}

static void smartconfig_task(void * parm)
{
    EventBits_t uxBits;
    while (1) 
    {
        uxBits = xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
        if(uxBits & CONNECTED_BIT) 
        {
            ESP_LOGI(TAG, "WiFi Connected to ap");
        }
        if(uxBits & ESPTOUCH_DONE_BIT) 
        {
            ESP_LOGI(TAG, "smartconfig over");
            esp_smartconfig_stop();
            xEventGroupClearBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
            // vTaskDelete(NULL);
        }
    }
}

void initialise_wifi_smartconfig(void)
{
    s_wifi_event_group = xEventGroupCreate();
    /* s1.1: The main task calls esp_netif_init() to create an LwIP core task and initialize LwIP-related work */ 
    esp_netif_init();

    /* s1.2: The main task calls esp_event_loop_create() to create a system Event task and initialize an application event’s callback function.
    In the scenario above, the application event’s callback function does nothing but relaying the event to the application task. */
    esp_event_loop_create_default();

    /* s1.3: The main task calls esp_netif_create_default_wifi_ap() or esp_netif_create_default_wifi_sta() 
    to create default network interface instance binding station or AP with TCP/IP stack */
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    /* s1.4: The main task calls esp_wifi_init() to create the Wi-Fi driver task and initialize the Wi-Fi driver */ 
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    /* Wifi and Smartconfig event handler */
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL);
    esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL);
    
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
}