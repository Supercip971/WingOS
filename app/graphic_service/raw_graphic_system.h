#ifndef RAW_GRAPHIC_SYSTEM_H
#define RAW_GRAPHIC_SYSTEM_H
#include "raw_window.h"
#include <stdint.h>
void init_raw_graphic_system();
uint64_t get_scr_width();
uint64_t get_scr_height();

void draw_mouse(uint64_t x, uint64_t y);
void graphic_system_update();
void draw_window(raw_window_data window);
#endif // RAW_GRAPHIC_SYSTEM_H
