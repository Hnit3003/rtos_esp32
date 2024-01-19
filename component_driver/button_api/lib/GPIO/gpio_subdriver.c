#include "gpio_subdriver.h"

void gpio_set_input(gpio_num_t gpio_pin)
{
    gpio_config_t pin_conf;
    pin_conf.pin_bit_mask = 1ULL<<gpio_pin;
    pin_conf.mode = GPIO_MODE_INPUT;
    pin_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    pin_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    pin_conf.intr_type = GPIO_INTR_DISABLE;

    gpio_config(&pin_conf);   
}

void gpio_set_output(gpio_num_t gpio_pin)
{
    gpio_config_t pin_conf;
    pin_conf.pin_bit_mask = 1ULL<<gpio_pin;
    pin_conf.mode = GPIO_MODE_OUTPUT;
    pin_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    pin_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    pin_conf.intr_type = GPIO_INTR_DISABLE;

    gpio_config(&pin_conf);   
}