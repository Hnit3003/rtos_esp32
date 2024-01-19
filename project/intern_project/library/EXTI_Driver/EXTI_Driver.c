#include "EXTI_Driver.h"
#include "stdbool.h"

xQueueHandle xQueue_ExternalInterrupt;
static bool is_initialized = false;

void (*EXTI_RisingEdgeDectect_Handle) (gpio_num_t EXTI_Pin);
void (*EXTI_FallingEdgeDectect_Handle) (gpio_num_t EXTI_Pin);

void EXTI_InitFunction(void (*RisingEdgeDectect_Handle) (gpio_num_t EXTI_Pin), void (*FallingEdgeDectect_Handle) (gpio_num_t EXTI_Pin))
{
    EXTI_RisingEdgeDectect_Handle = RisingEdgeDectect_Handle;
    EXTI_FallingEdgeDectect_Handle = FallingEdgeDectect_Handle;
}

static void IRAM_ATTR external_interrupt_handler(void *args)
{
    uint32_t pinNumber = (uint32_t)args;
    xQueueSendFromISR(xQueue_ExternalInterrupt, &pinNumber, NULL);
}

void external_interrupt_task(void *pvParameter)
{
    uint32_t EXTI_Pin = 0;
    while(1)
    {
        if(xQueueReceive(xQueue_ExternalInterrupt, &EXTI_Pin, portMAX_DELAY))
        {   
            vTaskDelay(15 / portTICK_PERIOD_MS);
            // xQueueReset(xQueue_ExternalInterrupt);
            for(uint8_t Queue_MessWaiting = uxQueueMessagesWaiting(xQueue_ExternalInterrupt); Queue_MessWaiting > 0; Queue_MessWaiting--)
            {
                uint32_t buffer_mess = 0;
                if(xQueueReceive(xQueue_ExternalInterrupt, &buffer_mess, 1 / portTICK_PERIOD_MS) == pdTRUE)
                {
                    if(buffer_mess != EXTI_Pin)
                    {
                        xQueueSendToBack(xQueue_ExternalInterrupt, &buffer_mess, 1 / portTICK_PERIOD_MS);
                    }
                }
            }

            if(gpio_get_level(EXTI_Pin) == 1)
            {
                //Function handle EXTI_Source (Rising edge)
                EXTI_RisingEdgeDectect_Handle((gpio_num_t)EXTI_Pin);
                // continue;         //debounce for falling edge
            }else if(gpio_get_level(EXTI_Pin) == 0)
            {
                //Function handle EXTI_Source (Falling edge)
                EXTI_FallingEdgeDectect_Handle((gpio_num_t)EXTI_Pin);
            }
        }
    }
}

void external_interrupt_initialize(gpio_num_t gpio_pin, gpio_int_type_t intr_edge)
{
    if(is_initialized == false)
    {
        xQueue_ExternalInterrupt = xQueueCreate(10, sizeof(uint32_t));
        TaskHandle_t xEXTI_TASK = NULL;
        xTaskCreate(external_interrupt_task, "EXTI_Task", 2048, NULL, 1, &xEXTI_TASK);
        is_initialized = true;
    }

    gpio_config_t pin_conf;
    pin_conf.pin_bit_mask = 1ULL << gpio_pin;
    pin_conf.mode = GPIO_MODE_INPUT;
    pin_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    pin_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    pin_conf.intr_type = intr_edge;
    gpio_config(&pin_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(gpio_pin, external_interrupt_handler, (void *)gpio_pin); 
}
