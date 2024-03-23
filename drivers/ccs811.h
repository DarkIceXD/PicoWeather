#ifndef _CCS811_H_
#define _CCS811_H_
#include <hardware/i2c.h>

#define CCS811_DRIVE_MODE_IDLE 0x00
#define CCS811_DRIVE_MODE_1SEC 0x01
#define CCS811_DRIVE_MODE_10SEC 0x02
#define CCS811_DRIVE_MODE_60SEC 0x03
#define CCS811_DRIVE_MODE_250MS 0x04

struct ccs811_sensor_data
{
    uint16_t voc, co2;
};

struct ccs811_sensor
{
    i2c_inst_t *i2c_port;
    struct ccs811_sensor_data data;
};

bool ccs811_init(struct ccs811_sensor *sensor, i2c_inst_t *i2c_port, const uint8_t drive_mode, const uint8_t int_data_ready, const uint8_t int_threshold);
bool ccs811_read_data(struct ccs811_sensor *sensor);
/**
 * @brief Sets environmental data for compensation of gas readings.
 *
 * This function sets the environmental data (temperature and humidity) for the CCS811 sensor.
 * These parameters are used to compensate gas readings for changes in temperature and humidity.
 *
 * @param sensor A pointer to the CCS811 sensor object.
 * @param humidity Relative humidity as a fraction (LSB = 1/512). For example, 50% RH would be 0x6400.
 * @param temperature Temperature in degrees Celsius offset by 25°C, where the LSB is 1/512. For example, 25°C would be 0x6400. Temperatures under -25°C should always be 0.
 */
void ccs811_set_env_data(const struct ccs811_sensor *sensor, const uint16_t humidity, const uint16_t temperature);

#endif