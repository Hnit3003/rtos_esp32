#ifndef WIFI_SMARTCONFIG_DRIVER_H
#define WIFI_SMARTCONFIG_DRIVER_H

#include <string.h>
#include <stdlib.h>
#include <tcpip_adapter.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_netif.h"
#include "esp_smartconfig.h"
#include "user_interface_driver.h"
#include "http_driver.h"

void initialise_wifi_smartconfig(void);
void smartconfig_start();

#endif
