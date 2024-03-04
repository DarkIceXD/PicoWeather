#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "temperature_sensor/temperature_sensor.h"
#include "drivers/ili9341.h"
#include "drivers/ccs811.h"
#include "ui/ui.h"

struct ili9341_display display;
struct ccs811_sensor sensor;
struct ui ui;

static void flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    ili9341_draw_window(&display, area->x1, area->y1, area->x2, area->y2);
    const uint32_t len = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1);
    lv_draw_sw_rgb565_swap(px_map, len);
    ili9341_draw(&display, px_map, len * 2);
    lv_disp_flush_ready(disp);
}

int main()
{
    stdio_init_all();
    adc_init();
    temperature_sensor_init();

    ili9341_init(&display, spi0, PICO_DEFAULT_SPI_SCK_PIN, PICO_DEFAULT_SPI_TX_PIN, 20, 21, 22);
    ili9341_rotate(&display, 90, true);

    const uint8_t valid = ccs811_init(&sensor, i2c0, PICO_DEFAULT_I2C_SCL_PIN, PICO_DEFAULT_I2C_SDA_PIN, CCS811_DRIVE_MODE_1SEC, 0, 0);
    ui_init(&ui, display.width, display.height, flush_cb);

    while (true)
    {
        if (ccs811_read_data(&sensor))
        {
            char value[20];

            const float temp_reading = temperature_sensor_read();
            lv_bar_set_value(ui.temperature, temp_reading, LV_ANIM_OFF);
            snprintf(value, sizeof(value), "%.1f", temp_reading);
            lv_label_set_text(ui.temperature_value, value);

            lv_bar_set_value(ui.eco2, sensor.data.eco2, LV_ANIM_OFF);
            snprintf(value, sizeof(value), "%d", sensor.data.eco2);
            lv_label_set_text(ui.eco2_value, value);

            lv_bar_set_value(ui.etvoc, sensor.data.etvoc, LV_ANIM_OFF);
            snprintf(value, sizeof(value), "%d", sensor.data.etvoc);
            lv_label_set_text(ui.etvoc_value, value);
        }
        lv_timer_handler();
        lv_tick_inc(5);
        sleep_ms(5);
    }
    return 0;
}
