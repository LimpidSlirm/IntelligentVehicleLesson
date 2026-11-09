#ifndef SERVO_H
#define SERVO_H

#include "zf_common_headfile.h"

#define SERVO_ANGLE_MIN       (40)
#define SERVO_ANGLE_MAX       (140)
#define SERVO_NEUTRAL_ANGLE   (90)

/* These defaults are common analog-servo values; verify them for the device. */
#define SERVO_PULSE_MIN_US    (500U)
#define SERVO_PULSE_MAX_US    (2500U)

void servo_init(void);
void servo_set_angle(int16 angle);
void servo_center(void);
void servo_update(void);
void servo_hold(void);
int16 servo_get_target_angle(void);
int16 servo_get_current_angle(void);

#endif
