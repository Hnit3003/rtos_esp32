#ifndef RELAY_CONTROL_DRIVER_H
#define RELAY_CONTROL_DRIVER_H

// #define RELAY_1         GPIO_NUM_4
// #define RELAY_2         GPIO_NUM_17
// #define RELAY_3         GPIO_NUM_21
// #define RELAY_4         GPIO_NUM_25

#define RELAY_1         GPIO_NUM_21
#define RELAY_2         GPIO_NUM_19
#define RELAY_3         GPIO_NUM_18
#define RELAY_4         GPIO_NUM_4

#define RELAY_ON        (0)
#define RELAY_OFF       (1)
    
#define BUTTON_1        GPIO_NUM_34
#define BUTTON_2        GPIO_NUM_35
#define BUTTON_3        GPIO_NUM_32
#define BUTTON_4        GPIO_NUM_33

// #define BUTTON_UP       GPIO_NUM_5
// #define BUTTON_ENTER    GPIO_NUM_26
// #define BUTTON_DOWN     GPIO_NUM_27

#define BUTTON_UP       GPIO_NUM_5
#define BUTTON_ENTER    GPIO_NUM_22
#define BUTTON_DOWN     GPIO_NUM_23

#include "driver/gpio.h"
#include "esp_log.h"
#include "gpio_driver.h"
#include "exti_driver.h"

void relay_init();
void relay_change_state(gpio_num_t relay_pin);
uint32_t relay_get_state(gpio_num_t relay_num);

#endif