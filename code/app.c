#include "app.h"
#include "camera.h"
#include "chassis.h"
#include "key.h"
#include "motor.h"
#include "servo.h"
#include "speed.h"

#pragma section all "cpu0_dsram"

/* The display is switched to landscape before init, so the usable area is
   160 x 128 and the 188x120 frame is scaled to 160x102. */
#define APP_IMAGE_WIDTH    (160U)
#define APP_IMAGE_HEIGHT   (102U)
#define APP_IMAGE_Y        (0U)

static char uart_command[32];
static uint8 uart_command_length;
static uint8 image_binary_mode;
static uint8 image_display_dirty;
static uint32 displayed_frame_count;

static void app_uart_send_status(void)
{
    uart_write_string(UART_0,
                      (image_binary_mode != 0U) ? "STATUS MODE=BIN\r\n"
                                                : "STATUS MODE=GRAY\r\n");
}

static void app_update_image_display(void)
{
    const uint8 *image;
    uint32 frame_count;
    uint8 threshold;

    frame_count = camera_otsu_get_frame_count();
    if(frame_count == 0U)
    {
        return;
    }

    if((frame_count == displayed_frame_count) && (image_display_dirty == 0U))
    {
        return;
    }

    displayed_frame_count = frame_count;
    image_display_dirty = 0U;
    threshold = camera_otsu_get_threshold();
    image = camera_otsu_get_image();

    /* threshold 0 draws the raw gray frame; non-zero draws a binary frame. */
    if(image_binary_mode != 0U)
    {
        tft180_show_gray_image(0, APP_IMAGE_Y, image,
                               camera_otsu_get_width(),
                               camera_otsu_get_height(),
                               APP_IMAGE_WIDTH, APP_IMAGE_HEIGHT,
                               (threshold == 0U) ? (uint8)1 : threshold);
    }
    else
    {
        tft180_show_gray_image(0, APP_IMAGE_Y, image,
                               camera_otsu_get_width(),
                               camera_otsu_get_height(),
                               APP_IMAGE_WIDTH, APP_IMAGE_HEIGHT, 0U);
    }
}

static void app_uart_process_byte(uint8 data)
{
    if(data == '\r' || data == '\n')
    {
        if(uart_command_length == 0U)
        {
            return;
        }

        uart_command[uart_command_length] = '\0';
        if(strcmp(uart_command, "PING") == 0)
        {
            uart_write_string(UART_0, "PONG\r\n");
        }
        else if(strcmp(uart_command, "STATUS") == 0)
        {
            app_uart_send_status();
        }
        else
        {
            uart_write_string(UART_0, "ERR UNKNOWN_COMMAND\r\n");
        }
        uart_command_length = 0;
    }
    else if(uart_command_length < (sizeof(uart_command) - 1U))
    {
        uart_command[uart_command_length] = (char)data;
        uart_command_length++;
    }
    else
    {
        uart_command_length = 0;
        uart_write_string(UART_0, "ERR COMMAND_TOO_LONG\r\n");
    }
}

static void app_uart_task(void)
{
    uint8 buffer[16];
    uint32 length;
    uint32 index;

    length = debug_read_ring_buffer(buffer, sizeof(buffer));
    for(index = 0; index < length; index++)
    {
        app_uart_process_byte(buffer[index]);
    }
}

void app_init(void)
{
    /* Direction must be selected before init: init writes the 0x36 register
       from the current direction and refreshes the width/height limits. */
    tft180_set_dir(TFT180_CROSSWISE);
    tft180_init();
    tft180_set_color(RGB565_GREEN, RGB565_BLACK);
    tft180_clear();
    uart_command_length = 0;
    uart_write_string(UART_0, "BOOT\r\n");

    key_module_init();
    motor_init();
    speed_control_init();
    servo_init();
    chassis_init();

    image_binary_mode = 0U;
    image_display_dirty = 1U;
    displayed_frame_count = 0U;

    if(camera_otsu_init() == 0U)
    {
        uart_write_string(UART_0, "CAM OK\r\n");
    }
    else
    {
        uart_write_string(UART_0, "CAM ERR\r\n");
        tft180_show_string(0, 40, "CAM ERR");
    }

    /* The PIT scans keys and updates the motor ramp every millisecond. */
    pit_ms_init(CCU60_CH0, 1);
}

void app_task(void)
{
    key_event_enum event;

    app_uart_task();
    camera_otsu_task();
    app_update_image_display();

    event = key_module_get_event(KEY_4);
    if(event == KEY_EVT_SHORT || event == KEY_EVT_LONG)
    {
        image_binary_mode = (uint8)!image_binary_mode;
        image_display_dirty = 1U;
    }

    event = key_module_get_event(KEY_2);
    if(event == KEY_EVT_SHORT || event == KEY_EVT_LONG)
    {
        chassis_stop_immediate();
    }
}

#pragma section all restore