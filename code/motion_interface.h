#ifndef MOTION_INTERFACE_H
#define MOTION_INTERFACE_H

#include "zf_common_typedef.h"

typedef struct
{
    int16 left_target;
    int16 right_target;
    uint8 enable;
    uint8 safety_stop;
} motion_command_t;

typedef struct
{
    int16 left_measurement;
    int16 right_measurement;
    int16 left_output;
    int16 right_output;
    uint8 feedback_valid;
} motion_feedback_t;

#endif
