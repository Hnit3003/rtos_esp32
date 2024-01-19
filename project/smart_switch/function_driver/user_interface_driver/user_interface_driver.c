#include "user_interface_driver.h"

#define TXTLENGHT(string)       (sizeof(string) - 1)

extern SSD1306_t main_display;
extern wifi_config_t wifi_config_current;

extern uint8_t relay1_state;
extern uint8_t relay2_state;
extern uint8_t relay3_state;
extern uint8_t relay4_state;

static char * ip_address_current = NULL;

static const char * TAG = "user_interface";

system_state_t system_state = DISPLAY;
menu_option_t menu_option = SMARTCONFIG;
display_option_t display_option = LOAD_1;

static EventGroupHandle_t s_usrinterface_event_group;

static const int STATECHANGE_BIT = BIT2;
static const int MENUCHOOSE_BIT = BIT3;
static const int MENUPOINTOPT_BIT = BIT4;
static const int DISPLAYPOINTOPT_BIT = BIT5;

void main_display_update()
{
    xEventGroupSetBits(s_usrinterface_event_group, DISPLAYPOINTOPT_BIT);
}

void load_1_display(bool invert)
{
    if(system_state == DISPLAY)
    {
        if (relay1_state == RELAY_OFF)
        {
            ssd1306_display_text(&main_display, 2, "Load 1: OFF", 11, invert);
        } else if (relay1_state == RELAY_ON)
        {
            ssd1306_display_text(&main_display, 2, "Load 1:  ON", 11, invert);
        }
    }

}

void load_2_display(bool invert)
{
    if(system_state == DISPLAY)
    {
        if (relay2_state == RELAY_OFF)
        {
            ssd1306_display_text(&main_display, 3, "Load 2: OFF", 11, invert);
        } else if (relay2_state == RELAY_ON)
        {
            ssd1306_display_text(&main_display, 3, "Load 2:  ON", 11, invert);
        }
    }

}

void load_3_display(bool invert)
{
    if(system_state == DISPLAY)
    {
        if (relay3_state == RELAY_OFF)
        {
            ssd1306_display_text(&main_display, 4, "Load 3: OFF", 11, invert);
        } else if (relay3_state == RELAY_ON)
        {
            ssd1306_display_text(&main_display, 4, "Load 3:  ON", 11, invert);
        }
    }

}

static void  mainscreen_entry_display()
{
    ssd1306_clear_screen(&main_display, false);
    ssd1306_contrast(&main_display, 0xFF);
    ssd1306_display_text(&main_display, 0, "  MAIN DISPLAY  ", 16, true);
    ssd1306_display_text(&main_display, 1, "----------------", 16, false);
    load_1_display(false);
    load_2_display(false);
    load_3_display(false);
    ssd1306_display_text(&main_display, 5, "Goto menu       ", 16, false);
    if(ip_address_current != NULL)
    {
        char mainscreen_ip[20] = {0};
        sprintf(mainscreen_ip, "IP%s", (char *) ip_address_current);
        ssd1306_display_text(&main_display, 7, (char *) mainscreen_ip, 16, false);
    }

    switch (display_option)
    {
    case LOAD_1:
        load_1_display(true);
        break;

    case LOAD_2:
            load_2_display(true);
        break;

    case LOAD_3:
        load_3_display(true);
        break;
    
    case GOTO_MENU:
        ssd1306_display_text(&main_display, 5, "Goto menu     ->", 16, true);
        break;

    default:
        break;
    }
}

static void menu_entry_display()
{
    ssd1306_clear_screen(&main_display, false);
    ssd1306_contrast(&main_display, 0xFF);
    ssd1306_display_text_x3(&main_display, 0, " MENU", TXTLENGHT(" MENU"), false);
    ssd1306_display_text(&main_display, 3, " Smart config   ", 16, false);
    ssd1306_display_text(&main_display, 4, " Connect wifi   ", 16, false);
    ssd1306_display_text(&main_display, 5, " Disconnect wifi", 16, false);
    ssd1306_display_text(&main_display, 6, " Exit menu      ", 16, false);

    switch (menu_option)
    {
    case SMARTCONFIG:
        ssd1306_display_text(&main_display, 3, "Smart config    ", 16, true);
        break;

    case CONNECT:
        ssd1306_display_text(&main_display, 4, "Connect wifi    ", 16, true);
        break;

    case DISCONNECT:
        ssd1306_display_text(&main_display, 5, "Disconnect wifi ", 16, true);
        break;

    case EXIT_MENU:
        ssd1306_display_text(&main_display, 6, "Exit menu       ", 16, true);
        break;
    
    default:
        break;
    }
}

static void user_interface_task(void * parm)
{
    EventBits_t uxBits;
    s_usrinterface_event_group = xEventGroupCreate();
    while(1)
    {
        uxBits = xEventGroupWaitBits(s_usrinterface_event_group, (STATECHANGE_BIT | MENUCHOOSE_BIT | MENUPOINTOPT_BIT | DISPLAYPOINTOPT_BIT), true, false, portMAX_DELAY);
        if (uxBits == STATECHANGE_BIT)
        {
            ESP_LOGI(TAG, "State change");
            switch (system_state)
            {
            case MENU:
                printf(" MENU\n");
                menu_entry_display();
                break;

            case DISPLAY:
                printf(" DISPLAY\n");
                mainscreen_entry_display();
                break;
            
            default:
                break;
            }

        } else if (uxBits == MENUPOINTOPT_BIT)
        {
            if(system_state == MENU)
            {
                ESP_LOGI(TAG, "MENU POINT TO");
                ssd1306_display_text(&main_display, 3, " Smart config   ", 16, false);
                ssd1306_display_text(&main_display, 4, " Connect wifi   ", 16, false);
                ssd1306_display_text(&main_display, 5, " Disconnect wifi", 16, false);
                ssd1306_display_text(&main_display, 6, " Exit menu      ", 16, false);
                switch (menu_option)
                {
                case SMARTCONFIG:
                    printf(" POINT TO SMARTCONFIG\n");
                    ssd1306_display_text(&main_display, 3, "Smart config    ", 16, true);
                    break;

                case CONNECT:
                    printf(" POINT TO CONNECT\n");
                    ssd1306_display_text(&main_display, 4, "Connect wifi    ", 16, true);
                    break;
                
                case DISCONNECT:
                    printf(" POINT TO DISCONNECT\n");
                    ssd1306_display_text(&main_display, 5, "Disconnect wifi ", 16, true);
                    break;

                case EXIT_MENU:
                    printf(" POINT TO EXIT MENU\n");
                    ssd1306_display_text(&main_display, 6, "Exit menu       ", 16, true);
                    break;
                
                default:
                    break;
                }
            }
        } else if (uxBits == MENUCHOOSE_BIT)
        {

            if(system_state == MENU)
            {
                ESP_LOGI(TAG, "MENU CHOOSE OPTION");
                switch (menu_option)
                {
                case SMARTCONFIG:
                    printf(" CHOOSE SMARTCONFIG\n");

                    break;

                case CONNECT:
                    printf(" CHOOSE CONNECT\n");
                    break;
                
                case DISCONNECT:
                    printf(" CHOOSE DISCONNECT\n");
                    break;

                case EXIT_MENU:
                    printf(" CHOOSE EXIT MENU\n");
                    break;
                
                default:
                    break;
                }
            }
        } else if (uxBits == DISPLAYPOINTOPT_BIT)
        {
            if(system_state == DISPLAY)
            {
                ESP_LOGI(TAG, "DISPLAY POINT TO");
                load_1_display(false);
                load_2_display(false);
                load_3_display(false);
                ssd1306_display_text(&main_display, 5, "Goto menu       ", 16, false);

                switch (display_option)
                {
                case LOAD_1:
                    load_1_display(true);
                    break;

                case LOAD_2:
                        load_2_display(true);
                    break;

                case LOAD_3:
                    load_3_display(true);
                    break;

                case GOTO_MENU:
                    ssd1306_display_text(&main_display, 5, "Goto menu     ->", 16, true);
                    break;

                default:
                    break;
                }
            }
        }
    }
}

void user_interface_init()
{
    ESP_LOGI(TAG, "Init user interface");
    xTaskCreate(user_interface_task, "user_interface_task", 4096, NULL, 1, NULL);
    ssd1306_init(&main_display, 128, 64);
    mainscreen_entry_display();
}

void button_up_handle()
{
    if (system_state == MENU)
    {
        if (menu_option == SMARTCONFIG)
        {
            menu_option = EXIT_MENU;
        } else
        {
            menu_option--;
        }
    }
    xEventGroupSetBits(s_usrinterface_event_group, MENUPOINTOPT_BIT);
}

void button_enter_handle()
{
    if (system_state == DISPLAY)
    {
        switch (display_option)
        {
        case LOAD_1:
            relay_change_state(RELAY_1);
            load_1_display(true);
            break;

        case LOAD_2:
            relay_change_state(RELAY_2);
            load_2_display(true);
            break;
        
        case LOAD_3:
            relay_change_state(RELAY_3);
            load_3_display(true);
            break;

        case GOTO_MENU:
            system_state = MENU;
            xEventGroupSetBits(s_usrinterface_event_group, STATECHANGE_BIT);
            break;

        default:
            break;
        }


    } else if (system_state == MENU)
    {
        switch (menu_option)
        {
        case SMARTCONFIG:
            esp_smartconfig_stop();
            esp_wifi_disconnect();
            smartconfig_start();
            ssd1306_clear_screen(&main_display, false);
            ssd1306_contrast(&main_display, 0xFF);
            ssd1306_display_text(&main_display, 0, "                ", 16, true);
            ssd1306_display_text(&main_display, 1, "  SMART CONFIG  ", 16, true);
            system_state = MENU_CHOOSED;
            break;
        
        case CONNECT:
            printf("Re-Connect wifi\n");
            ssd1306_clear_screen(&main_display, false);
            ssd1306_contrast(&main_display, 0xFF);
            ssd1306_display_text(&main_display, 0, "                ", 16, true);
            ssd1306_display_text(&main_display, 1, " RECONNECT WIFI ", 16, true);
            esp_wifi_disconnect();
            esp_wifi_set_config(WIFI_IF_STA, &wifi_config_current);
            esp_wifi_connect();
            system_state = MENU_CHOOSED;
            break;

        case DISCONNECT:
            esp_wifi_disconnect();
            ssd1306_clear_screen(&main_display, false);
            ssd1306_contrast(&main_display, 0xFF);
            ssd1306_display_text(&main_display, 0, "                ", 16, true);
            ssd1306_display_text(&main_display, 1, " DISCONNECT WFI ", 16, true);
            ssd1306_display_text(&main_display, 3, " Disconnected...", 16, false);
            vTaskDelay(500/portTICK_PERIOD_MS);
            ssd1306_display_text(&main_display, 4, "Stop web-server ", 16, false);
            ssd1306_display_text(&main_display, 7, " ENTER to MENU->", 16, false);
            ip_address_current = NULL;
            system_state = MENU_CHOOSED;
                break;
            
        case EXIT_MENU:
                system_state = DISPLAY;
                xEventGroupSetBits(s_usrinterface_event_group, STATECHANGE_BIT);
                break;

            default:
                break;
        }

        xEventGroupSetBits(s_usrinterface_event_group, MENUCHOOSE_BIT);

    } else if (system_state == MENU_CHOOSED)
    {
        switch (menu_option)
        {
        case SMARTCONFIG:
            esp_smartconfig_stop();
            // esp_wifi_connect();
            system_state = MENU;
            xEventGroupSetBits(s_usrinterface_event_group, STATECHANGE_BIT);
            break;

        case DISCONNECT:
            system_state = MENU;
            ssd1306_clear_screen(&main_display, false);
            ssd1306_contrast(&main_display, 0xFF);
            xEventGroupSetBits(s_usrinterface_event_group, STATECHANGE_BIT);
            break;

        case CONNECT:
            system_state = MENU;
            ssd1306_clear_screen(&main_display, false);
            ssd1306_contrast(&main_display, 0xFF);
            xEventGroupSetBits(s_usrinterface_event_group, STATECHANGE_BIT);
            break;
        
        default:
            break;
        }
    }

}

void button_down_handle()
{
    if (system_state == MENU)
    {
        if (menu_option == EXIT_MENU)
        {
            menu_option = SMARTCONFIG;
        } else
        {
            menu_option++;
        }
        xEventGroupSetBits(s_usrinterface_event_group, MENUPOINTOPT_BIT);
    } else if (system_state == DISPLAY)
    {
        if (display_option == GOTO_MENU)
        {
            display_option = LOAD_1;
        } else
        {
            display_option++;
        }
        xEventGroupSetBits(s_usrinterface_event_group, DISPLAYPOINTOPT_BIT);
    }

}

void SSID_display()
{
    if((system_state == MENU_CHOOSED) && ((menu_option == SMARTCONFIG) || (menu_option == CONNECT)))
    {
        ssd1306_display_text(&main_display, 3, "SSID:", 5, false);
        char ssid_print[33] = {0};
        sprintf(ssid_print, " %s", (char *) wifi_config_current.ap.ssid);
        ssd1306_display_text(&main_display, 4, (char *) ssid_print, 16, false);
        ssd1306_display_text(&main_display, 7, " ENTER to MENU->", 16, false);
    }
}

void USRIP_display(char * ip_address)
{
    if((system_state == MENU_CHOOSED) && ((menu_option == SMARTCONFIG) || (menu_option == CONNECT)))
    {
        ssd1306_display_text(&main_display, 5, "IP:", 3, false);
        ssd1306_display_text(&main_display, 6, (char *) ip_address, 16, false);

        ip_address_current = ip_address;
    }
}   