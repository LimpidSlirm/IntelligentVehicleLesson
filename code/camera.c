#include "camera.h"
#include "otsu.h"
#include "zf_device_mt9v03x.h"

#pragma section all "cpu0_dsram"

static volatile uint8 camera_threshold;
static volatile uint32 camera_frame_count;
static volatile uint8 camera_ready;

uint8 camera_otsu_init(void)
{
    uint8 result;

    camera_threshold = 0U;
    camera_frame_count = 0U;
    camera_ready = 0U;

    result = mt9v03x_init();
    if(result == 0U)
    {
        camera_ready = 1U;
    }

    return result;
}

void camera_otsu_task(void)
{
    if((camera_ready == 0U) || (mt9v03x_finish_flag == 0U))
    {
        return;
    }

    camera_threshold = otsu_calculate_threshold(&mt9v03x_image[0][0],
                                                MT9V03X_W,
                                                MT9V03X_H);
    camera_frame_count++;
    mt9v03x_finish_flag = 0U;
}

uint8 camera_otsu_get_threshold(void)
{
    return camera_threshold;
}

uint32 camera_otsu_get_frame_count(void)
{
    return camera_frame_count;
}

const uint8 *camera_otsu_get_image(void)
{
    return &mt9v03x_image[0][0];
}

uint16 camera_otsu_get_width(void)
{
    return MT9V03X_W;
}

uint16 camera_otsu_get_height(void)
{
    return MT9V03X_H;
}

#pragma section all restore
