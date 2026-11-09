#ifndef KEY_MODULE_H
#define KEY_MODULE_H

#include "zf_common_headfile.h"

#define KEY_MODULE_SAMPLE_PERIOD_MS  (5U)     /* Scan interval within the 1 ms PIT. */
#define KEY_MODULE_DEBOUNCE_SAMPLES  (2U)     /* Stable samples required per transition. */
#define KEY_MODULE_LONG_PRESS_MS     (1000U)  /* Hold time that triggers a long event. */

typedef enum
{
    KEY_EVT_NONE = 0,  /* No unread event. */
    KEY_EVT_SHORT,     /* Press and release before the long-press threshold. */
    KEY_EVT_LONG       /* Long-press threshold reached while held. */
} key_event_enum;

/* Configure the four active-low key inputs. */
void key_module_init(void);

/* Advance the debounce and short/long press state machines. */
void key_module_scan(void);

/* Read and clear one pending event. */
key_event_enum key_module_get_event(key_index_enum key);

/* Discard all pending key events. */
void key_module_clear_all(void);

#endif
