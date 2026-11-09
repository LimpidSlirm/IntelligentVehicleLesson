#include "speed_control.h"
#include "motor.h"
#include "pid.h"

#pragma section all "cpu0_dsram"

#define SPEED_CONTROL_LEFT_FEEDBACK_SIGN   (1)
#define SPEED_CONTROL_RIGHT_FEEDBACK_SIGN  (1)

/* Gains are scaled by PID_GAIN_SCALE and should be retuned on the vehicle. */
#define SPEED_CONTROL_KP  (1000)
#define SPEED_CONTROL_KI  (40)
#define SPEED_CONTROL_KD  (0)

static pid_controller_t left_pid;
static pid_controller_t right_pid;
static volatile int16 left_target;
static volatile int16 right_target;
static volatile int16 left_measurement;
static volatile int16 right_measurement;
static volatile int16 left_output;
static volatile int16 right_output;

void speed_control_set_command(const motion_command_t *command)
{
    if(command == 0)
    {
        return;
    }

    if(command->enable == 0U || command->safety_stop != 0U)
    {
        left_target = 0;
        right_target = 0;
    }
    else
    {
        left_target = command->left_target;
        right_target = command->right_target;
    }
}

void speed_control_get_feedback(motion_feedback_t *feedback)
{
    if(feedback == 0)
    {
        return;
    }

    feedback->left_measurement = left_measurement;
    feedback->right_measurement = right_measurement;
    feedback->left_output = left_output;
    feedback->right_output = right_output;
    feedback->feedback_valid = 1U;
}

static int16 speed_control_apply_sign(int16 value, int32 sign)
{
    return (sign < 0) ? (int16)-value : value;
}

void speed_control_init(void)
{
    encoder_dir_init(TIM5_ENCODER,
                     TIM5_ENCODER_CH1_P10_3,
                     TIM5_ENCODER_CH2_P10_1);
    encoder_dir_init(TIM6_ENCODER,
                     TIM6_ENCODER_CH1_P20_3,
                     TIM6_ENCODER_CH2_P20_0);
    encoder_clear_count(TIM5_ENCODER);
    encoder_clear_count(TIM6_ENCODER);

    pid_init(&left_pid, SPEED_CONTROL_KP, SPEED_CONTROL_KI,
             SPEED_CONTROL_KD, -MOTOR_SPEED_MAX, MOTOR_SPEED_MAX);
    pid_init(&right_pid, SPEED_CONTROL_KP, SPEED_CONTROL_KI,
             SPEED_CONTROL_KD, -MOTOR_SPEED_MAX, MOTOR_SPEED_MAX);

    left_target = 0;
    right_target = 0;
    left_measurement = 0;
    right_measurement = 0;
    left_output = 0;
    right_output = 0;
}

void speed_control_set_target(int16 new_left_target, int16 new_right_target)
{
    motion_command_t command;

    command.left_target = new_left_target;
    command.right_target = new_right_target;
    command.enable = 1U;
    command.safety_stop = 0U;
    speed_control_set_command(&command);
}

void speed_control_update(void)
{
    int16 left_count;
    int16 right_count;
    int32 left_result;
    int32 right_result;

    left_count = encoder_get_count(TIM5_ENCODER);
    right_count = encoder_get_count(TIM6_ENCODER);
    encoder_clear_count(TIM5_ENCODER);
    encoder_clear_count(TIM6_ENCODER);

    left_measurement = speed_control_apply_sign(
        left_count, SPEED_CONTROL_LEFT_FEEDBACK_SIGN);
    right_measurement = speed_control_apply_sign(
        right_count, SPEED_CONTROL_RIGHT_FEEDBACK_SIGN);

    if(left_target == 0)
    {
        pid_reset(&left_pid);
        left_result = 0;
    }
    else
    {
        left_result = pid_update(&left_pid, left_target, left_measurement);
    }

    if(right_target == 0)
    {
        pid_reset(&right_pid);
        right_result = 0;
    }
    else
    {
        right_result = pid_update(&right_pid, right_target, right_measurement);
    }

    left_output = (int16)left_result;
    right_output = (int16)right_result;
    motor_set(left_output, right_output);
}

int16 speed_control_get_left_target(void)
{
    return left_target;
}

int16 speed_control_get_right_target(void)
{
    return right_target;
}

int16 speed_control_get_left_measurement(void)
{
    return left_measurement;
}

int16 speed_control_get_right_measurement(void)
{
    return right_measurement;
}

int16 speed_control_get_left_output(void)
{
    return left_output;
}

int16 speed_control_get_right_output(void)
{
    return right_output;
}

#pragma section all restore
