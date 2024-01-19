#include "adc_subdriver.h"

static const char *TAG = "ADC";

static esp_adc_cal_characteristics_t adc1_chars;

void adc_sampling_task(void *pvParameter)
{
    uint32_t voltage;

    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);

    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11));

    while (1) 
    {
        voltage = esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_5), &adc1_chars);
        // ESP_LOGI(TAG, "ADC1_CHANNEL_5: %d mV", voltage);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void adc_initialize(void)
{
    TaskHandle_t xADC_TASK = NULL;

    xTaskCreate(adc_sampling_task, "adc_sampling_task", 2048, NULL, 2, &xADC_TASK);
}