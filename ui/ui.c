#include "ui.h"
#include <stdio.h>

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

static lv_color_t buf1[(240 * 320) / 10];

void ui_init(struct ui *ui, const uint32_t horizontal_resolution, const uint32_t vertical_resolution, const lv_display_flush_cb_t flush_cb)
{
    lv_init();
    lv_display_t *display = lv_display_create(horizontal_resolution, vertical_resolution);
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
    lv_obj_set_flex_flow(col2, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_flex_main_place(col2, LV_FLEX_ALIGN_SPACE_EVENLY, LV_PART_MAIN);

    ui->date = lv_label_create(col1);
    ui->time = lv_label_create(col1);
    lv_obj_set_style_text_font(ui->time, &lv_font_montserrat_48, LV_PART_MAIN);

    lv_obj_t *location = lv_label_create(col1);
    lv_label_set_text(location, LV_SYMBOL_HOME "Berlin");

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
    lv_obj_set_flex_grow(indicators, 1);
    lv_obj_set_width(indicators, lv_pct(100));
    lv_obj_set_flex_flow(indicators, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(indicators, LV_FLEX_ALIGN_SPACE_EVENLY, LV_PART_MAIN);

    lv_obj_t *temperature_column = lv_obj_create(indicators);
    lv_obj_remove_style_all(temperature_column);
    lv_obj_set_size(temperature_column, lv_pct(33), lv_pct(100));
    lv_obj_set_flex_flow(temperature_column, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_flex_cross_place(temperature_column, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN);

    lv_obj_t *temperature_label = lv_label_create(temperature_column);
    lv_label_set_text(temperature_label, "Temp");

    ui->temperature = lv_bar_create(temperature_column);
    lv_obj_add_style(ui->temperature, &temperature_style, LV_PART_INDICATOR);
    lv_obj_set_flex_grow(ui->temperature, 1);
    lv_obj_set_width(ui->temperature, lv_pct(50));
    lv_bar_set_range(ui->temperature, 0, 40);

    ui->temperature_value = lv_label_create(temperature_column);
    lv_obj_t *deg_c = lv_label_create(temperature_column);
    lv_label_set_text(deg_c, "°C");

    lv_obj_t *co2_column = lv_obj_create(indicators);
    lv_obj_remove_style_all(co2_column);
    lv_obj_set_size(co2_column, lv_pct(33), lv_pct(100));
    lv_obj_set_flex_flow(co2_column, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_flex_cross_place(co2_column, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN);

    lv_obj_t *co2_label = lv_label_create(co2_column);
    lv_label_set_text(co2_label, "CO2");

    ui->co2 = lv_bar_create(co2_column);
    lv_obj_add_style(ui->co2, &other_bar_style, LV_PART_INDICATOR);
    lv_obj_set_flex_grow(ui->co2, 1);
    lv_obj_set_width(ui->co2, lv_pct(50));
    lv_bar_set_range(ui->co2, 400, 1000);

    ui->co2_value = lv_label_create(co2_column);
    lv_obj_t *ppm = lv_label_create(co2_column);
    lv_label_set_text(ppm, "ppm");

    lv_obj_t *voc_column = lv_obj_create(indicators);
    lv_obj_remove_style_all(voc_column);
    lv_obj_set_size(voc_column, lv_pct(33), lv_pct(100));
    lv_obj_set_flex_flow(voc_column, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_flex_cross_place(voc_column, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN);

    lv_obj_t *voc_label = lv_label_create(voc_column);
    lv_label_set_text(voc_label, "VOC");

    ui->voc = lv_bar_create(voc_column);
    lv_obj_add_style(ui->voc, &other_bar_style, LV_PART_INDICATOR);
    lv_obj_set_flex_grow(ui->voc, 1);
    lv_obj_set_width(ui->voc, lv_pct(50));
    lv_bar_set_range(ui->voc, 0, 100);

    ui->voc_value = lv_label_create(voc_column);
    lv_obj_t *ppb = lv_label_create(voc_column);
    lv_label_set_text(ppb, "ppb");

    const uint8_t points = 3;

    lv_obj_t *chart1_wrapper = lv_obj_create(col2);
    lv_obj_remove_style_all(chart1_wrapper);
    lv_obj_set_size(chart1_wrapper, lv_pct(100), lv_pct(50));
    lv_obj_set_flex_flow(chart1_wrapper, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *today = lv_label_create(chart1_wrapper);
    lv_label_set_text(today, "Today");

    lv_obj_t *today_chart = lv_chart_create(chart1_wrapper);
    lv_obj_set_flex_grow(today_chart, 1);
    lv_obj_set_width(today_chart, lv_pct(100));
    lv_chart_set_point_count(today_chart, points);
    lv_chart_set_div_line_count(today_chart, 0, 0);
    lv_obj_add_event_cb(today_chart, chart_event_cb, LV_EVENT_ALL, "°C");

    lv_obj_t *scale_bottom = lv_scale_create(chart1_wrapper);
    lv_scale_set_mode(scale_bottom, LV_SCALE_MODE_HORIZONTAL_BOTTOM);
    lv_obj_set_size(scale_bottom, lv_pct(100), 25);
    lv_scale_set_total_tick_count(scale_bottom, points);
    lv_scale_set_major_tick_every(scale_bottom, 1);
    lv_obj_set_style_pad_hor(scale_bottom, lv_chart_get_first_point_center_offset(today_chart), 0);

    static const char *month[] = {"Jan", "Febr", "March", "Apr", "May", "Jun", "July", "Aug", "Sept", "Oct", "Nov", "Dec", NULL};
    lv_scale_set_text_src(scale_bottom, month);

    lv_chart_series_t *ser1 = lv_chart_add_series(today_chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
    for (uint32_t i = 0; i < points; i++)
    {
        lv_chart_set_next_value(today_chart, ser1, lv_rand(60, 90));
    }

    lv_obj_t *forecast = lv_label_create(col2);
    lv_label_set_text(forecast, "Forecast");

    lv_obj_t *forecast_chart = lv_chart_create(col2);
    lv_obj_set_size(forecast_chart, lv_pct(100), lv_pct(40));
    lv_chart_set_point_count(forecast_chart, points);
    lv_chart_set_div_line_count(forecast_chart, 0, 0);
    lv_obj_add_event_cb(forecast_chart, chart_event_cb, LV_EVENT_ALL, "°C");
    lv_chart_series_t *ser3 = lv_chart_add_series(forecast_chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_series_t *ser4 = lv_chart_add_series(forecast_chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_SECONDARY_Y);
    for (uint32_t i = 0; i < points; i++)
    {
        lv_chart_set_next_value(forecast_chart, ser3, lv_rand(60, 90));
        lv_chart_set_next_value(forecast_chart, ser4, lv_rand(10, 40));
    }
}