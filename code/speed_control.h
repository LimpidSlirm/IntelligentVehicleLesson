#ifndef SPEED_CONTROL_H
#define SPEED_CONTROL_H

#include "zf_driver_encoder.h"
#include "motion_interface.h"

void speed_control_init(void);
void speed_control_set_command(const motion_command_t *command);
void speed_control_get_feedback(motion_feedback_t *feedback);
void speed_control_set_target(int16 left_target, int16 right_target);
void speed_control_update(void);
int16 speed_control_get_left_target(void);
int16 speed_control_get_right_target(void);
int16 speed_control_get_left_measurement(void);
int16 speed_control_get_right_measurement(void);
int16 speed_control_get_left_output(void);
int16 speed_control_get_right_output(void);

#endif
