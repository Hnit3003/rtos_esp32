idf_component_register(SRCS "app_main.c"
                    # REQUIRES ADC_Driver GPIO_Driver EXTI_Driver GPTIMER_Driver Wifi_Driver
                    INCLUDE_DIRS ".")
