#include "app.h"
#include "key_module.h"
#include "motor.h"
#include "speed_control.h"
#include "motion_interface.h"

#pragma section all "cpu0_dsram"

/* The debug key steps through closed-loop speed targets. */
#define MOTOR_SPEED_LEVEL_NUM  (5U)

static const int16 motor_speed_levels[MOTOR_SPEED_LEVEL_NUM] =
{
    0, 750, 1500, 2250, MOTOR_SPEED_MAX
};

static uint8 motor_speed_index;
static uint8 motor_reverse;
static uint8 motor_test_armed;
static motion_command_t motion_command;
static char uart_command[32];
static uint8 uart_command_length;

static void app_uart_show_status(const char *status)
{
    tft180_set_color(RGB565_GREEN, RGB565_BLACK);
    tft180_show_string(0, 32, "UART0 115200");
    tft180_show_string(0, 48, status);
}

/* Apply the selected speed and direction to both wheels. */
static void app_update_motor(void)
{
    int16 speed = motor_speed_levels[motor_speed_index];

    if(motor_reverse != 0U)
    {
        speed = (int16)-speed;
    }

    motion_command.left_target = speed;
    motion_command.right_target = speed;
    motion_command.enable = motor_test_armed;
    motion_command.safety_stop = 0U;
    speed_control_set_command(&motion_command);
}

static void app_uart_send_status(void)
{
    static const char *speed_names[MOTOR_SPEED_LEVEL_NUM] =
    {
        "STOP", "LOW", "MID", "HIGH", "MAX"
    };
    const char *direction = (motor_reverse != 0U) ? "REV" : "FWD";

    uart_write_string(UART_0, "STATUS SPEED=");
    uart_write_string(UART_0, speed_names[motor_speed_index]);
    uart_write_string(UART_0, " DIR=");
    uart_write_string(UART_0, direction);
    uart_write_string(UART_0, "\r\n");
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
    motor_speed_index = 0;
    motor_reverse = 0;
    motor_test_armed = 0;
    motion_command.left_target = 0;
    motion_command.right_target = 0;
    motion_command.enable = 0;
    motion_command.safety_stop = 1U;
    motor_init();
    speed_control_init();

    /* The PIT scans keys and updates the motor ramp every millisecond. */
    pit_ms_init(CCU60_CH0, 1);
}

void app_task(void)
{
    key_event_enum event;

    app_uart_task();

    event = key_module_get_event(KEY_3);
    if(event == KEY_EVT_SHORT)
    {
        motor_test_armed = 1;
        /* KEY3 cycles through stop and four closed-loop speed targets. */
        motor_speed_index++;
        if(motor_speed_index >= MOTOR_SPEED_LEVEL_NUM)
        {
            motor_speed_index = 0;
        }
        app_update_motor();
    }

    event = key_module_get_event(KEY_4);
    if(event == KEY_EVT_SHORT)
    {
        if(motor_test_armed == 0U)
        {
            return;
        }
        /* KEY4 short press reverses both motors. */
        motor_reverse = (uint8)!motor_reverse;
        app_update_motor();
    }
    else if(event == KEY_EVT_LONG)
    {
        motor_test_armed = 1;
        /* A long press requests a ramped stop. */
        motion_command.left_target = 0;
        motion_command.right_target = 0;
        motion_command.enable = 1U;
        motion_command.safety_stop = 0U;
        speed_control_set_command(&motion_command);
        motor_stop();
        motor_speed_index = 0;
    }
}

#pragma section all restore
