#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "drivers/ili9341.h"
#include <lvgl.h>

struct ili9341_display d;

void flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    ili9341_draw_window(&d, area->x1, area->y1, area->x2, area->y2);
    const uint32_t len = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1);
    lv_draw_sw_rgb565_swap(px_map, len);
    ili9341_draw(&d, px_map, len * 2);
    lv_disp_flush_ready(disp);
}

static void set_temp(void *bar, int32_t temp)
{
    lv_bar_set_value(bar, temp, LV_ANIM_ON);
}

int main()
{
    stdio_init_all();
    ili9341_init(&d, spi0, 18, 19, 20, 21, 22);
    ili9341_rotate(&d, 90, true);
    // debug();
    lv_init();
    lv_display_t *display = lv_display_create(SCREEN_HEIGHT, SCREEN_WIDTH);
    static lv_color_t buf1[(SCREEN_WIDTH * SCREEN_HEIGHT) / 2];
    lv_display_set_buffers(display, buf1, NULL, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, flush_cb);

    static lv_style_t style_indic;
    lv_style_init(&style_indic);
    lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);
    lv_style_set_bg_color(&style_indic, lv_palette_main(LV_PALETTE_RED));
    lv_style_set_bg_grad_color(&style_indic, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_bg_grad_dir(&style_indic, LV_GRAD_DIR_VER);

    lv_obj_t *bar = lv_bar_create(lv_screen_active());
    lv_obj_add_style(bar, &style_indic, LV_PART_INDICATOR);
    lv_obj_set_size(bar, 20, 200);
    lv_obj_center(bar);
    lv_bar_set_range(bar, -20, 40);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_exec_cb(&a, set_temp);
    lv_anim_set_duration(&a, 3000);
    lv_anim_set_playback_duration(&a, 3000);
    lv_anim_set_var(&a, bar);
    lv_anim_set_values(&a, -20, 40);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);
    while (true)
    {
        lv_timer_handler();
        lv_tick_inc(5);
        sleep_ms(5);
    }
    return 0;
}
