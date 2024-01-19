#include <stdio.h>
#include "string.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
// #include "esp_wpa2.h"
#include "nvs_flash.h"

#include "gpio_driver.h"
#include "exti_driver.h"
#include "wifi_smartconfig_driver.h"
#include "relay_control_driver.h"
#include "user_interface_driver.h"
#include "ssd1306_driver.h"

static const char * TAG = "main";

SSD1306_t main_display;

void buttonDetect_handle(gpio_num_t button_pin);
void buttonRelease_handle(gpio_num_t button_pin);

void app_main(void)
{
    /* Initialise NVS */
    nvs_flash_init();
    
    /* RELAY */
    relay_init();

    /* EXTI */
    exti_initFunction(buttonRelease_handle, buttonDetect_handle);    
    external_interrupt_initialize(BUTTON_UP, GPIO_INTR_ANYEDGE);
    external_interrupt_initialize(BUTTON_ENTER, GPIO_INTR_ANYEDGE);
    external_interrupt_initialize(BUTTON_DOWN, GPIO_INTR_ANYEDGE);

    /* WIFI_SMARTCONFIG INIT */
    initialise_wifi_smartconfig();
    user_interface_init();
	// ssd1306_init(&main_display, 128, 64);
    // ssd1306_clear_screen(&main_display, false);
    // ssd1306_contrast(&main_display, 0xFF);
    // ssd1306_display_text_x3(&main_display, 0, "Smart config", 12, false);
    // ssd1306_display_text(&main_display, 2, "Connecting...", 13, true);
    external_interrupt_initialize(BUTTON_1, GPIO_INTR_ANYEDGE);

}

void buttonDetect_handle(gpio_num_t button_pin)
{
        switch (button_pin)
        {
            case BUTTON_1:
                printf("BUTTON 1 pressed\n");
                relay_change_state(RELAY_1);

                break;
            
            case BUTTON_2:
                printf("BUTTON 2 pressed\n");
                relay_change_state(RELAY_2);

                break;

            case BUTTON_3:
                printf("BUTTON 3 pressed\n");
                relay_change_state(RELAY_3);

                break;

            case BUTTON_4:
                printf("BUTTON 4 pressed\n");
                relay_change_state(RELAY_4);

                break;

            case BUTTON_UP:
                printf("BUTTON UP pressed\n");

                break;

                case BUTTON_ENTER:
                printf("BUTTON ENTER pressed\n");

                break;

            case BUTTON_DOWN:
                printf("BUTTON DOWN pressed\n");

                break;

            default:
                break;
        }
}

void buttonRelease_handle(gpio_num_t button_pin)
{
        switch (button_pin)
        {
        case BUTTON_1:
            printf("BUTTON 1 release\n");

            break;
        
        case BUTTON_2:
            printf("BUTTON 2 release\n");

            break;

        case BUTTON_3:
            printf("BUTTON 3 release\n");

            break;

        case BUTTON_4:
            printf("BUTTON 4 release\n");

            break;

        case BUTTON_UP:
            printf("BUTTON UP release\n");
            button_up_handle();
            break;

        case BUTTON_ENTER:
            printf("BUTTON ENTER release\n");
            button_enter_handle();

            break;

        case BUTTON_DOWN:
            printf("BUTTON DOWN release\n");
            button_down_handle();

            break;

        default:
            break;
        }
}