#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "gpio_subdriver.h"
#include "gptimer_subdriver.h"
#include "exti_subdriver.h"
#include "adc_subdriver.h"
#include "wifi_subdriver.h"

// static void IRAM_ATTR external_interrupt_handler(void *args);

uint16_t count_button1;
uint16_t count_button2;
uint16_t count_button3;

void FallingEdge(gpio_num_t pinNumber)
{
    switch (pinNumber)
    {
    case GPIO_NUM_5:
        printf("Button %d pressed -- %d times\n", pinNumber, count_button1++);
        break;
    
    case GPIO_NUM_18:
        printf("Button %d pressed -- %d times\n", pinNumber, count_button2++);
        break;

    case GPIO_NUM_19:
        printf("Button %d pressed -- %d times\n", pinNumber, count_button3++);
        break;

    default:
        break;
    }
}

void RisingEdge(gpio_num_t pinNumber)
{
    switch (pinNumber)
    {
    case GPIO_NUM_5:
        printf("Button %d release\n", pinNumber);
        break;
    case GPIO_NUM_18:
        break;
    case GPIO_NUM_19:
        break;
    default:
        break;
    }
}

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    EXTI_InitFunction(RisingEdge, FallingEdge);
    external_interrupt_initialize(GPIO_NUM_5, GPIO_INTR_ANYEDGE);
    external_interrupt_initialize(GPIO_NUM_18, GPIO_INTR_NEGEDGE);
    external_interrupt_initialize(GPIO_NUM_19, GPIO_INTR_NEGEDGE);
    adc_initialize();
    wifi_init_station();
}
