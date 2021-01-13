#ifndef GENERAL_DEVICE_H
#define GENERAL_DEVICE_H
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
    DEBUG_DEVICE = 6,
    NULL_DEVICE = 0xfffff,
};
extern const char *device_type_to_str[];

class general_device;

void add_device(general_device *dev);

general_device *get_device(uint32_t id);
uint32_t get_device_count();

template <class end_type>
extern auto find_device(device_type type) -> end_type *;

class general_device
{
public:
    virtual const char *get_name() const;
    virtual device_type get_type() const;
    uint32_t device_id;
};

class general_mouse : public general_device
{
public:
    device_type get_type() const final { return device_type::MOUSE_DEVICE; };
    virtual int32_t get_mouse_x() = 0;
    virtual int32_t get_mouse_y() = 0;
    virtual uint64_t get_mouse_button(int code) = 0;
    virtual void set_ptr_to_update(uint32_t *x, uint32_t *y);
};

class general_keyboard : public general_device
{
public:
    device_type get_type() const final { return device_type::KEYBOARD_DEVICE; };

    virtual bool get_key(uint8_t keycode) const = 0;
    virtual uint8_t get_last_keypress() const = 0;
    virtual void set_ptr_to_update(uint32_t *d) = 0;
};

class generic_io_device : public general_device
{
public:
    device_type get_type() const final { return device_type::DRIVE_DEVICE; };

    enum io_rw_output
    {
        io_ERROR = 0,
        io_OK = 1,
    };
    virtual io_rw_output read(uint8_t *data, uint64_t count, uint64_t cursor) = 0;
    virtual io_rw_output write(uint8_t *data, uint64_t count, uint64_t cursor) = 0;
};

class debug_device : public general_device
{

public:
    device_type get_type() const final { return device_type::DEBUG_DEVICE; };
    virtual bool echo_out(const char *data, uint64_t data_length) = 0;
    virtual bool echo_out(const char *data) = 0;
};

#endif // TIMER_DEVICE_H
