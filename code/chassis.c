#include "chassis.h"
#include "motor.h"
#include "servo.h"
#include "speed_control.h"

static int16 chassis_limit(int32 value)
{
    if(value > CHASSIS_SPEED_MAX)
    {
        return CHASSIS_SPEED_MAX;
    }
    if(value < -CHASSIS_SPEED_MAX)
    {
        return (int16)-CHASSIS_SPEED_MAX;
    }
    return (int16)value;
}

static int16 chassis_limit_steering(int16 angle)
{
    if(angle < CHASSIS_STEERING_MIN)
    {
        return CHASSIS_STEERING_MIN;
    }
    if(angle > CHASSIS_STEERING_MAX)
    {
        return CHASSIS_STEERING_MAX;
    }
    return angle;
}

void chassis_init(void)
{
    chassis_stop_immediate();
    chassis_center_steering();
}

void chassis_set_wheel_speed(int16 left, int16 right)
{
    motion_command_t command;

    command.left_target = chassis_limit(left);
    command.right_target = chassis_limit(right);
    command.enable = 1U;
    command.safety_stop = 0U;
    speed_control_set_command(&command);
}

void chassis_set_velocity(int16 linear, int16 angular)
{
    int32 half_track;
    int32 left;
    int32 right;

    half_track = CHASSIS_TRACK_WIDTH_UNITS / 2;
    left = (int32)linear - (int32)angular * half_track;
    right = (int32)linear + (int32)angular * half_track;
    chassis_set_wheel_speed(chassis_limit(left), chassis_limit(right));
}

void chassis_stop_ramped(void)
{
    chassis_set_wheel_speed(0, 0);
}

void chassis_stop_immediate(void)
{
    motion_command_t command;

    command.left_target = 0;
    command.right_target = 0;
    command.enable = 0U;
    command.safety_stop = 1U;
    speed_control_set_command(&command);
    motor_stop_immediate();
}

void chassis_set_steering_angle(int16 angle)
{
    servo_set_angle(chassis_limit_steering(angle));
}

void chassis_center_steering(void)
{
    chassis_set_steering_angle(CHASSIS_STEERING_NEUTRAL);
}
