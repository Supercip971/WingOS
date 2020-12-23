#ifndef CURSOR_H
#define CURSOR_H

#include <stdint.h>
bool is_mouse_in_window(uint64_t wid);
void init_cursor();
void update_mouse();
void set_mouse_on_window(uint64_t wid);
#endif // CURSOR_H
