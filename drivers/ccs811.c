#include "ccs811.h"
#include <pico/stdlib.h>

#define CCS811_STATUS 0x00          /* Status register */
#define CCS811_MEAS_MODE 0x01       /* Measurement mode and conditions register */
#define CCS811_ALG_RESULT_DATA 0x02 /* Algorithm result */
#define CCS811_RAW_DATA 0x03        /* Raw ADC data values for resistance and current source used */
#define CCS811_ENV_DATA 0x05        /* Temperature and humidity data can be written to enable compensation */
#define CCS811_THRESHOLDS 0x10      /* Thresholds for operation when interrupts are only generated when eCO 2 ppm crosses a threshold */
#define CCS811_BASELINE 0x11        /* The encoded current baseline value can be read. A previously saved encoded baseline can be written. */
#define CCS811_HW_ID 0x20           /* Hardware ID */
#define CCS811_HW_VERSION 0x21      /* Hardware Version */
#define CCS811_FW_BOOT_VERSION 0x23 /* Firmware Boot Version */
#define CCS811_FW_APP_VERSION 0x24  /* Firmware Application Version */
#define CCS811_ERROR_ID 0xE0        /* Error ID */
#define CCS811_SW_RESET 0xFF        /* If the correct 4 bytes (0x11 0xE5 0x72 0x8A) are written to this register in a single sequence the device will reset and return to BOOT mode */

#define CCS811_HW_ID_CODE 0x81

#define CCS811_BOOTLOADER_APP_ERASE 0xF1
#define CCS811_BOOTLOADER_APP_DATA 0xF2
#define CCS811_BOOTLOADER_APP_VERIFY 0xF3
#define CCS811_BOOTLOADER_APP_START 0xF4

#define CCS811_STATUS_ERROR (1 << 0)
#define CCS811_STATUS_DATA_READY (1 << 3)
#define CCS811_STATUS_APP_VALID (1 << 4)
#define CCS811_STATUS_FW_MODE (1 << 7)

#define CCS811_ERROR_WRITE_REG_INVALID (1 << 0)
#define CCS811_ERROR_READ_REG_INVALID (1 << 1)
#define CCS811_ERROR_MEASMODE_INVALID (1 << 2)
#define CCS811_ERROR_MAX_RESISTANCE (1 << 3)
#define CCS811_ERROR_HEATER_FAULT (1 << 4)
#define CCS811_ERROR_HEATER_SUPPLY (1 << 5)

#define CCS811_ADDR 0x5A

static void read(i2c_inst_t *i2c_port, const uint8_t command, uint8_t *buf, const uint8_t num)
{
    i2c_write_blocking(i2c_port, CCS811_ADDR, &command, 1, true);
    i2c_read_blocking(i2c_port, CCS811_ADDR, buf, num, false);
}

static uint8_t read_byte(i2c_inst_t *i2c_port, const uint8_t command)
{
    uint8_t r;
    read(i2c_port, command, &r, 1);
    return r;
}

static void command(i2c_inst_t *i2c_port, const uint8_t buf[], const uint8_t num)
{
    i2c_write_blocking(i2c_port, CCS811_ADDR, buf, num, false);
}

bool ccs811_init(struct ccs811_sensor *sensor, i2c_inst_t *i2c_port, const uint8_t drive_mode, const uint8_t int_data_ready, const uint8_t int_threshold)
{
    sensor->i2c_port = i2c_port;

    if (read_byte(sensor->i2c_port, CCS811_HW_ID) != CCS811_HW_ID_CODE)
        return false;

    command(sensor->i2c_port, (uint8_t[]){CCS811_SW_RESET, 0x11, 0xE5, 0x72, 0x8A}, 5);
    sleep_ms(2);

    command(sensor->i2c_port, (uint8_t[]){CCS811_BOOTLOADER_APP_START}, 1);
    sleep_ms(1);

    const uint8_t status = read_byte(sensor->i2c_port, CCS811_STATUS);
    if (status & CCS811_STATUS_ERROR)
        return false;

    if (!(status & CCS811_STATUS_FW_MODE))
        return false;

    if (!(status & CCS811_STATUS_APP_VALID))
        return false;

    command(sensor->i2c_port, (uint8_t[]){CCS811_MEAS_MODE, (drive_mode << 4) | (int_data_ready << 3) | (int_threshold << 2)}, 2);
    return true;
}

bool ccs811_read_data(struct ccs811_sensor *sensor)
{
    uint8_t buf[5];
    read(sensor->i2c_port, CCS811_ALG_RESULT_DATA, buf, 5);
    const uint8_t status = buf[4];
    if (status & CCS811_STATUS_ERROR)
        return false;
    if (!(status & CCS811_STATUS_DATA_READY))
        return false;

    sensor->data.co2 = ((uint16_t)buf[0] << 8) | ((uint16_t)buf[1]);
    sensor->data.voc = ((uint16_t)buf[2] << 8) | ((uint16_t)buf[3]);
    return true;
}

void ccs811_set_env_data(const struct ccs811_sensor *sensor, const uint16_t humidity, const uint16_t temperature)
{
    command(sensor->i2c_port, (uint8_t[]){CCS811_ENV_DATA, humidity >> 8, humidity & 0xff, temperature >> 8, temperature & 0xff}, 5);
}