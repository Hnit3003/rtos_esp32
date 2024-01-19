idf_component_register(SRCS "relay_control_driver.c"
                    REQUIRES gpio_driver exti_driver
                    INCLUDE_DIRS ".")