#ifndef CHASSIS_H
#define CHASSIS_H

#include "motion.h"

/* Abstract speed units are intentionally kept independent of wheel geometry. */
#define CHASSIS_TRACK_WIDTH_UNITS  (1000)
#define CHASSIS_SPEED_MAX          (5000)
#define CHASSIS_STEERING_MIN       (40)
#define CHASSIS_STEERING_MAX       (140)
#define CHASSIS_STEERING_NEUTRAL   (90)

void chassis_init(void);
void chassis_set_velocity(int16 linear, int16 angular);
void chassis_set_wheel_speed(int16 left, int16 right);
void chassis_stop_ramped(void);
void chassis_stop_immediate(void);
void chassis_set_steering_angle(int16 angle);
void chassis_center_steering(void);

#endif
