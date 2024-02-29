#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "temperature_sensor/temperature_sensor.h"
#include "drivers/ili9341.h"
#include "drivers/ccs811.h"
#include <lvgl.h>

struct ili9341_display display;
struct ccs811_sensor sensor;

void flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    ili9341_draw_window(&display, area->x1, area->y1, area->x2, area->y2);
    const uint32_t len = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1);
    lv_draw_sw_rgb565_swap(px_map, len);
    ili9341_draw(&display, px_map, len * 2);
    lv_disp_flush_ready(disp);
}

static void chart_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *chart = lv_event_get_target(e);
    if (code == LV_EVENT_REFR_EXT_DRAW_SIZE)
    {
        int32_t *s = lv_event_get_param(e);
        *s = LV_MAX(*s, 20);
    }
    else if (code == LV_EVENT_DRAW_POST_END)
    {
        lv_chart_series_t *ser = lv_chart_get_series_next(chart, NULL);
        while (ser)
        {
            for (uint32_t i = 0; i < lv_chart_get_point_count(chart); i++)
            {
                lv_point_t p;
                lv_chart_get_point_pos_by_id(chart, ser, i, &p);
                int32_t *y_array = lv_chart_get_y_array(chart, ser);
                int32_t value = y_array[i];

                lv_draw_label_dsc_t label_dsc;
                lv_draw_label_dsc_init(&label_dsc);
                label_dsc.font = LV_FONT_DEFAULT;

                char buf[16];
                snprintf(buf, sizeof(buf), "%ld°C", value);

                lv_point_t txt_size;
                lv_text_get_size(&txt_size, buf, label_dsc.font, label_dsc.letter_space, label_dsc.line_space, LV_COORD_MAX, label_dsc.flag);

                lv_area_t txt_area;
                txt_area.x1 = chart->coords.x1 + p.x - txt_size.x / 2;
                txt_area.x2 = chart->coords.x1 + p.x + txt_size.x / 2;
                txt_area.y1 = chart->coords.y1 + p.y - 10 - txt_size.y / 2;
                txt_area.y2 = chart->coords.y1 + p.y - 10 + txt_size.y / 2;

                label_dsc.color = lv_color_black();
                label_dsc.text = buf;
                label_dsc.text_local = true;
                lv_layer_t *layer = lv_event_get_layer(e);
                lv_draw_label(layer, &label_dsc, &txt_area);
            }
            ser = lv_chart_get_series_next(chart, ser);
        }
    }
}

int main()
{
    stdio_init_all();
    adc_init();
    temperature_sensor_init();

    ili9341_init(&display, spi0, PICO_DEFAULT_SPI_SCK_PIN, PICO_DEFAULT_SPI_TX_PIN, 20, 21, 22);
    ili9341_rotate(&display, 90, true);

    lv_init();
    lv_display_t *display = lv_display_create(SCREEN_HEIGHT, SCREEN_WIDTH);
    static lv_color_t buf1[(SCREEN_WIDTH * SCREEN_HEIGHT) / 2];
    lv_display_set_buffers(display, buf1, NULL, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, flush_cb);

    lv_obj_t *row = lv_obj_create(lv_screen_active());
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);

    lv_obj_t *col1 = lv_obj_create(row);
    lv_obj_set_style_pad_row(col1, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(col1, 0, LV_PART_MAIN);
    lv_obj_set_size(col1, lv_pct(50), lv_pct(100));
    lv_obj_set_flex_flow(col1, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *col2 = lv_obj_create(row);
    lv_obj_set_style_pad_row(col2, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(col2, 0, LV_PART_MAIN);
    lv_obj_set_size(col2, lv_pct(50), lv_pct(100));
    lv_obj_set_style_flex_main_place(col2, LV_FLEX_ALIGN_SPACE_EVENLY, LV_PART_MAIN);
    lv_obj_set_flex_flow(col2, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *location = lv_label_create(col1);
    lv_label_set_text(location, LV_SYMBOL_HOME "Berlin");

    lv_obj_t *time = lv_label_create(col1);
    lv_label_set_text(time, "13:37");
    lv_obj_set_style_text_font(time, &lv_font_montserrat_48, LV_PART_MAIN);

    lv_obj_t *date = lv_label_create(col1);
    lv_label_set_text(date, "29.02.2024");

    static lv_style_t temperature_style;
    lv_style_init(&temperature_style);
    lv_style_set_bg_color(&temperature_style, lv_palette_main(LV_PALETTE_RED));
    lv_style_set_bg_grad_color(&temperature_style, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_bg_grad_dir(&temperature_style, LV_GRAD_DIR_VER);

    static lv_style_t other_bar_style;
    lv_style_init(&other_bar_style);
    lv_style_set_bg_color(&other_bar_style, lv_palette_main(LV_PALETTE_RED));
    lv_style_set_bg_grad_color(&other_bar_style, lv_palette_main(LV_PALETTE_GREEN));
    lv_style_set_bg_grad_dir(&other_bar_style, LV_GRAD_DIR_VER);

    lv_obj_t *indicators = lv_obj_create(col1);
    lv_obj_remove_style_all(indicators);
    lv_obj_set_flex_grow(indicators, 2);
    lv_obj_set_width(indicators, lv_pct(100));
    lv_obj_set_style_flex_main_place(indicators, LV_FLEX_ALIGN_SPACE_EVENLY, LV_PART_MAIN);
    lv_obj_set_flex_flow(indicators, LV_FLEX_FLOW_ROW);

    lv_obj_t *temperature_column = lv_obj_create(indicators);
    lv_obj_remove_style_all(temperature_column);
    lv_obj_set_size(temperature_column, lv_pct(20), lv_pct(100));
    lv_obj_set_flex_flow(temperature_column, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *temperature_label = lv_label_create(temperature_column);
    lv_label_set_text(temperature_label, "Temp");

    lv_obj_t *temperature = lv_bar_create(temperature_column);
    lv_obj_add_style(temperature, &temperature_style, LV_PART_INDICATOR);
    lv_obj_set_flex_grow(temperature, 2);
    lv_obj_set_width(temperature, lv_pct(100));
    lv_bar_set_range(temperature, -20, 60);

    lv_obj_t *temperature_value = lv_label_create(temperature_column);

    lv_obj_t *eco2_column = lv_obj_create(indicators);
    lv_obj_remove_style_all(eco2_column);
    lv_obj_set_size(eco2_column, lv_pct(20), lv_pct(100));
    lv_obj_set_flex_flow(eco2_column, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *eco2_label = lv_label_create(eco2_column);
    lv_label_set_text(eco2_label, "eCO2");

    lv_obj_t *eco2 = lv_bar_create(eco2_column);
    lv_obj_add_style(eco2, &other_bar_style, LV_PART_INDICATOR);
    lv_obj_set_flex_grow(eco2, 2);
    lv_obj_set_width(eco2, lv_pct(100));
    lv_bar_set_range(eco2, 350, 700);

    lv_obj_t *eco2_value = lv_label_create(eco2_column);

    lv_obj_t *etvoc_column = lv_obj_create(indicators);
    lv_obj_remove_style_all(etvoc_column);
    lv_obj_set_size(etvoc_column, lv_pct(20), lv_pct(100));
    lv_obj_set_flex_flow(etvoc_column, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *etvoc_label = lv_label_create(etvoc_column);
    lv_label_set_text(etvoc_label, "eTVOC");

    lv_obj_t *etvoc = lv_bar_create(etvoc_column);
    lv_obj_add_style(etvoc, &other_bar_style, LV_PART_INDICATOR);
    lv_obj_set_flex_grow(etvoc, 2);
    lv_obj_set_width(etvoc, lv_pct(100));
    lv_bar_set_range(etvoc, 0, 50);

    lv_obj_t *etvoc_value = lv_label_create(etvoc_column);

    lv_obj_t *today = lv_label_create(col2);
    lv_label_set_text(today, "Today");

    const uint8_t points = 3;

    lv_obj_t *chart1 = lv_chart_create(col2);
    lv_obj_set_size(chart1, lv_pct(100), lv_pct(40));
    lv_chart_set_point_count(chart1, points);
    lv_chart_set_div_line_count(chart1, 0, 0);
    lv_obj_add_event_cb(chart1, chart_event_cb, LV_EVENT_ALL, NULL);
    lv_chart_series_t *ser1 = lv_chart_add_series(chart1, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
    for (uint32_t i = 0; i < points; i++)
    {
        lv_chart_set_next_value(chart1, ser1, lv_rand(60, 90));
    }

    lv_obj_t *tomorrow = lv_label_create(col2);
    lv_label_set_text(tomorrow, "Tomorrow");

    lv_obj_t *chart2 = lv_chart_create(col2);
    lv_obj_set_size(chart2, lv_pct(100), lv_pct(40));
    lv_chart_set_point_count(chart2, points);
    lv_chart_set_div_line_count(chart2, 0, 0);
    lv_obj_add_event_cb(chart2, chart_event_cb, LV_EVENT_ALL, NULL);
    lv_chart_series_t *ser3 = lv_chart_add_series(chart2, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_series_t *ser4 = lv_chart_add_series(chart2, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_SECONDARY_Y);
    for (uint32_t i = 0; i < points; i++)
    {
        lv_chart_set_next_value(chart2, ser3, lv_rand(60, 90));
        lv_chart_set_next_value(chart2, ser4, lv_rand(10, 40));
    }

    const uint8_t valid = ccs811_init(&sensor, i2c0, PICO_DEFAULT_I2C_SCL_PIN, PICO_DEFAULT_I2C_SDA_PIN, CCS811_DRIVE_MODE_1SEC, 0, 0);
    while (true)
    {
        if (ccs811_read_data(&sensor))
        {
            char value[16];

            const float temp_reading = temperature_sensor_read();
            lv_bar_set_value(temperature, temp_reading, LV_ANIM_ON);
            snprintf(value, sizeof(value), "%.1f°C", temp_reading);
            lv_label_set_text(temperature_value, value);

            lv_bar_set_value(eco2, sensor.data.eco2, LV_ANIM_ON);
            snprintf(value, sizeof(value), "%dppm", sensor.data.eco2);
            lv_label_set_text(eco2_value, value);

            lv_bar_set_value(etvoc, sensor.data.etvoc, LV_ANIM_ON);
            snprintf(value, sizeof(value), "%dppb", sensor.data.etvoc);
            lv_label_set_text(etvoc_value, value);
        }
        lv_timer_handler();
        lv_tick_inc(5);
        sleep_ms(5);
    }
    return 0;
}
