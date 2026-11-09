#include "key.h"

#pragma section all "cpu0_dsram"

#define KEY_MODULE_PRESS_LEVEL    (GPIO_LOW)
#define KEY_MODULE_RELEASE_LEVEL  (GPIO_HIGH)

/* Each state separates a stable press/release from contact bounce. */
typedef enum
{
    KEY_STATE_IDLE = 0,
    KEY_STATE_PRESS_DEBOUNCE,
    KEY_STATE_PRESSED,
    KEY_STATE_RELEASE_DEBOUNCE
} key_module_state_enum;

static const gpio_pin_enum key_pin_list[KEY_NUMBER] =
{
    /* KEY1..KEY4 board pin mapping. */
    P20_6, P20_7, P11_2, P11_3
};

static key_module_state_enum key_state[KEY_NUMBER];
static uint8 key_stable_samples[KEY_NUMBER];
static uint32 key_press_time_ms[KEY_NUMBER];
static uint8 key_long_triggered[KEY_NUMBER];
static key_event_enum key_events[KEY_NUMBER];
static uint8 sample_divider;

void key_module_init(void)
{
    uint8 index;

    for(index = 0; index < KEY_NUMBER; index++)
    {
        gpio_init(key_pin_list[index], GPI, GPIO_HIGH, GPI_PULL_UP);
        key_state[index] = KEY_STATE_IDLE;
        key_stable_samples[index] = 0;
        key_press_time_ms[index] = 0;
        key_long_triggered[index] = 0;
        key_events[index] = KEY_EVT_NONE;
    }

    sample_divider = 0;
}

void key_module_scan(void)
{
    uint8 index;
    uint8 level;

    sample_divider++;
    if(sample_divider < KEY_MODULE_SAMPLE_PERIOD_MS)
    {
        return;
    }
    sample_divider = 0;

    for(index = 0; index < KEY_NUMBER; index++)
    {
        level = gpio_get_level(key_pin_list[index]);

        /* Debounce both edges; emit short presses on release and long presses once. */
        switch(key_state[index])
        {
            case KEY_STATE_IDLE:
                if(level == KEY_MODULE_PRESS_LEVEL)
                {
                    key_stable_samples[index] = 1;
                    key_state[index] = KEY_STATE_PRESS_DEBOUNCE;
                }
                break;

            case KEY_STATE_PRESS_DEBOUNCE:
                if(level == KEY_MODULE_PRESS_LEVEL)
                {
                    key_stable_samples[index]++;
                    if(key_stable_samples[index] >= KEY_MODULE_DEBOUNCE_SAMPLES)
                    {
                        key_press_time_ms[index] = 0;
                        key_long_triggered[index] = 0;
                        key_state[index] = KEY_STATE_PRESSED;
                    }
                }
                else
                {
                    key_stable_samples[index] = 0;
                    key_state[index] = KEY_STATE_IDLE;
                }
                break;

            case KEY_STATE_PRESSED:
                if(level == KEY_MODULE_PRESS_LEVEL)
                {
                    key_press_time_ms[index] += KEY_MODULE_SAMPLE_PERIOD_MS;
                    if((key_press_time_ms[index] >= KEY_MODULE_LONG_PRESS_MS) &&
                       (key_long_triggered[index] == 0))
                    {
                        key_events[index] = KEY_EVT_LONG;
                        key_long_triggered[index] = 1;
                    }
                }
                else
                {
                    key_stable_samples[index] = 1;
                    key_state[index] = KEY_STATE_RELEASE_DEBOUNCE;
                }
                break;

            case KEY_STATE_RELEASE_DEBOUNCE:
                if(level == KEY_MODULE_RELEASE_LEVEL)
                {
                    key_stable_samples[index]++;
                    if(key_stable_samples[index] >= KEY_MODULE_DEBOUNCE_SAMPLES)
                    {
                        if(key_long_triggered[index] == 0)
                        {
                            key_events[index] = KEY_EVT_SHORT;
                        }
                        key_stable_samples[index] = 0;
                        key_state[index] = KEY_STATE_IDLE;
                    }
                }
                else
                {
                    key_stable_samples[index] = 0;
                    key_state[index] = KEY_STATE_PRESSED;
                }
                break;

            default:
                key_stable_samples[index] = 0;
                key_state[index] = KEY_STATE_IDLE;
                break;
        }
    }
}

key_event_enum key_module_get_event(key_index_enum key)
{
    key_event_enum event;
    uint32 interrupt_state;

    if((uint32)key >= KEY_NUMBER)
    {
        return KEY_EVT_NONE;
    }

    /* Consume the event atomically because the PIT ISR may replace it. */
    interrupt_state = interrupt_global_disable();
    event = key_events[key];
    key_events[key] = KEY_EVT_NONE;
    interrupt_global_enable(interrupt_state);

    return event;
}

void key_module_clear_all(void)
{
    uint8 index;
    /* Prevent the scanner from changing the event array during the clear. */
    uint32 interrupt_state = interrupt_global_disable();

    for(index = 0; index < KEY_NUMBER; index++)
    {
        key_events[index] = KEY_EVT_NONE;
    }

    interrupt_global_enable(interrupt_state);
}

#pragma section all restore
