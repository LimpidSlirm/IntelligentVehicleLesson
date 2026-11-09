#include "pid.h"

#pragma section all "cpu0_dsram"

static int32 pid_limit(int32 value, int32 minimum, int32 maximum)
{
    if(value < minimum)
    {
        return minimum;
    }
    if(value > maximum)
    {
        return maximum;
    }
    return value;
}

void pid_init(pid_controller_t *pid,
              int32 kp,
              int32 ki,
              int32 kd,
              int32 output_min,
              int32 output_max)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->output_min = output_min;
    pid->output_max = output_max;
    pid->integral_min = -200000;
    pid->integral_max = 200000;
    pid_reset(pid);
}

void pid_reset(pid_controller_t *pid)
{
    pid->integral = 0;
    pid->previous_error = 0;
    pid->output = 0;
    pid->initialized = 0;
}

int32 pid_update(pid_controller_t *pid, int32 target, int32 measurement)
{
    int32 error = target - measurement;
    int32 derivative;
    int32 proportional;
    int32 integral_term;
    int32 derivative_term;
    int32 output;

    if(pid->initialized == 0U)
    {
        derivative = 0;
        pid->initialized = 1U;
    }
    else
    {
        derivative = error - pid->previous_error;
    }

    pid->integral = pid_limit(pid->integral + error,
                              pid->integral_min,
                              pid->integral_max);

    proportional = (pid->kp * error) / PID_GAIN_SCALE;
    integral_term = (pid->ki * pid->integral) / PID_GAIN_SCALE;
    derivative_term = (pid->kd * derivative) / PID_GAIN_SCALE;
    output = proportional + integral_term + derivative_term;
    output = pid_limit(output, pid->output_min, pid->output_max);

    pid->previous_error = error;
    pid->output = output;
    return output;
}

#pragma section all restore
