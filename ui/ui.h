#ifndef _UI_H_
#define _UI_H_
#include <lvgl.h>

struct ui
{
    lv_obj_t *time;
    lv_obj_t *date;
    lv_obj_t *temperature;
    lv_obj_t *temperature_value;
    lv_obj_t *eco2;
    lv_obj_t *eco2_value;
    lv_obj_t *etvoc;
    lv_obj_t *etvoc_value;
};

void ui_init(struct ui *ui, const uint32_t horizontal_resolution, const uint32_t vertical_resolution, const lv_display_flush_cb_t flush_cb);

#endif