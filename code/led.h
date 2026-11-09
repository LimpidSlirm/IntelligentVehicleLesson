#ifndef LED_H
#define LED_H

#include "zf_common_headfile.h"

#define LED_STRIP_NUM  (4U)

/* Available display patterns. */
typedef enum
{
    LED_STRIP_OFF = 0, /* All LEDs off. */
    LED_STRIP_ALL_ON,  /* All LEDs on. */
    LED_STRIP_FLOW,    /* One active LED moves continuously around the strip. */
    LED_STRIP_BLINK,   /* All LEDs blink together. */
    LED_STRIP_MARQUEE, /* One active LED bounces between the strip ends. */
    LED_STRIP_MODE_NUM
} led_strip_mode_enum;

/* Direction used by the flowing and marquee patterns. */
typedef enum
{
    LED_DIR_FORWARD = 0,
    LED_DIR_BACKWARD
} led_strip_dir_enum;

/* Animation step intervals. */
typedef enum
{
    LED_SPEED_SLOW = 0,
    LED_SPEED_MID,
    LED_SPEED_FAST,
    LED_SPEED_NUM
} led_strip_speed_enum;

/* Configure the strip and select its pattern, direction, speed, or pause state. */
void led_strip_init(void);
void led_strip_set_mode(led_strip_mode_enum mode);
void led_strip_set_direction(led_strip_dir_enum direction);
void led_strip_set_speed(led_strip_speed_enum speed);
void led_strip_set_paused(uint8 paused);
/* Advance the current animation by one timer tick. */
void led_strip_update(void);

#endif
