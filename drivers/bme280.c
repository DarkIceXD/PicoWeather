#include "bme280.h"
#include <pico/stdlib.h>

#define BME280_REG_CHIP_ID 0xD0
#define BME280_REG_RESET 0xE0
#define BME280_REG_TEMP_PRESS_CALIB_DATA 0x88
#define BME280_REG_HUMIDITY_CALIB_DATA 0xE1
#define BME280_REG_CTRL_HUM 0xF2
#define BME280_REG_STATUS 0xF3
#define BME280_REG_PWR_CTRL 0xF4
#define BME280_REG_CTRL_MEAS 0xF4
#define BME280_REG_CONFIG 0xF5
#define BME280_REG_DATA 0xF7

#define BME280_MEAS_OFFSET 1250
#define BME280_MEAS_DUR 2300
#define BME280_PRES_HUM_MEAS_OFFSET 575

#define BME280_ADDR 0x76

#define BME280_CONCAT_BYTES(msb, lsb) (((uint16_t)msb << 8) | (uint16_t)lsb)
#define BME280_LEN_TEMP_PRESS_CALIB_DATA 26
#define BME280_LEN_HUMIDITY_CALIB_DATA 7
#define BME280_LEN_P_T_H_DATA 8

static void read(i2c_inst_t *i2c_port, const uint8_t command, uint8_t *buf, const uint8_t num)
{
    i2c_write_blocking(i2c_port, BME280_ADDR, &command, 1, true);
    i2c_read_blocking(i2c_port, BME280_ADDR, buf, num, false);
}

static uint8_t read_byte(i2c_inst_t *i2c_port, const uint8_t command)
{
    uint8_t r;
    read(i2c_port, command, &r, 1);
    return r;
}

static void command(i2c_inst_t *i2c_port, const uint8_t buf[], const uint8_t num)
{
    i2c_write_blocking(i2c_port, BME280_ADDR, buf, num, false);
}

static void parse_temp_press_calib_data(struct bme280_calib_data *calib_data, const uint8_t *reg_data)
{
    calib_data->dig_t1 = BME280_CONCAT_BYTES(reg_data[1], reg_data[0]);
    calib_data->dig_t2 = (int16_t)BME280_CONCAT_BYTES(reg_data[3], reg_data[2]);
    calib_data->dig_t3 = (int16_t)BME280_CONCAT_BYTES(reg_data[5], reg_data[4]);
    calib_data->dig_p1 = BME280_CONCAT_BYTES(reg_data[7], reg_data[6]);
    calib_data->dig_p2 = (int16_t)BME280_CONCAT_BYTES(reg_data[9], reg_data[8]);
    calib_data->dig_p3 = (int16_t)BME280_CONCAT_BYTES(reg_data[11], reg_data[10]);
    calib_data->dig_p4 = (int16_t)BME280_CONCAT_BYTES(reg_data[13], reg_data[12]);
    calib_data->dig_p5 = (int16_t)BME280_CONCAT_BYTES(reg_data[15], reg_data[14]);
    calib_data->dig_p6 = (int16_t)BME280_CONCAT_BYTES(reg_data[17], reg_data[16]);
    calib_data->dig_p7 = (int16_t)BME280_CONCAT_BYTES(reg_data[19], reg_data[18]);
    calib_data->dig_p8 = (int16_t)BME280_CONCAT_BYTES(reg_data[21], reg_data[20]);
    calib_data->dig_p9 = (int16_t)BME280_CONCAT_BYTES(reg_data[23], reg_data[22]);
    calib_data->dig_h1 = reg_data[25];
}

static void parse_humidity_calib_data(struct bme280_calib_data *calib_data, const uint8_t *reg_data)
{
    calib_data->dig_h2 = (int16_t)BME280_CONCAT_BYTES(reg_data[1], reg_data[0]);
    calib_data->dig_h3 = reg_data[2];
    calib_data->dig_h4 = ((int16_t)(int8_t)reg_data[3] * 16) | ((int16_t)(reg_data[4] & 0x0F));
    calib_data->dig_h5 = ((int16_t)(int8_t)reg_data[5] * 16) | ((int16_t)(reg_data[4] >> 4));
    calib_data->dig_h6 = (int8_t)reg_data[6];
}

bool bme280_init(struct bme280_sensor *sensor, i2c_inst_t *i2c_port)
{
    sensor->i2c_port = i2c_port;
    if (read_byte(sensor->i2c_port, BME280_REG_CHIP_ID) != 0x60)
        return false;

    command(sensor->i2c_port, (uint8_t[]){BME280_REG_RESET, 0xB6}, 2);
    uint8_t status;
    uint8_t try_run = 5;
    do
    {
        sleep_ms(2);
        status = read_byte(sensor->i2c_port, BME280_REG_STATUS);
    } while ((try_run--) && (status & (1 << 0)));

    if (status & (1 << 0))
        return false;

    uint8_t calib_data[BME280_LEN_TEMP_PRESS_CALIB_DATA];

    read(sensor->i2c_port, BME280_REG_TEMP_PRESS_CALIB_DATA, calib_data, BME280_LEN_TEMP_PRESS_CALIB_DATA);
    parse_temp_press_calib_data(&sensor->calib_data, calib_data);

    read(sensor->i2c_port, BME280_REG_HUMIDITY_CALIB_DATA, calib_data, BME280_LEN_HUMIDITY_CALIB_DATA);
    parse_humidity_calib_data(&sensor->calib_data, calib_data);

    bme280_set_sampling(sensor, BME280_POWERMODE_NORMAL, BME280_OVERSAMPLING_1X, BME280_OVERSAMPLING_1X, BME280_OVERSAMPLING_1X, BME280_FILTER_COEFF_OFF, BME280_STANDBY_TIME_1000_MS);
    return true;
}

static uint8_t map_sampling_option(const uint8_t oversampling)
{
    switch (oversampling)
    {
    case BME280_NO_OVERSAMPLING:
        return 0;
    case BME280_OVERSAMPLING_1X:
        return 1;
    case BME280_OVERSAMPLING_2X:
        return 2;
    case BME280_OVERSAMPLING_4X:
        return 4;
    case BME280_OVERSAMPLING_8X:
        return 8;
    default:
        return 16;
    }
}

static uint32_t calculate_measure_delay(const uint8_t temp_sampling, const uint8_t press_sampling, const uint8_t hum_sampling)
{
    return BME280_MEAS_OFFSET + BME280_MEAS_DUR * map_sampling_option(temp_sampling) + BME280_MEAS_DUR * map_sampling_option(press_sampling) + BME280_PRES_HUM_MEAS_OFFSET + BME280_MEAS_DUR * map_sampling_option(hum_sampling) + BME280_PRES_HUM_MEAS_OFFSET;
}

void bme280_set_sampling(struct bme280_sensor *sensor, const uint8_t sensor_mode, const uint8_t temp_sampling, const uint8_t press_sampling, const uint8_t hum_sampling, const uint8_t filter, const uint8_t standby_duration)
{
    command(sensor->i2c_port, (uint8_t[]){BME280_REG_PWR_CTRL, BME280_POWERMODE_SLEEP}, 2);
    command(sensor->i2c_port, (uint8_t[]){BME280_REG_CTRL_HUM, hum_sampling}, 2);
    command(sensor->i2c_port, (uint8_t[]){BME280_REG_CONFIG, (standby_duration << 5) | (filter << 2)}, 2);
    command(sensor->i2c_port, (uint8_t[]){BME280_REG_CTRL_MEAS, (temp_sampling << 5) | (press_sampling << 2) | sensor_mode}, 2);
    sensor->measurement_delay = calculate_measure_delay(temp_sampling, press_sampling, hum_sampling);
    sensor->next_measurement = 0;
}

static int32_t compensate_temperature(const uint32_t uncompensated_data, struct bme280_calib_data *calib_data)
{
    int32_t var1, var2, var3, T;
    var1 = (((uncompensated_data >> 3) - ((int32_t)calib_data->dig_t1 << 1)) * ((int32_t)calib_data->dig_t2)) >> 11;
    var2 = (uncompensated_data >> 4) - ((int32_t)calib_data->dig_t1);
    var3 = (((var2 * var2) >> 12) * ((int32_t)calib_data->dig_t3)) >> 14;
    calib_data->t_fine = var1 + var3;
    T = (calib_data->t_fine * 5 + 128) >> 8;
    return T;
}

static uint32_t compensate_pressure(const uint32_t uncompensated_data, const struct bme280_calib_data *calib_data)
{
    int64_t var1, var2, p;
    var1 = ((int64_t)calib_data->t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calib_data->dig_p6;
    var2 = var2 + ((var1 * (int64_t)calib_data->dig_p5) << 17);
    var2 = var2 + (((int64_t)calib_data->dig_p4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib_data->dig_p3) >> 8) + ((var1 * ((int64_t)calib_data->dig_p2) << 12));
    var1 = ((((int64_t)1) << 47) + var1) * ((int64_t)calib_data->dig_p1) >> 33;
    if (var1 == 0)
        return 0;

    p = 1048576 - uncompensated_data;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calib_data->dig_p9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)calib_data->dig_p8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)calib_data->dig_p7) << 4);
    return p;
}

static uint32_t compensate_humidity(const uint32_t uncompensated_data, const struct bme280_calib_data *calib_data)
{
    int32_t var1;
    var1 = calib_data->t_fine - ((int32_t)76800);
    var1 = ((((uncompensated_data << 14) - (((int32_t)calib_data->dig_h4) << 20) - (((int32_t)calib_data->dig_h5) * var1)) + ((int32_t)16384)) >> 15) * (((((((var1 * ((int32_t)calib_data->dig_h6)) >> 10) * (((var1 * ((int32_t)calib_data->dig_h3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)calib_data->dig_h2) + 8192) >> 14);
    var1 = (var1 - (((((var1 >> 15) * (var1 >> 15)) >> 7) * ((int32_t)calib_data->dig_h1)) >> 4));
    if (var1 < 0)
        return 0;
    if (var1 > 419430400)
        return 419430400 >> 12;
    return var1 >> 12;
}

bool bme280_read_data(struct bme280_sensor *sensor)
{
    if (sensor->next_measurement > to_ms_since_boot(get_absolute_time()))
        return false;

    sensor->next_measurement = to_ms_since_boot(make_timeout_time_ms(sensor->measurement_delay));

    uint8_t reg_data[BME280_LEN_P_T_H_DATA];
    read(sensor->i2c_port, BME280_REG_DATA, reg_data, BME280_LEN_P_T_H_DATA);

    const uint32_t temperature = ((uint32_t)reg_data[3] << 12) | ((uint32_t)reg_data[4] << 4) | ((uint32_t)reg_data[5] >> 4);
    sensor->data.temperature = compensate_temperature(temperature, &sensor->calib_data);
    const uint32_t pressure = ((uint32_t)reg_data[0] << 12) | ((uint32_t)reg_data[1] << 4) | ((uint32_t)reg_data[2] >> 4);
    sensor->data.pressure = compensate_pressure(pressure, &sensor->calib_data);
    const uint32_t humidity = ((uint32_t)reg_data[6] << 8) | ((uint32_t)reg_data[7]);
    sensor->data.humidity = compensate_humidity(humidity, &sensor->calib_data);
    return true;
}