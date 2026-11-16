#include "otsu.h"

#pragma section all "cpu0_dsram"

#define OTSU_GRAY_LEVEL_NUM  (256U)

static uint32 otsu_histogram[OTSU_GRAY_LEVEL_NUM];

uint8 otsu_calculate_threshold(const uint8 *image, uint16 width, uint16 height)
{
    uint32 total_pixels;
    uint32 total_gray_sum = 0U;
    uint32 foreground_pixels = 0U;
    uint32 foreground_gray_sum = 0U;
    uint32 background_pixels;
    uint32 background_gray_sum;
    uint32 foreground_mean;
    uint32 background_mean;
    uint32 index;
    uint32 level;
    int32 mean_diff;
    uint64 score;
    uint64 best_score = 0U;
    uint8 threshold = 0U;

    if((image == 0) || (width == 0U) || (height == 0U))
    {
        return 0U;
    }

    for(level = 0U; level < OTSU_GRAY_LEVEL_NUM; level++)
    {
        otsu_histogram[level] = 0U;
    }

    total_pixels = (uint32)width * (uint32)height;
    for(index = 0U; index < total_pixels; index++)
    {
        otsu_histogram[image[index]]++;
    }

    for(level = 0U; level < OTSU_GRAY_LEVEL_NUM; level++)
    {
        total_gray_sum += level * otsu_histogram[level];
    }

    /* Split the histogram once per level: the accumulated sums replace the
       nested re-summation used by the direct implementation. */
    for(level = 0U; level < OTSU_GRAY_LEVEL_NUM; level++)
    {
        foreground_pixels += otsu_histogram[level];
        foreground_gray_sum += level * otsu_histogram[level];

        background_pixels = total_pixels - foreground_pixels;
        if((foreground_pixels == 0U) || (background_pixels == 0U))
        {
            continue;
        }

        background_gray_sum = total_gray_sum - foreground_gray_sum;
        foreground_mean = foreground_gray_sum / foreground_pixels;
        background_mean = background_gray_sum / background_pixels;
        mean_diff = (int32)foreground_mean - (int32)background_mean;
        score = (uint64)foreground_pixels * (uint64)background_pixels
              * (uint64)(mean_diff * mean_diff);

        if(score > best_score)
        {
            best_score = score;
            threshold = (uint8)level;
        }
    }

    return threshold;
}

#pragma section all restore
