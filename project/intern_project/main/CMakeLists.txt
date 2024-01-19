idf_component_register(SRCS "intern_project.c"
                    REQUIRES ADC_Driver GPIO_Driver EXTI_Driver HTTP_Driver Wifi_Driver
                    INCLUDE_DIRS ".")
