#include "servo.h"

#define SERVO_PWM_CHANNEL  (ATOM0_CH1_P33_9)
#define SERVO_PWM_FREQ     (50U)
#define SERVO_PERIOD_US    (20000U)

static volatile int16 servo_target_angle;
static volatile int16 servo_current_angle;
static volatile uint8 servo_is_held;

static int16 servo_limit_angle(int16 angle)
{
    if(angle < SERVO_ANGLE_MIN)
    {
        return SERVO_ANGLE_MIN;
    }
    if(angle > SERVO_ANGLE_MAX)
    {
        return SERVO_ANGLE_MAX;
    }
    return angle;
}

static uint32 servo_angle_to_duty(int16 angle)
{
    uint32 pulse_us;
    uint32 pulse_range;

    pulse_range = SERVO_PULSE_MAX_US - SERVO_PULSE_MIN_US;
    pulse_us = SERVO_PULSE_MIN_US
             + ((uint32)(angle - SERVO_ANGLE_MIN) * pulse_range)
               / (SERVO_ANGLE_MAX - SERVO_ANGLE_MIN);
    return (pulse_us * PWM_DUTY_MAX) / SERVO_PERIOD_US;
}

void servo_init(void)
{
    servo_target_angle = servo_limit_angle(SERVO_NEUTRAL_ANGLE);
    servo_current_angle = servo_target_angle;
    servo_is_held = 0U;
    pwm_init(SERVO_PWM_CHANNEL, SERVO_PWM_FREQ,
             servo_angle_to_duty(servo_current_angle));
}

void servo_set_angle(int16 angle)
{
    servo_target_angle = servo_limit_angle(angle);
    servo_is_held = 0U;
}

void servo_center(void)
{
    servo_set_angle(SERVO_NEUTRAL_ANGLE);
}

void servo_update(void)
{
    if(servo_is_held != 0U)
    {
        return;
    }

    if(servo_current_angle < servo_target_angle)
    {
        servo_current_angle++;
    }
    else if(servo_current_angle > servo_target_angle)
    {
        servo_current_angle--;
    }
    else
    {
        return;
    }

    pwm_set_duty(SERVO_PWM_CHANNEL,
                 servo_angle_to_duty(servo_current_angle));
}

void servo_hold(void)
{
    servo_target_angle = servo_current_angle;
    servo_is_held = 1U;
}

int16 servo_get_target_angle(void)
{
    return servo_target_angle;
}

int16 servo_get_current_angle(void)
{
    return servo_current_angle;
}
