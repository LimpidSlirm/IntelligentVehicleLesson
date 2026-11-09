#ifndef PID_H
#define PID_H

#include "zf_common_headfile.h"

#define PID_GAIN_SCALE (1000)

typedef struct
{
    int32 kp;
    int32 ki;
    int32 kd;
    int32 output_min;
    int32 output_max;
    int32 integral_min;
    int32 integral_max;
    int32 integral;
    int32 previous_error;
    int32 output;
    uint8 initialized;
} pid_controller_t;

void pid_init(pid_controller_t *pid,
              int32 kp,
              int32 ki,
              int32 kd,
              int32 output_min,
              int32 output_max);
void pid_reset(pid_controller_t *pid);
int32 pid_update(pid_controller_t *pid, int32 target, int32 measurement);

#endif
