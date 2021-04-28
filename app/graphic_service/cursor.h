#ifndef CURSOR_H
#define CURSOR_H

#include "raw_window.h"
#include <stdint.h>
bool is_mouse_in_window(raw_window_data *window);
void init_cursor();
void update_mouse();
uint64_t get_mouse_on_window_wid();
void set_mouse_on_window(raw_window_data *window);
#endif // CURSOR_H
