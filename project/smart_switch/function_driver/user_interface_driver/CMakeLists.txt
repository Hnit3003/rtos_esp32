idf_component_register(SRCS "user_interface_driver.c"
                    REQUIRES wifi_smartconfig_driver ssd1306_driver relay_control_driver
                    INCLUDE_DIRS ".")