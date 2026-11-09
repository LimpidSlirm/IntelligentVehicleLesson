#ifndef APP_H
#define APP_H

#include "zf_common_headfile.h"

/* Initialize the key, motor, and periodic timer modules. */
void app_init(void);

/* Handle key events in the main loop. */
void app_task(void);

#endif
