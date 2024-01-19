idf_component_register(SRCS "http_driver.c"
                    REQUIRES esp_http_server gpio_driver relay_control_driver user_interface_driver
                    INCLUDE_DIRS ".")