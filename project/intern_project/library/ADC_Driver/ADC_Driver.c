#include "ADC_Driver.h"

#define SAMPLING_TIMES  100

static const char *TAG = "ADC";

static esp_adc_cal_characteristics_t adc1_chars;

void adc_sampling_task(void *pvParameter)
{
    uint8_t countup = 0;
    uint32_t voltage_sum = 0;

    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);

    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11));

    while (1) 
    {
        uint32_t voltage_buffer = esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_5), &adc1_chars);
        if (countup <= SAMPLING_TIMES)
        {
            voltage_sum += voltage_buffer;
            countup++;
        }else
        {
            voltage = (uint32_t) (voltage_sum/SAMPLING_TIMES) - voltage_calib;
            voltage_sum = 0;
            countup = 0; 
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void adc_initialize(void)
{
    TaskHandle_t xADC_TASK = NULL;

    xTaskCreate(adc_sampling_task, "adc_sampling_task", 2048, NULL, 1, &xADC_TASK);

    uint32_t voltage_calib_sum = 0;

    for(uint8_t i = 0; i<20; i++)
    {
        vTaskDelay(510);

        voltage_calib_sum += (voltage - 1250);
        ESP_LOGI(TAG, "Voltage = %d\n", voltage);
    }

    voltage_calib = (uint32_t) (voltage_calib_sum/20);
    
}