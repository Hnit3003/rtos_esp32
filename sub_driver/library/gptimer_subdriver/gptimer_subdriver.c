#include "gptimer_subdriver.h"
#include "stdbool.h"

static SemaphoreHandle_t Sem_GPTimerInterrupt;
static bool is_initialized = false;

static bool IRAM_ATTR gptimer_interrupt_handle(void *args)
{
    BaseType_t is_high_task_awoken = pdFALSE;
    xSemaphoreGiveFromISR(Sem_GPTimerInterrupt, &is_high_task_awoken);
    return (is_high_task_awoken == pdTRUE);
}

void gptimer_interrupt_initialize(timer_group_t TIMER_GROUP, timer_idx_t TIMER_NUM, uint32_t DIVIDER)
{
    if(is_initialized == false)
    {
        Sem_GPTimerInterrupt = xSemaphoreCreateBinary();
        is_initialized = true;
    }

    timer_config_t gptimer_conf = {
        .divider = DIVIDER,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = TIMER_AUTORELOAD_EN
    };

    timer_init(TIMER_GROUP, TIMER_NUM, &gptimer_conf);
    timer_set_counter_value(TIMER_GROUP, TIMER_NUM, 0);
    timer_enable_intr(TIMER_GROUP, TIMER_NUM);
    timer_isr_callback_add(TIMER_GROUP, TIMER_NUM, gptimer_interrupt_handle, (void *)TIMER_NUM, 0);
}

void gptimer_start_once(timer_group_t TIMER_GROUP, timer_idx_t TIMER_NUM, uint32_t DIVIDER, uint64_t ALARM_MS)
{
    timer_set_divider(TIMER_GROUP, TIMER_NUM, DIVIDER);
    timer_set_alarm_value(TIMER_GROUP, TIMER_NUM, (ALARM_MS*TIMER_BASE_CLK)/(DIVIDER*1000));
    timer_start(TIMER_GROUP_0, TIMER_0);
}