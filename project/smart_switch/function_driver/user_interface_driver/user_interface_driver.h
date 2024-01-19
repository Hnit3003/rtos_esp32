#ifndef USER_INTERFACE_DRIVER_H
#define USER_INTERFACE_DRIVER_H

#include <tcpip_adapter.h>
#include <stdio.h>
#include "esp_log.h"
#include "wifi_smartconfig_driver.h"
#include "ssd1306_driver.h"
#include "relay_control_driver.h"

typedef enum 
{
    MENU,
    DISPLAY,
    MENU_CHOOSED
} system_state_t;

typedef enum
{
    SMARTCONFIG,
    CONNECT,
    DISCONNECT,
    EXIT_MENU
} menu_option_t;

typedef enum
{
    LOAD_1,
    LOAD_2,
    LOAD_3,
    GOTO_MENU
} display_option_t;

void user_interface_init(void);
void button_up_handle(void);
void button_enter_handle(void);
void button_down_handle(void);
void SSID_display();
void USRIP_display(char * ip_address);
void load_1_display(bool invert);
void load_2_display(bool invert);
void load_3_display(bool invert);
void main_display_update();


#endif
