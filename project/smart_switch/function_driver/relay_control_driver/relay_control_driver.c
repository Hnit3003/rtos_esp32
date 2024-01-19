#include "relay_control_driver.h"

static const char * TAG = "relay_control";

uint8_t relay1_state = RELAY_OFF;
uint8_t relay2_state = RELAY_OFF;
uint8_t relay3_state = RELAY_OFF;
uint8_t relay4_state = RELAY_OFF;

void relay_init()
{
    gpio_set_output(RELAY_1);
    gpio_set_output(RELAY_2);
    gpio_set_output(RELAY_3);
    gpio_set_output(RELAY_4);

    gpio_set_level(RELAY_1, (uint32_t)RELAY_OFF);
    gpio_set_level(RELAY_2, (uint32_t)RELAY_OFF);
    gpio_set_level(RELAY_3, (uint32_t)RELAY_OFF);
    gpio_set_level(RELAY_4, (uint32_t)RELAY_OFF);

//    external_interrupt_initialize(BUTTON_1, GPIO_INTR_ANYEDGE);
    external_interrupt_initialize(BUTTON_2, GPIO_INTR_ANYEDGE);
    external_interrupt_initialize(BUTTON_3, GPIO_INTR_ANYEDGE);
    external_interrupt_initialize(BUTTON_4, GPIO_INTR_ANYEDGE);
}

void relay_change_state(gpio_num_t relay_num)
{
    switch (relay_num)
    {
    case RELAY_1:
        ESP_LOGI(TAG, "RELAY 1 STATE CHANGE");
        relay1_state = !relay1_state;
        gpio_set_level(RELAY_1, relay1_state);
        break;

    case RELAY_2:
        ESP_LOGI(TAG, "RELAY 2 STATE CHANGE");
        relay2_state = !relay2_state;
        gpio_set_level(RELAY_2, relay2_state);
        break;

    case RELAY_3:
        ESP_LOGI(TAG, "RELAY 3 STATE CHANGE");
        relay3_state = !relay3_state;
        gpio_set_level(RELAY_3, relay3_state);
        break;

    case RELAY_4:
        ESP_LOGI(TAG, "RELAY 4 STATE CHANGE");
        relay4_state = !relay4_state;
        gpio_set_level(RELAY_4, relay4_state);
        break;

    default:
        break;
    }
}

uint32_t relay_get_state(gpio_num_t relay_num)
{
    return gpio_get_level(relay_num);
}