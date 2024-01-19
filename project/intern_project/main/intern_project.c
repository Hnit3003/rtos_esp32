#include <stdio.h>
#include "GPIO_Driver.h"
#include "Wifi_Driver.h"
#include "EXTI_Driver.h"
#include "HTTP_Driver.h"
#include "ADC_Driver.h"

static const char *TAG = "wi-fi station";

extern uint32_t voltage;

void app_main(void)
{


    printf("VOLTAGE = %d", voltage);
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_station();

    http_start_webserver();

    adc_initialize();

    // while(1)
    // {
    //     ESP_LOGI(TAG, "ADC1_CHANNEL_5: %d mV", voltage);
    //     vTaskDelay(pdMS_TO_TICKS(1000));
    // }
}
