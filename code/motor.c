#include "motor.h"

#pragma section all "cpu0_dsram"

#define MOTOR_PWM_FREQ   (17000U)
#define MOTOR_RAMP_STEP  (50)

#define MOTOR_LEFT_A_PWM     (ATOM0_CH4_P02_4)
#define MOTOR_LEFT_B_PWM     (ATOM0_CH5_P02_5)
#define MOTOR_RIGHT_A_PWM    (ATOM0_CH6_P02_6)
#define MOTOR_RIGHT_B_PWM    (ATOM0_CH7_P02_7)

static volatile int16 motor_left_target;
static volatile int16 motor_right_target;
static volatile int16 motor_left_current;
static volatile int16 motor_right_current;

static int16 motor_approach_target(int16 current, int16 target)
{
    if(current < target)
    {
        if((target - current) <= MOTOR_RAMP_STEP)
        {
            return target;
        }
        return (int16)(current + MOTOR_RAMP_STEP);
    }

    if(current > target)
    {
        if((current - target) <= MOTOR_RAMP_STEP)
        {
            return target;
        }
        return (int16)(current - MOTOR_RAMP_STEP);
    }

    return current;
}

static int16 motor_limit_speed(int16 speed)
{
    if(speed > MOTOR_SPEED_MAX)
    {
        return MOTOR_SPEED_MAX;
    }
    if(speed < -MOTOR_SPEED_MAX)
    {
        return (int16)-MOTOR_SPEED_MAX;
    }
    return speed;
}

static void motor_apply_wheel(int16 speed,
                              pwm_channel_enum positive_channel,
                              pwm_channel_enum negative_channel)
{
    uint32 duty;

    if(speed < 0)
    {
        duty = (uint32)(-speed);
        pwm_set_duty(positive_channel, 0);
        pwm_set_duty(negative_channel, duty);
    }
    else
    {
        duty = (uint32)speed;
        pwm_set_duty(positive_channel, duty);
        pwm_set_duty(negative_channel, 0);
    }
}

void motor_init(void)
{
    motor_left_target = 0;
    motor_right_target = 0;
    motor_left_current = 0;
    motor_right_current = 0;

    pwm_init(MOTOR_LEFT_A_PWM, MOTOR_PWM_FREQ, 0);
    pwm_init(MOTOR_LEFT_B_PWM, MOTOR_PWM_FREQ, 0);
    pwm_init(MOTOR_RIGHT_A_PWM, MOTOR_PWM_FREQ, 0);
    pwm_init(MOTOR_RIGHT_B_PWM, MOTOR_PWM_FREQ, 0);

    motor_apply_wheel(0, MOTOR_LEFT_A_PWM, MOTOR_LEFT_B_PWM);
    motor_apply_wheel(0, MOTOR_RIGHT_A_PWM, MOTOR_RIGHT_B_PWM);
}

void motor_set(int16 left_speed, int16 right_speed)
{
    motor_left_target = motor_limit_speed(left_speed);
    motor_right_target = motor_limit_speed(right_speed);
}

void motor_update(void)
{
    motor_left_current = motor_approach_target(motor_left_current,
                                                motor_left_target);
    motor_right_current = motor_approach_target(motor_right_current,
                                                 motor_right_target);

    motor_apply_wheel(motor_left_current,
                      MOTOR_LEFT_A_PWM,
                      MOTOR_LEFT_B_PWM);
    motor_apply_wheel(motor_right_current,
                      MOTOR_RIGHT_A_PWM,
                      MOTOR_RIGHT_B_PWM);
}

void motor_stop(void)
{
    motor_left_target = 0;
    motor_right_target = 0;
}

void motor_stop_immediate(void)
{
    motor_left_target = 0;
    motor_right_target = 0;
    motor_left_current = 0;
    motor_right_current = 0;
    motor_apply_wheel(0, MOTOR_LEFT_A_PWM, MOTOR_LEFT_B_PWM);
    motor_apply_wheel(0, MOTOR_RIGHT_A_PWM, MOTOR_RIGHT_B_PWM);
}

#pragma section all restore
