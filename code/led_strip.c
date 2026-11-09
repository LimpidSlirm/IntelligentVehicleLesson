#include "led_strip.h"

#pragma section all "cpu0_dsram"

#define LED_ON_LEVEL       (GPIO_LOW)
#define LED_OFF_LEVEL      (GPIO_HIGH)
#define LED_SPEED_SLOW_MS  (400U)
#define LED_SPEED_MID_MS   (200U)
#define LED_SPEED_FAST_MS  (100U)

/* LEDs are active-low; keep the array in physical strip order. */
static const gpio_pin_enum led_pin_list[LED_STRIP_NUM] =
{
    P20_9, P20_8, P21_5, P21_4
};

static const uint32 speed_period_ms[LED_SPEED_NUM] =
{
    LED_SPEED_SLOW_MS,
    LED_SPEED_MID_MS,
    LED_SPEED_FAST_MS
};

static volatile led_strip_mode_enum strip_mode;
static volatile led_strip_dir_enum strip_direction;
static volatile led_strip_speed_enum strip_speed;
static volatile uint8 strip_paused;
static volatile uint32 step_elapsed_ms;
static volatile uint8 flow_index;
static volatile uint8 blink_on;
static volatile int8 marquee_step;

/* Helpers below update all LEDs or one selected strip position. */
static void led_all_off(void)
{
    uint8 index;

    for(index = 0; index < LED_STRIP_NUM; index++)
    {
        gpio_set_level(led_pin_list[index], LED_OFF_LEVEL);
    }
}

static void led_all_on(void)
{
    uint8 index;

    for(index = 0; index < LED_STRIP_NUM; index++)
    {
        gpio_set_level(led_pin_list[index], LED_ON_LEVEL);
    }
}

static void led_show_index(uint8 index)
{
    led_all_off();
    gpio_set_level(led_pin_list[index], LED_ON_LEVEL);
}

static void led_reset_animation(void)
{
    step_elapsed_ms = 0;
    blink_on = 0;
    if(strip_direction == LED_DIR_FORWARD)
    {
        flow_index = 0;
        marquee_step = 1;
    }
    else
    {
        flow_index = LED_STRIP_NUM - 1U;
        marquee_step = -1;
    }
}

static void led_render_current_mode(void)
{
    if(strip_paused != 0U || strip_mode == LED_STRIP_OFF)
    {
        led_all_off();
    }
    else if(strip_mode == LED_STRIP_ALL_ON)
    {
        led_all_on();
    }
    else if(strip_mode == LED_STRIP_FLOW || strip_mode == LED_STRIP_MARQUEE)
    {
        led_show_index(flow_index);
    }
    else
    {
        led_all_off();
    }
}

void led_strip_init(void)
{
    uint8 index;

    for(index = 0; index < LED_STRIP_NUM; index++)
    {
        gpio_init(led_pin_list[index], GPO, LED_OFF_LEVEL, GPO_PUSH_PULL);
    }

    strip_mode = LED_STRIP_OFF;
    strip_direction = LED_DIR_FORWARD;
    strip_speed = LED_SPEED_MID;
    strip_paused = 0;
    flow_index = 0;
    blink_on = 0;
    marquee_step = 1;
    step_elapsed_ms = 0;
    led_all_off();
}

void led_strip_set_mode(led_strip_mode_enum mode)
{
    uint32 interrupt_state;

    if((uint32)mode >= (uint32)LED_STRIP_MODE_NUM)
    {
        return;
    }

    /* Setters share animation state with the periodic update routine. */
    interrupt_state = interrupt_global_disable();
    strip_mode = mode;
    led_reset_animation();
    led_render_current_mode();
    interrupt_global_enable(interrupt_state);
}

void led_strip_set_direction(led_strip_dir_enum direction)
{
    uint32 interrupt_state;

    if((uint32)direction > (uint32)LED_DIR_BACKWARD)
    {
        return;
    }

    interrupt_state = interrupt_global_disable();
    strip_direction = direction;
    led_reset_animation();
    led_render_current_mode();
    interrupt_global_enable(interrupt_state);
}

void led_strip_set_speed(led_strip_speed_enum speed)
{
    uint32 interrupt_state;

    if((uint32)speed >= (uint32)LED_SPEED_NUM)
    {
        return;
    }

    interrupt_state = interrupt_global_disable();
    strip_speed = speed;
    step_elapsed_ms = 0;
    interrupt_global_enable(interrupt_state);
}

void led_strip_set_paused(uint8 paused)
{
    uint32 interrupt_state = interrupt_global_disable();

    strip_paused = (paused != 0U);
    led_reset_animation();
    led_render_current_mode();

    interrupt_global_enable(interrupt_state);
}

void led_strip_update(void)
{
    /* Called from a 1 ms timer; animation changes never block the main loop. */
    if(strip_paused != 0U ||
       strip_mode == LED_STRIP_OFF ||
       strip_mode == LED_STRIP_ALL_ON)
    {
        return;
    }

    step_elapsed_ms++;
    if(step_elapsed_ms < speed_period_ms[strip_speed])
    {
        return;
    }
    step_elapsed_ms = 0;

    if(strip_mode == LED_STRIP_FLOW)
    {
        if(strip_direction == LED_DIR_FORWARD)
        {
            flow_index = (uint8)((flow_index + 1U) % LED_STRIP_NUM);
        }
        else
        {
            flow_index = (uint8)((flow_index + LED_STRIP_NUM - 1U) % LED_STRIP_NUM);
        }
        led_show_index(flow_index);
    }
    else if(strip_mode == LED_STRIP_BLINK)
    {
        blink_on = (uint8)!blink_on;
        if(blink_on != 0U)
        {
            led_all_on();
        }
        else
        {
            led_all_off();
        }
    }
    else if(strip_mode == LED_STRIP_MARQUEE)
    {
        if((flow_index == (LED_STRIP_NUM - 1U)) && (marquee_step > 0))
        {
            marquee_step = -1;
        }
        else if((flow_index == 0U) && (marquee_step < 0))
        {
            marquee_step = 1;
        }
        flow_index = (uint8)((int8)flow_index + marquee_step);
        led_show_index(flow_index);
    }
}

#pragma section all restore
