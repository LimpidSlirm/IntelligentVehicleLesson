#include "app.h"
#include "chassis.h"
#include "key_module.h"
#include "motor.h"
#include "servo.h"
#include "speed_control.h"
#include "motion_interface.h"

#pragma section all "cpu0_dsram"

#define SERVO_TEST_ANGLE_NUM  (3U)

static const int16 servo_test_angles[SERVO_TEST_ANGLE_NUM] =
{
    SERVO_NEUTRAL_ANGLE, SERVO_ANGLE_MIN, SERVO_ANGLE_MAX
};

static uint8 servo_test_index;
static char uart_command[32];
static uint8 uart_command_length;
static int16 displayed_servo_angle;

static void app_uart_show_status(const char *status)
{
    tft180_set_color(RGB565_GREEN, RGB565_BLACK);
    tft180_show_string(0, 32, "UART0 115200");
    tft180_show_string(0, 48, status);
}

static void app_uart_send_status(void)
{
    uart_write_string(UART_0, "STATUS DRIVE=DISABLED\r\n");
}

static void app_update_servo_display(void)
{
    int16 angle = servo_get_current_angle();

    displayed_servo_angle = angle;
    tft180_set_color(RGB565_GREEN, RGB565_BLACK);
    tft180_show_string(0, 64, "CMD");
    tft180_show_int(24, 64, angle, 3);
    tft180_show_string(64, 64, "TGT");
    tft180_show_int(88, 64, servo_get_target_angle(), 3);
}

static void app_uart_process_byte(uint8 data)
{
    app_uart_show_status("RX DATA");

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
            app_uart_show_status("PING OK");
        }
        else if(strcmp(uart_command, "STATUS") == 0)
        {
            app_uart_send_status();
            app_uart_show_status("STATUS OK");
        }
        else
        {
            uart_write_string(UART_0, "ERR UNKNOWN_COMMAND\r\n");
            app_uart_show_status("CMD ERR");
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
    tft180_init();
    tft180_set_color(RGB565_GREEN, RGB565_BLACK);
    tft180_show_string(0, 0, "E01 GPIO DEMO");
    tft180_show_string(0, 16, "BOOT OK");
    tft180_show_string(0, 32, "UART0 115200");
    tft180_show_string(0, 48, "WAIT PING");
    uart_command_length = 0;
    uart_write_string(UART_0, "BOOT OK\r\n");

    key_module_init();
    servo_test_index = 0U;
    motor_init();
    speed_control_init();
    chassis_init();
    servo_init();
    chassis_stop_immediate();
    displayed_servo_angle = (int16)-1;
    app_update_servo_display();

    /* The PIT scans keys and updates the motor ramp every millisecond. */
    pit_ms_init(CCU60_CH0, 1);
}

void app_task(void)
{
    key_event_enum event;

    app_uart_task();
    app_update_servo_display();

    event = key_module_get_event(KEY_4);
    if(event == KEY_EVT_SHORT)
    {
        servo_test_index++;
        if(servo_test_index >= SERVO_TEST_ANGLE_NUM)
        {
            servo_test_index = 0;
        }
        servo_set_angle(servo_test_angles[servo_test_index]);
    }
    else if(event == KEY_EVT_LONG)
    {
        chassis_stop_immediate();
        servo_center();
        servo_test_index = 1U;
    }

    event = key_module_get_event(KEY_2);
    if(event == KEY_EVT_SHORT || event == KEY_EVT_LONG)
    {
        servo_hold();
    }
}

#pragma section all restore
