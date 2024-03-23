#ifndef _BME280_H_
#define _BME280_H_
#include <hardware/i2c.h>

#define BME280_POWERMODE_SLEEP 0x00
#define BME280_POWERMODE_FORCED 0x01
#define BME280_POWERMODE_NORMAL 0x03

#define BME280_NO_OVERSAMPLING 0x00
#define BME280_OVERSAMPLING_1X 0x01
#define BME280_OVERSAMPLING_2X 0x02
#define BME280_OVERSAMPLING_4X 0x03
#define BME280_OVERSAMPLING_8X 0x04
#define BME280_OVERSAMPLING_16X 0x05
#define BME280_OVERSAMPLING_MAX 16

#define BME280_FILTER_COEFF_OFF 0x00
#define BME280_FILTER_COEFF_2 0x01
#define BME280_FILTER_COEFF_4 0x02
#define BME280_FILTER_COEFF_8 0x03
#define BME280_FILTER_COEFF_16 0x04

#define BME280_STANDBY_TIME_0_5_MS 0x00
#define BME280_STANDBY_TIME_62_5_MS 0x01
#define BME280_STANDBY_TIME_125_MS 0x02
#define BME280_STANDBY_TIME_250_MS 0x03
#define BME280_STANDBY_TIME_500_MS 0x04
#define BME280_STANDBY_TIME_1000_MS 0x05
#define BME280_STANDBY_TIME_10_MS 0x06
#define BME280_STANDBY_TIME_20_MS 0x07

struct bme280_calib_data
{
    uint16_t dig_t1;
    int16_t dig_t2;
    int16_t dig_t3;
    uint16_t dig_p1;
    int16_t dig_p2;
    int16_t dig_p3;
    int16_t dig_p4;
    int16_t dig_p5;
    int16_t dig_p6;
    int16_t dig_p7;
    int16_t dig_p8;
    int16_t dig_p9;
    uint8_t dig_h1;
    int16_t dig_h2;
    uint8_t dig_h3;
    int16_t dig_h4;
    int16_t dig_h5;
    int8_t dig_h6;
    int32_t t_fine;
};

struct bme280_sensor_data
{
    /**Value of "5123" represents 51.23 DegC.*/
    int32_t temperature;
    /**Value of "47445" represents 47445/1024 = 46.333 %RH.*/
    uint32_t humidity;
    /**Value of "24674867" represents 24674867/256 = 96386.2 Pa = 963.862 hPa.*/
    uint32_t pressure;
};

struct bme280_sensor
{
    i2c_inst_t *i2c_port;
    uint32_t cycle_delay;
    uint32_t next_measurement;
    struct bme280_calib_data calib_data;
    struct bme280_sensor_data data;
};

bool bme280_init(struct bme280_sensor *sensor, i2c_inst_t *i2c_port);
void bme280_set_sampling(struct bme280_sensor *sensor, const uint8_t sensor_mode, const uint8_t temp_sampling, const uint8_t press_sampling, const uint8_t hum_sampling, const uint8_t filter, const uint8_t standby_duration);
bool bme280_read_data(struct bme280_sensor *sensor);

#endif