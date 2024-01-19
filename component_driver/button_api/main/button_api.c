#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "gpio_subdriver.h"

void app_main(void)
{
    uint32_t time_last = 0;
    int gpio_lastState = 0;
    int gpio_currentState = 0;

    uint8_t is_debouncing = 0;

    gpio_set_input(GPIO_NUM_25);
    gpio_set_output(GPIO_NUM_2);

    while(1)
    {
        gpio_currentState = gpio_get_level(GPIO_NUM_25);
        if(gpio_currentState != gpio_lastState)
        {
            if(is_debouncing == 0)
            {
                time_last = xTaskGetTickCount()*portTICK_PERIOD_MS;
                is_debouncing = 1;
            }

            if((xTaskGetTickCount()*portTICK_PERIOD_MS - time_last >= 20) && (is_debouncing == 1)) 
            {
                if(gpio_currentState == 0)
                {
                    gpio_set_level(GPIO_NUM_2, 1);
                    printf("Pressing\n");
                }else
                {
                    gpio_set_level(GPIO_NUM_2, 0);
                    printf("Release\n");
                }

                is_debouncing = 0;
                gpio_lastState = gpio_currentState;
            }
        }
    }
}

