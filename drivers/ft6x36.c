#include "ft6x36.h"
#include <pico/stdlib.h>

#define FT6X36_DEVICE_MODE 0x00
#define FT6X36_GESTURE_ID 0x01
#define FT6X36_NUM_TOUCHES 0x02
#define FT6X36_P1_XH 0x03
#define FT6X36_P1_XL 0x04
#define FT6X36_P1_YH 0x05
#define FT6X36_P1_YL 0x06
#define FT6X36_P1_WEIGHT 0x07
#define FT6X36_P1_MISC 0x08
#define FT6X36_P2_XH 0x09
#define FT6X36_P2_XL 0x0A
#define FT6X36_P2_YH 0x0B
#define FT6X36_P2_YL 0x0C
#define FT6X36_P2_WEIGHT 0x0D
#define FT6X36_P2_MISC 0x0E
#define FT6X36_THRESHHOLD 0x80
#define FT6X36_FILTER_COEF 0x85
#define FT6X36_CTRL 0x86
#define FT6X36_TIME_ENTER_MONITOR 0x87
#define FT6X36_TOUCHRATE_ACTIVE 0x88
#define FT6X36_TOUCHRATE_MONITOR 0x89
#define FT6X36_RADIAN_VALUE 0x91
#define FT6X36_OFFSET_LEFT_RIGHT 0x92
#define FT6X36_OFFSET_UP_DOWN 0x93
#define FT6X36_DISTANCE_LEFT_RIGHT 0x94
#define FT6X36_DISTANCE_UP_DOWN 0x95
#define FT6X36_DISTANCE_ZOOM 0x96
#define FT6X36_LIB_VERSION_H 0xA1
#define FT6X36_LIB_VERSION_L 0xA2
#define FT6X36_CHIPID 0xA3
#define FT6X36_INTERRUPT_MODE 0xA4
#define FT6X36_POWER_MODE 0xA5
#define FT6X36_FIRMWARE_VERSION 0xA6
#define FT6X36_PANEL_ID 0xA8
#define FT6X36_STATE 0xBC

#define FT6X36_VENDID 0x11
#define FT6206_CHIPID 0x06
#define FT6236_CHIPID 0x36
#define FT6336_CHIPID 0x64

#define FT6X36_ADDR 0x38

static void read(i2c_inst_t *i2c_port, const uint8_t command, uint8_t *buf, const uint8_t num)
{
    i2c_write_blocking(i2c_port, FT6X36_ADDR, &command, 1, true);
    i2c_read_blocking(i2c_port, FT6X36_ADDR, buf, num, false);
}

static uint8_t read_byte(i2c_inst_t *i2c_port, const uint8_t command)
{
    uint8_t r;
    read(i2c_port, command, &r, 1);
    return r;
}

static void command(i2c_inst_t *i2c_port, const uint8_t buf[], const uint8_t num)
{
    i2c_write_blocking(i2c_port, FT6X36_ADDR, buf, num, false);
}

bool ft6x36_init(struct ft6x36_touch *touch, i2c_inst_t *i2c_port, const uint8_t threshold, const bool is_rotated)
{
    touch->i2c_port = i2c_port;
    touch->is_rotated = is_rotated;

    if (read_byte(touch->i2c_port, FT6X36_PANEL_ID) != FT6X36_VENDID)
        return false;

    const uint8_t chip_id = read_byte(touch->i2c_port, FT6X36_CHIPID);
    if (chip_id != FT6206_CHIPID && chip_id != FT6236_CHIPID && chip_id != FT6336_CHIPID)
        return false;

    command(touch->i2c_port, (uint8_t[]){FT6X36_DEVICE_MODE, 0x00}, 2);
    command(touch->i2c_port, (uint8_t[]){FT6X36_THRESHHOLD, threshold}, 2);
    command(touch->i2c_port, (uint8_t[]){FT6X36_TOUCHRATE_ACTIVE, 0x0E}, 2);
    return true;
}

static void read_touch_point(struct ft6x36_point *point, const uint8_t data[], const bool is_rotated)
{
    point->event = data[0] >> 6;
    if (!is_rotated)
    {
        point->x = ((data[0] & 0x0F) << 8) | data[1];
        point->y = ((data[2] & 0x0F) << 8) | data[3];
    }
    else
    {
        point->y = ((data[0] & 0x0F) << 8) | data[1];
        point->x = ((data[2] & 0x0F) << 8) | data[3];
    }
}

uint8_t ft6x36_read_data(struct ft6x36_touch *touch)
{
    const uint8_t data_size = FT6X36_P2_WEIGHT - FT6X36_NUM_TOUCHES;
    uint8_t data[data_size];
    read(touch->i2c_port, FT6X36_NUM_TOUCHES, data, data_size);
    touch->data.touch_count = data[0] < 2 ? data[0] : 2;
    for (uint8_t i = 0; i < touch->data.touch_count; i++)
        read_touch_point(&touch->data.touch_points[i], data + (FT6X36_P1_XH - FT6X36_NUM_TOUCHES) + (FT6X36_P2_XH - FT6X36_P1_XH) * i, touch->is_rotated);
    return touch->data.touch_count;
}