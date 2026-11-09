#ifndef MOTOR_H
#define MOTOR_H

#include "zf_common_headfile.h"

#define MOTOR_SPEED_MAX  (5000) /* Maximum drive duty; PWM_DUTY_MAX is 10000. */

/* Initialize two PWM outputs for each motor's A/B H-bridge inputs. */
void motor_init(void);

/* Set signed target duties; positive is forward, negative is reverse. */
void motor_set(int16 left_speed, int16 right_speed);

/* Advance the soft-start/soft-stop ramp; call every 1 ms. */
void motor_update(void);

/* Ramp both motors to zero. */
void motor_stop(void);

#endif
