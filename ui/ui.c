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
                snprintf(buf, sizeof(buf), "%ld", value);

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

static void create_chart(lv_obj_t *parent, const uint32_t points, const uint32_t series_count, const char **series_labels, lv_obj_t **chart, lv_chart_series_t **series)
{
    lv_obj_t *wrapper = lv_obj_create(parent);
    lv_obj_remove_style_all(wrapper);
    lv_obj_set_flex_grow(wrapper, 1);
    lv_obj_set_width(wrapper, lv_pct(200));
    lv_obj_set_flex_flow(wrapper, LV_FLEX_FLOW_COLUMN);

    *chart = lv_chart_create(wrapper);
    lv_obj_set_flex_grow(*chart, 1);
    lv_obj_set_width(*chart, lv_pct(100));
    lv_chart_set_point_count(*chart, points);
    lv_chart_set_div_line_count(*chart, 0, 0);
    lv_obj_add_event_cb(*chart, chart_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_t *scale = lv_scale_create(wrapper);
    lv_scale_set_mode(scale, LV_SCALE_MODE_HORIZONTAL_BOTTOM);
    lv_obj_set_size(scale, lv_pct(100), 25);
    lv_scale_set_total_tick_count(scale, points);
    lv_scale_set_major_tick_every(scale, 1);
    lv_scale_set_range(scale, 0, points);
    lv_obj_set_style_pad_hor(scale, lv_chart_get_first_point_center_offset(*chart), 0);

    lv_obj_t *legend = lv_obj_create(wrapper);
    lv_obj_remove_style_all(legend);
    lv_obj_set_size(legend, lv_pct(100), 15);
    lv_obj_set_flex_flow(legend, LV_FLEX_FLOW_ROW);

    static lv_palette_t colors[] = {LV_PALETTE_RED, LV_PALETTE_GREEN};
    for (uint8_t i = 0; i < series_count; i++)
    {
        lv_obj_t *series_container = lv_obj_create(legend);
        lv_obj_remove_style_all(series_container);
        lv_obj_set_flex_grow(series_container, 1);
        lv_obj_set_height(series_container, lv_pct(100));
        lv_obj_set_flex_flow(series_container, LV_FLEX_FLOW_ROW);

        lv_obj_t *color_square = lv_obj_create(series_container);
        lv_obj_set_size(color_square, 10, 10);
        lv_obj_set_style_radius(color_square, 0, LV_PART_MAIN);
        lv_obj_set_style_bg_color(color_square, lv_palette_main(colors[i]), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(color_square, LV_OPA_100, LV_PART_MAIN);

        lv_obj_t *series_label = lv_label_create(series_container);
        lv_label_set_text_static(series_label, series_labels[i]);

        series[i] = lv_chart_add_series(*chart, lv_palette_main(colors[i]), LV_CHART_AXIS_PRIMARY_Y);
    }
}

static void create_weather_charts(lv_obj_t *parent, struct chart *chart_data)
{
    static const char *temp_legend[] = {"Temperature in °C", "Feels like in °C"};
    static const char *wind_legend[] = {"Wind in km/h"};
    static const char *pressure_legend[] = {"Pressure in hPa"};
    static const char *humidity_legend[] = {"Relative humidity in %"};
    static const char *rain_legend[] = {"Chance of rain in %", "Chance of snow in %"};

    lv_obj_t *wrapper = lv_obj_create(parent);
    lv_obj_set_style_pad_row(wrapper, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(wrapper, 0, LV_PART_MAIN);
    lv_obj_set_size(wrapper, lv_pct(100), lv_pct(6 * 50));
    lv_obj_set_flex_flow(wrapper, LV_FLEX_FLOW_COLUMN);

    chart_data->label = lv_label_create(wrapper);
    lv_obj_set_style_text_font(chart_data->label, &lv_font_montserrat_24, LV_PART_MAIN);
    create_chart(wrapper, 24, 2, temp_legend, &chart_data->temp, chart_data->temp_series);
    create_chart(wrapper, 24, 1, wind_legend, &chart_data->wind, chart_data->wind_series);
    create_chart(wrapper, 24, 1, pressure_legend, &chart_data->pressure, chart_data->pressure_series);
    create_chart(wrapper, 24, 1, humidity_legend, &chart_data->humidity, chart_data->humidity_series);
    create_chart(wrapper, 24, 2, rain_legend, &chart_data->rain, chart_data->rain_series);
}

static lv_obj_t *keyboard;

static void textarea_event(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *ta = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED)
    {
        if (keyboard)
            lv_keyboard_set_textarea(keyboard, ta);
    }
}

static void create_textarea(lv_obj_t *parent, const char *label, lv_obj_t **area)
{
    lv_obj_t *label_obj = lv_label_create(parent);
    lv_label_set_text_static(label_obj, label);

    *area = lv_textarea_create(parent);
    lv_textarea_set_one_line(*area, true);
    lv_obj_set_width(*area, lv_pct(60));
    lv_obj_add_event_cb(*area, textarea_event, LV_EVENT_ALL, NULL);
}

void ui_init(struct ui *ui, const uint32_t horizontal_resolution, const uint32_t vertical_resolution, const lv_display_flush_cb_t flush_cb, const lv_indev_read_cb_t read_cb, const lv_event_cb_t save_cb)
{
    lv_init();
    lv_display_t *display = lv_display_create(horizontal_resolution, vertical_resolution);
    lv_display_set_buffers(display, buf1, NULL, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, flush_cb);

    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, read_cb);

    lv_obj_t *menu = lv_menu_create(lv_screen_active());
    lv_obj_set_size(menu, lv_pct(100), lv_pct(100));

    lv_obj_t *back_btn = lv_menu_get_main_header_back_button(menu);
    lv_obj_t *back_button_label = lv_label_create(back_btn);
    lv_label_set_text_static(back_button_label, "Back");

    lv_obj_t *sub_page = lv_menu_page_create(menu, "Settings");

    lv_obj_t *sub = lv_obj_create(sub_page);
    lv_obj_set_style_pad_row(sub, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(sub, 0, LV_PART_MAIN);
    lv_obj_set_size(sub, lv_pct(100), lv_pct(50));
    lv_obj_set_flex_flow(sub, LV_FLEX_FLOW_COLUMN);

    create_textarea(sub, "SSID:", &ui->ssid);
    create_textarea(sub, "Password:", &ui->password);

    lv_obj_t *connect = lv_button_create(sub);
    lv_obj_t *connect_label = lv_label_create(connect);
    lv_label_set_text_static(connect_label, LV_SYMBOL_WIFI " Connect");

    create_textarea(sub, "WeatherAPI Key:", &ui->weatherapi_key);
    create_textarea(sub, "Query (e.g. City name):", &ui->query);

    lv_obj_t *save = lv_button_create(sub);
    lv_obj_add_event_cb(save, save_cb, LV_EVENT_ALL, NULL);
    lv_obj_t *save_label = lv_label_create(save);
    lv_label_set_text_static(save_label, LV_SYMBOL_SAVE " Save");

    keyboard = lv_keyboard_create(sub_page);
    lv_obj_set_size(keyboard, lv_pct(100), lv_pct(50));

    lv_obj_t *main_page = lv_menu_page_create(menu, NULL);

    lv_obj_t *main = lv_obj_create(main_page);
    lv_obj_set_style_pad_row(main, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(main, 0, LV_PART_MAIN);
    lv_obj_set_size(main, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(main, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *settings_button = lv_button_create(main_page);
    lv_obj_t *label = lv_label_create(settings_button);
    lv_label_set_text_static(label, LV_SYMBOL_SETTINGS);
    lv_menu_set_load_page_event(menu, settings_button, sub_page);

    ui->date = lv_label_create(main);
    ui->time = lv_label_create(main);
    lv_obj_set_style_text_font(ui->time, &lv_font_montserrat_48, LV_PART_MAIN);

    ui->location = lv_label_create(main);
    lv_label_set_text(ui->location, LV_SYMBOL_HOME " Berlin");

    lv_obj_t *indicators = lv_obj_create(main);
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

    create_weather_charts(main_page, &ui->days[0]);
    lv_label_set_text(ui->days[0].label, "Today");

    create_weather_charts(main_page, &ui->days[1]);
    lv_label_set_text(ui->days[1].label, "Tomorrow");

    create_weather_charts(main_page, &ui->days[2]);
    lv_label_set_text(ui->days[2].label, "Overmorrow");

    lv_menu_set_page(menu, main_page);
}