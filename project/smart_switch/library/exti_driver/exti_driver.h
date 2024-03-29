#ifndef EXTI_DRIVER_H
#define EXTI_DRIVER_H

#include "driver/gpio.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "stdbool.h"

// static void (*EXTI_RisingDectect_Handle)   (gpio_num_t EXTI_Pin);
// static void (*EXTI_FallingDectect_Handle)  (gpio_num_t EXTI_Pin);

void exti_initFunction                  (void (*RisingEdgeDectect_Handle) (gpio_num_t EXTI_Pin), 
                                         void (*FallingEdgeDectect_Handle) (gpio_num_t EXTI_Pin));

// static void IRAM_ATTR external_interrupt_handler(void *args);

void external_interrupt_task            (void *pvParameter);
void external_interrupt_initialize      (gpio_num_t gpio_pin, gpio_int_type_t intr_edge);

#endif
