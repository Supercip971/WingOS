#ifndef TIMER_DEVICE_H
#define TIMER_DEVICE_H
#include <logging.h>
#include <stdint.h>
#define MAX_DEVICE 128
enum device_type : uint32_t
{
    TIMER_DEVICE = 0,
    DRIVE_DEVICE = 1,
    NETWORK_DEVICE = 2,
    MOUSE_DEVICE = 3,
    KEYBOARD_DEVICE = 4,
    CLOCK_DEVICE = 5,
    NULL_DEVICE = 0xfffff,
};
extern const char *device_type_to_str[];
class general_device
{
public:
    virtual const char *get_name() const;
    virtual device_type get_type() const;
    uint32_t device_id;
};

void add_device(general_device *dev);

general_device *get_device(uint32_t id);
uint32_t get_device_count();

template <class end_type>
extern end_type *find_device(device_type type);

class general_mouse : public general_device
{
public:
    device_type get_type() const final { return device_type::MOUSE_DEVICE; };
    virtual int32_t get_mouse_x() = 0;
    virtual int32_t get_mouse_y() = 0;
    virtual uint64_t get_mouse_button(int code) = 0;
    virtual void set_ptr_to_update(uint32_t *x, uint32_t *y);
};
#endif // TIMER_DEVICE_H
