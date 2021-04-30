#include "device_generic_driver.h"
generic_io_device::io_rw_output generic_io_device::read_unaligned(uint8_t *data, uint64_t count, uint64_t cursor)
{
    uint64_t max_block_count = (((cursor % 512) + (count)) / 512) + 1;
    uint8_t *raw = (uint8_t *)malloc(max_block_count * 512);
    io_rw_output r = io_rw_output::io_OK;
    for (size_t i = 0; i < max_block_count; i++)
    {

        r = read(raw + (i * 512), 1, i + (cursor / 512));
        if (r != io_rw_output::io_OK)
        {
            free(raw);
            return r;
        }
    }
    memcpy(data, raw + cursor % 512, (count));
    free(raw);
    return r;
}

generic_io_device::io_rw_output generic_io_device::write_unaligned(uint8_t *data, uint64_t count, uint64_t cursor)
{
    uint64_t max_block_count = (((cursor % 512) + (count)) / 512) + 1;
    uint64_t cur_block = ((cursor / 512)) * 512;
    uint8_t *raw = (uint8_t *)malloc(max_block_count * 512);
    read_unaligned(raw, max_block_count * 512, cur_block);

    memcpy(raw + cursor % 512, data, (count));
    io_rw_output r = io_rw_output::io_OK;
    for (size_t i = 0; i < max_block_count; i++)
    {

        r = write(raw + (i * 512), 1, i + (cur_block / 512));
        if (r != io_rw_output::io_OK)
        {
            free(raw);
            return r;
        }
    }
    free(raw);
    return r;
}

size_t general_mouse::
    read_mouse_buffer(void *addr, size_t buffer_idx, size_t length)
{
    size_t rlength = length;

    if (buffer_idx > sizeof(mouse_buff_info))
    {

        return 0;
    }
    else if (buffer_idx + length > sizeof(mouse_buff_info))
    {
        rlength = sizeof(mouse_buff_info) - buffer_idx;
    }
    mouse_buff_info info;

    info.mouse_x = get_mouse_x();
    info.mouse_y = get_mouse_y();

    info.left = get_mouse_button(0);
    info.right = get_mouse_button(1);
    info.middle = get_mouse_button(2);

    memcpy((uint8_t *)addr, (uint8_t *)&info + buffer_idx, rlength);
    return rlength;
}
