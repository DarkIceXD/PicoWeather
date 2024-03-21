#ifndef _UI_H_
#define _UI_H_
#include <lvgl.h>

struct ui
{
    lv_obj_t *time;
    lv_obj_t *date;
    lv_obj_t *temperature;
    lv_obj_t *temperature_value;
    lv_obj_t *humidity;
    lv_obj_t *humidity_value;
    lv_obj_t *pressure;
    lv_obj_t *pressure_value;
    lv_obj_t *co2;
    lv_obj_t *co2_value;
    lv_obj_t *voc;
    lv_obj_t *voc_value;
    lv_obj_t *chart[3];
    lv_chart_series_t *chart_series[3];
};

void ui_init(struct ui *ui, const uint32_t horizontal_resolution, const uint32_t vertical_resolution, const lv_display_flush_cb_t flush_cb, const lv_indev_read_cb_t read_cb);

#endif