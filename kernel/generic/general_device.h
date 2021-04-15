#ifndef GENERAL_DEVICE_H
#define GENERAL_DEVICE_H
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <utils/device_file_info.h>
#include <utils/wvector.h>
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
    FRAMEBUFFER_DEVICE = 7,
    NULL_DEVICE = 0xfffff,
};
extern const char *device_type_to_str[];

class general_device;

void add_device(general_device *dev);

general_device *get_device(uint32_t id);
uint32_t get_device_count();

template <class end_type>
extern auto find_device() -> end_type *;

class general_device
{
public:
    virtual const char *get_name() const { return "null device"; };
    virtual device_type get_type() const { return device_type::NULL_DEVICE; };
    virtual ~general_device() = default;
    uint32_t device_id;
};

class interrupt_timer : public general_device
{
protected:
    bool state;
    uint32_t current_clock;

public:
    static device_type get_stype() { return device_type::TIMER_DEVICE; };
    device_type get_type() const final { return get_stype(); };

    virtual void set_clock(uint32_t clock) = 0;
    virtual void turn_off() = 0;
    virtual void turn_on() = 0;
    bool get_state() const { return state; }
};

class general_mouse : public general_device
{
public:
    static device_type get_stype() { return device_type::MOUSE_DEVICE; };
    device_type get_type() const final { return get_stype(); };
    virtual int32_t get_mouse_x() = 0;
    virtual int32_t get_mouse_y() = 0;
    virtual uint64_t get_mouse_button(int code) = 0;
    virtual void set_ptr_to_update(uint32_t *x, uint32_t *y);
    size_t read_mouse_buffer(void *addr, size_t buffer_idx, size_t length);
};

class general_keyboard : public general_device
{
protected:
    utils::vector<keyboard_buff_info> buffer;

public:
    static device_type get_stype() { return device_type::KEYBOARD_DEVICE; };
    device_type get_type() const final { return get_stype(); };
    void update_key_out(char out);
    virtual bool get_key(uint8_t keycode) const = 0;
    virtual uint8_t get_last_keypress() const = 0;
    virtual void set_ptr_to_update(uint32_t *d) = 0;
    size_t read_key_buffer(void *addr, size_t buffer_idx, size_t length)
    {

        size_t rlength = length;
        if (buffer_idx > buffer.size() * sizeof(keyboard_buff_info))
        {
            return 0;
        }
        else if (buffer_idx + length > buffer.size() * sizeof(keyboard_buff_info))
        {
            rlength = sizeof(mouse_buff_info) - buffer_idx;
        }
        memcpy((uint8_t *)addr, ((uint8_t *)buffer.raw()) + buffer_idx, rlength);
        return rlength;
    }
    size_t get_key_buffer_size()
    {
        return buffer.size() * sizeof(keyboard_buff_info);
    }
};

class generic_io_device : public general_device
{
public:
    static device_type get_stype() { return device_type::DRIVE_DEVICE; };
    device_type get_type() const final { return get_stype(); };

    enum io_rw_output
    {
        io_ERROR = 0,
        io_OK = 1,
    };
    virtual io_rw_output read(uint8_t *data, uint64_t count, uint64_t cursor) = 0;
    io_rw_output read_unaligned(uint8_t *data, uint64_t count, uint64_t cursor);
    virtual io_rw_output write(uint8_t *data, uint64_t count, uint64_t cursor) = 0;
    io_rw_output write_unaligned(uint8_t *data, uint64_t count, uint64_t cursor);
};

class debug_device : public general_device
{

public:
    static device_type get_stype() { return device_type::DEBUG_DEVICE; };
    device_type get_type() const final { return get_stype(); };
    virtual bool echo_out(const char *data, uint64_t data_length) = 0;
    virtual bool echo_out(const char *data) = 0;
};

class generic_framebuffer : public general_device
{
public:
    static device_type get_stype() { return device_type::FRAMEBUFFER_DEVICE; };
    device_type get_type() const final { return get_stype(); };
    virtual uintptr_t get_addr() = 0;
    virtual size_t width() = 0;
    virtual size_t height() = 0;

    size_t read_buffer(void *addr, size_t length)
    {

        size_t rlength = length;
        if (length > get_buffer_size())
        {
            rlength = sizeof(framebuffer_buff_info);
        }
        framebuffer_buff_info info{0};
        info.width = width();
        info.height = height();
        info.addr = get_addr();
        memcpy((uint8_t *)addr, ((uint8_t *)&info), rlength);
        return rlength;
    }

    size_t get_buffer_size()
    {
        return sizeof(framebuffer_buff_info);
    }
};

#endif // TIMER_DEVICE_H
