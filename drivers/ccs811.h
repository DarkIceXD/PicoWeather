#ifndef _CCS811_H_
#define _CCS811_H_
#include "hardware/i2c.h"

#define CCS811_DRIVE_MODE_IDLE 0x00
#define CCS811_DRIVE_MODE_1SEC 0x01
#define CCS811_DRIVE_MODE_10SEC 0x02
#define CCS811_DRIVE_MODE_60SEC 0x03
#define CCS811_DRIVE_MODE_250MS 0x04

struct ccs811_sensor_data
{
    uint16_t etvoc, eco2;
};

struct ccs811_sensor
{
    i2c_inst_t *i2c_port;
    struct ccs811_sensor_data data;
};

uint8_t ccs811_init(struct ccs811_sensor *sensor, i2c_inst_t *i2c_port, const uint scl, const uint sda, const uint8_t drive_mode, const uint8_t int_data_ready, const uint8_t int_threshold);
uint8_t ccs811_read_data(struct ccs811_sensor *sensor);

#endif