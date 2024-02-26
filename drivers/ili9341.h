#ifndef _ILI9341_H_
#define _ILI9341_H_
#include "hardware/spi.h"

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
#define SCREEN_TOTAL_PIXELS (SCREEN_WIDTH * SCREEN_HEIGHT)
#define BUFFER_SIZE (SCREEN_TOTAL_PIXELS * 2)

struct ili9341_display
{
    spi_inst_t *spi_port;
    uint sck, tx, reset, dc, cs;
};

void ili9341_init(struct ili9341_display *display, spi_inst_t *spi_port, const uint sck, const uint tx, const uint reset, const uint dc, const uint cs);
void ili9341_rotate(const struct ili9341_display *display, int degrees, bool use_bgr);
void ili9341_draw_window(const struct ili9341_display *display, const uint16_t x1, const uint16_t y1, const uint16_t x2, const uint16_t y2);
void ili9341_draw(const struct ili9341_display *display, const uint8_t *buffer, const uint len);

#endif