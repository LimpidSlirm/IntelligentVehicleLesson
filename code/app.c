#include "app.h"
#include "chassis.h"
#include "key.h"
#include "motor.h"
#include "servo.h"
#include "speed.h"
#include "wifi_spi.h"
#include "seekfree_assistant.h"
#include "zf_device_mt9v03x.h"

#pragma section all "cpu0_dsram"

// Network settings: the IP below is the PC address of the TCP server opened
// by the SeekFree assistant. Re-flash after changing it.
#define APP_WIFI_SSID       ("Mi9 Pro 5G")
#define APP_WIFI_PASSWORD   ("20070510")
#define APP_TARGET_IP       ("192.168.100.8")
#define APP_TARGET_PORT     ("9086")
#define APP_LOCAL_PORT      ("6666")

// 6x8 font: 128 x 160 portrait, 21 characters per line.
#define TFT_DEBUG_LINE_HEIGHT   (10U)
#define TFT_DEBUG_LINE_CHARS    (21U)

// The frame is copied before sending so a new frame cannot tear the one
// that is being transmitted.
uint8 image_copy[MT9V03X_H][MT9V03X_W];

static seekfree_assistant_camera_struct camera_information;
static char tft_debug_text[40];
static volatile uint32 camera_frame_count;
static volatile uint8 one_second_flag;

// Clear the whole line before writing, truncate long text: writing past the
// right edge would trip the driver assertion.
static void debug_tft_show(uint8 line, const char *text)
{
    char buffer[TFT_DEBUG_LINE_CHARS + 1U];
    uint8 index = 0U;

    for(index = 0U; TFT_DEBUG_LINE_CHARS > index; index++)
    {
        buffer[index] = ' ';
    }
    buffer[TFT_DEBUG_LINE_CHARS] = '\0';
    tft180_show_string(0, line * TFT_DEBUG_LINE_HEIGHT, buffer);

    index = 0U;
    while(('\0' != text[index]) && (TFT_DEBUG_LINE_CHARS > index))
    {
        buffer[index] = text[index];
        index++;
    }
    buffer[index] = '\0';
    tft180_show_string(0, line * TFT_DEBUG_LINE_HEIGHT, buffer);
}

// The WiFi module shares SPI_3 with nothing else, but its init must stay
// before motor/encoder/servo init: their pin setup breaks the SPI transfer.
static void app_wifi_init(void)
{
    while(wifi_init(APP_WIFI_SSID, APP_WIFI_PASSWORD))
    {
        uart_write_string(UART_0, "WIFI FAIL\r\n");
        debug_tft_show(1, "WiFi: connect fail");
        debug_tft_show(2, "WiFi: retry ...");
        system_delay_ms(100);
    }
    uart_write_string(UART_0, "WIFI OK\r\n");
    debug_tft_show(1, "WiFi: connected");
    debug_tft_show(2, "");

    uart_write_string(UART_0, "VER:");
    uart_write_string(UART_0, wifi_version);
    uart_write_string(UART_0, "\r\nMAC:");
    uart_write_string(UART_0, wifi_mac_addr);
    uart_write_string(UART_0, "\r\nIP :");
    uart_write_string(UART_0, wifi_ip_addr_port);
    uart_write_string(UART_0, "\r\n");

    sprintf(tft_debug_text, "Ver :%s", wifi_version);
    debug_tft_show(3, tft_debug_text);
    sprintf(tft_debug_text, "MAC :%s", wifi_mac_addr);
    debug_tft_show(4, tft_debug_text);
    sprintf(tft_debug_text, "IP  :%s", wifi_ip_addr_port);
    debug_tft_show(5, tft_debug_text);

    while(wifi_socket_connect("TCP", APP_TARGET_IP, APP_TARGET_PORT, APP_LOCAL_PORT))
    {
        uart_write_string(UART_0, "TCP FAIL\r\n");
        debug_tft_show(6, "TCP : connect fail");
        debug_tft_show(7, "TCP : retry ...");
        system_delay_ms(100);
    }
    uart_write_string(UART_0, "TCP OK\r\n");
    debug_tft_show(6, "TCP : connected");
    debug_tft_show(7, "");

    seekfree_assistant_interface_init(SEEKFREE_ASSISTANT_WIFI_SPI);
}

static void app_camera_init(void)
{
    while(mt9v03x_init())
    {
        uart_write_string(UART_0, "CAM FAIL\r\n");
        debug_tft_show(7, "CAM : init fail");
        debug_tft_show(8, "CAM : retry ...");
        system_delay_ms(100);
    }
    uart_write_string(UART_0, "CAM OK\r\n");
    debug_tft_show(7, "CAM : init OK");
    debug_tft_show(8, "");

    seekfree_assistant_camera_config(&camera_information,
                                     SEEKFREE_ASSISTANT_CAMERA_TYPE_MT9V03X,
                                     MT9V03X_W, MT9V03X_H, image_copy[0]);
}

void app_pit_1ms(void)
{
    static uint16 tick;

    tick++;
    if(tick >= 1000U)
    {
        tick = 0U;
        one_second_flag = 1U;
    }
}

void app_init(void)
{
    tft180_init();
    tft180_set_font(TFT180_6X8_FONT);
    tft180_set_color(RGB565_GREEN, RGB565_BLACK);
    tft180_clear();
    debug_tft_show(0, "--- WIFI CAMERA ---");

    app_wifi_init();
    key_module_init();
    motor_init();
    speed_control_init();
    servo_init();
    chassis_init();

    app_camera_init();
    debug_tft_show(9, "IMG : streaming");

    // The PIT scans keys and updates the motor ramp every millisecond.
    pit_ms_init(CCU60_CH0, 1);
}

void app_task(void)
{
    key_event_enum event;

    if(mt9v03x_finish_flag)
    {
        mt9v03x_finish_flag = 0U;
        memcpy(image_copy[0], mt9v03x_image[0], MT9V03X_IMAGE_SIZE);
        seekfree_assistant_camera_send(&camera_information);
        camera_frame_count++;
    }

    if(one_second_flag)
    {
        one_second_flag = 0U;
        sprintf(tft_debug_text, "FPS :%d", (int)camera_frame_count);
        debug_tft_show(9, tft_debug_text);
        camera_frame_count = 0U;
    }

    event = key_module_get_event(KEY_2);
    if(event == KEY_EVT_SHORT || event == KEY_EVT_LONG)
    {
        chassis_stop_immediate();
    }
}

#pragma section all restore
