idf_component_register(SRCS "HTTP_Driver.c"
                    REQUIRES esp_http_server GPIO_Driver ADC_Driver
                    INCLUDE_DIRS ".")