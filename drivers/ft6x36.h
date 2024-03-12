#ifndef _FT6X36_H_
#define _FT6X36_H_
#include "hardware/i2c.h"

struct ft6x36_point
{
    uint8_t event;
    uint16_t x, y;
};

struct ft6x36_touch_data
{
    uint8_t touch_count;
    struct ft6x36_point touch_points[2];
};

struct ft6x36_touch
{
    i2c_inst_t *i2c_port;
    bool is_rotated;
    struct ft6x36_touch_data data;
};

uint8_t ft6x36_init(struct ft6x36_touch *touch, i2c_inst_t *i2c_port, const uint scl, const uint sda, const uint8_t threshold, const bool is_rotated);
uint8_t ft6x36_read_data(struct ft6x36_touch *touch);

#endif