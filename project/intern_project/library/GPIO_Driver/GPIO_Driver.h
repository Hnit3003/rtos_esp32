#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#include "driver/gpio.h"

void gpio_set_output(gpio_num_t gpio_pin);
void gpio_set_output_opendrain(gpio_num_t gpio_pin);
void gpio_set_input(gpio_num_t gpio_pin);
void gpio_set_input_output(gpio_num_t gpio_pin);
void gpio_set_input_output_opendrain(gpio_num_t gpio_pin);
void gpio_set_input_pullup(gpio_num_t gpio_pin);
void gpio_set_input_pulldown(gpio_num_t gpio_pin);

#endif
