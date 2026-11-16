#ifndef CAMERA_H
#define CAMERA_H

#include "zf_common_headfile.h"

/* Initialize the MT9V03X. Returns 0 on success. */
uint8 camera_otsu_init(void);

/* Process at most one finished frame; call from the main loop. */
void camera_otsu_task(void);

/* Latest Otsu threshold; valid after the first processed frame. */
uint8 camera_otsu_get_threshold(void);

/* Number of frames processed so far. */
uint32 camera_otsu_get_frame_count(void);

/* Raw gray buffer of the latest frame, row-major. */
const uint8 *camera_otsu_get_image(void);

/* Captured image width and height in pixels. */
uint16 camera_otsu_get_width(void);
uint16 camera_otsu_get_height(void);

#endif
