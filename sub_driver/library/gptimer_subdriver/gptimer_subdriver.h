#ifndef GPTIMER_DRIVER_H
#define EGPTIMER_DRIVER_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/timer.h"
#include "driver/gpio.h"

void (*EXTI_RisingEdgeDectect_Handle) (gpio_num_t EXTI_Pin);
void (*EXTI_FallingEdgeDectect_Handle) (gpio_num_t EXTI_Pin);
void EXTI_InitFunction( void (*RisingEdgeDectect_Handle) (gpio_num_t EXTI_Pin), 
                        void (*FallingEdgeDectect_Handle) (gpio_num_t EXTI_Pin) );
static void IRAM_ATTR external_interrupt_handler(void *args);

void external_interrupt_task(void *pvParameter);
void external_interrupt_initialize(gpio_num_t gpio_pin, gpio_int_type_t intr_edge);


#endif
