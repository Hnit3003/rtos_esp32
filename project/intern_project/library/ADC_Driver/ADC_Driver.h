#ifndef ADC_DRIVER_H
#define ADC_DRIVER_H

#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

uint32_t voltage;
uint32_t voltage_calib;

void adc_initialize(void);

#endif
