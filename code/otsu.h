#ifndef OTSU_H
#define OTSU_H

#include "zf_common_typedef.h"

/* Return the gray level that maximizes the between-class variance. */
uint8 otsu_calculate_threshold(const uint8 *image, uint16 width, uint16 height);

#endif
