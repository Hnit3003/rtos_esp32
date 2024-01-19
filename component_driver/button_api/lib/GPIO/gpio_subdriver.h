#ifndef GPIO_H
#define GPIO_H

#include "driver/gpio.h"

void gpio_set_input(gpio_num_t gpio_pin);
void gpio_set_output(gpio_num_t gpio_pin);

#endif