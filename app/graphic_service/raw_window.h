#ifndef RAW_WINDOW_H
#define RAW_WINDOW_H
#include <gui/graphic_system.h>
#include <gui/raw_graphic.h>
#include <stdint.h>

#define MAX_WINDOW 255

struct raw_window_data
{
    uint64_t wid;
    uint64_t pid;
    uint64_t px;
    uint64_t py;
    uint64_t width;
    uint64_t height;
    char *window_name;
    gui::pixel *window_front_buffer;
    gui::pixel *window_back_buffer;
    bool used;
    bool background;

    bool should_redraw;
    uint64_t depth;
};
void init_raw_windows();
raw_window_data *get_window_list();
uint64_t get_window_list_count();

void set_window_on_top(uint64_t wid);
void set_window_background(uint64_t wid);
void set_window_top_background(uint64_t wid);

uint64_t create_window(gui::graphic_system_service_protocol *request, uint64_t pid);

bool valid_window(uint64_t target_wid, uint64_t pid);

uint64_t get_window_back_buffer(gui::graphic_system_service_protocol *request, uint64_t pid);
uint64_t window_swap_buffer(gui::graphic_system_service_protocol *request, uint64_t pid);
uint64_t get_window_position(gui::graphic_system_service_protocol *request, uint64_t pid);
uint64_t set_window_position(gui::graphic_system_service_protocol *request, uint64_t pid);
uint64_t window_depth_action(gui::graphic_system_service_protocol *request, uint64_t pid);

void update_mouse_in_window();

void draw_all_window();
#endif // RAW_WINDOW_H
