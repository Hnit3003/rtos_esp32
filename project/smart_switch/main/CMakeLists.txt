idf_component_register(SRCS "smarthomeIOT.c"
                    REQUIRES wifi_smartconfig_driver gpio_driver exti_driver nvs_flash relay_control_driver user_interface_driver ssd1306_driver
                    INCLUDE_DIRS ".")
