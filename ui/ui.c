#include "ui.h"
#include <stdio.h>

static lv_color_t buf1[(240 * 320) / 10];

static void create_indicator(lv_obj_t *parent, const char *label, const char *unit, const int32_t min, const int32_t max, lv_obj_t **bar, lv_obj_t **value)
{
    lv_obj_t *column = lv_obj_create(parent);
    lv_obj_remove_style_all(column);
    lv_obj_set_size(column, lv_pct(16), lv_pct(100));
    lv_obj_set_flex_flow(column, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_flex_cross_place(column, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN);

    lv_obj_t *label_obj = lv_label_create(column);
    lv_label_set_text_static(label_obj, label);

    *bar = lv_bar_create(column);
    lv_obj_set_flex_grow(*bar, 1);
    lv_obj_set_width(*bar, lv_pct(50));
    lv_bar_set_range(*bar, min, max);

    *value = lv_label_create(column);
    lv_obj_t *unit_obj = lv_label_create(column);
    lv_label_set_text_static(unit_obj, unit);
}

static void chart_event_cb(lv_event_t *e)
{
    const lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *chart = lv_event_get_target(e);
    if (code == LV_EVENT_REFR_EXT_DRAW_SIZE)
    {
        int32_t *s = lv_event_get_param(e);
        *s = LV_MAX(*s, 20);
    }
    else if (code == LV_EVENT_DRAW_POST_END)
    {
        const char *unit = lv_event_get_user_data(e);
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
                snprintf(buf, sizeof(buf), "%ld%s", value, unit);

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

static void create_chart(lv_obj_t *parent, const char *label, const uint32_t points, lv_obj_t **chart, lv_chart_series_t **series)
{
    lv_obj_t *wrapper = lv_obj_create(parent);
    lv_obj_remove_style_all(wrapper);
    lv_obj_set_size(wrapper, lv_pct(300), lv_pct(50));
    lv_obj_set_flex_flow(wrapper, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *label_text = lv_label_create(wrapper);
    lv_label_set_text(label_text, label);

    *chart = lv_chart_create(wrapper);
    lv_obj_set_flex_grow(*chart, 1);
    lv_obj_set_width(*chart, lv_pct(100));
    lv_chart_set_point_count(*chart, points);
    lv_chart_set_div_line_count(*chart, 0, 0);
    lv_obj_add_event_cb(*chart, chart_event_cb, LV_EVENT_ALL, "°C");

    lv_obj_t *scale = lv_scale_create(wrapper);
    lv_scale_set_mode(scale, LV_SCALE_MODE_HORIZONTAL_BOTTOM);
    lv_obj_set_size(scale, lv_pct(100), 25);
    lv_scale_set_total_tick_count(scale, points);
    lv_scale_set_major_tick_every(scale, 1);
    lv_scale_set_range(scale, 0, points);
    lv_obj_set_style_pad_hor(scale, lv_chart_get_first_point_center_offset(*chart), 0);

    *series = lv_chart_add_series(*chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
}

void ui_init(struct ui *ui, const uint32_t horizontal_resolution, const uint32_t vertical_resolution, const lv_display_flush_cb_t flush_cb, const lv_indev_read_cb_t read_cb)
{
    lv_init();
    lv_display_t *display = lv_display_create(horizontal_resolution, vertical_resolution);
    lv_display_set_buffers(display, buf1, NULL, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, flush_cb);

    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, read_cb);

    lv_obj_t *row = lv_obj_create(lv_screen_active());
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *col1 = lv_obj_create(row);
    lv_obj_set_style_pad_row(col1, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(col1, 0, LV_PART_MAIN);
    lv_obj_set_size(col1, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(col1, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *col2 = lv_obj_create(row);
    lv_obj_set_style_pad_row(col2, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(col2, 0, LV_PART_MAIN);
    lv_obj_set_size(col2, lv_pct(100), lv_pct(150));
    lv_obj_set_flex_flow(col2, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_flex_main_place(col2, LV_FLEX_ALIGN_SPACE_EVENLY, LV_PART_MAIN);

    ui->date = lv_label_create(col1);
    ui->time = lv_label_create(col1);
    lv_obj_set_style_text_font(ui->time, &lv_font_montserrat_48, LV_PART_MAIN);

    lv_obj_t *location = lv_label_create(col1);
    lv_label_set_text(location, LV_SYMBOL_HOME "Berlin");

    lv_obj_t *indicators = lv_obj_create(col1);
    lv_obj_remove_style_all(indicators);
    lv_obj_set_flex_grow(indicators, 1);
    lv_obj_set_width(indicators, lv_pct(100));
    lv_obj_set_flex_flow(indicators, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(indicators, LV_FLEX_ALIGN_SPACE_EVENLY, LV_PART_MAIN);

    create_indicator(indicators, "Temp", "°C", 0, 40, &ui->temperature, &ui->temperature_value);
    create_indicator(indicators, "Hum", "%RH", 0, 100, &ui->humidity, &ui->humidity_value);
    create_indicator(indicators, "Pres", "hPa", 950, 1050, &ui->pressure, &ui->pressure_value);
    create_indicator(indicators, "CO2", "ppm", 400, 1000, &ui->co2, &ui->co2_value);
    create_indicator(indicators, "VOC", "ppb", 0, 100, &ui->voc, &ui->voc_value);

    create_chart(col2, "Today", 24, &ui->chart[0], &ui->chart_series[0]);
    create_chart(col2, "Tomorrow", 24, &ui->chart[1], &ui->chart_series[1]);
    // create_chart(col2, "", 24, &ui->chart[2], &ui->chart_series[2]);
}