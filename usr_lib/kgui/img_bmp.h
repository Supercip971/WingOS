#ifndef IMG_BMP_H
#define IMG_BMP_H
#include <stdint.h>
namespace gui
{
    enum bmp_offset
    {
        BMP_DATA_OFFSET = 0xA,
        BMP_WIDTH = 0x12,
        BMP_HEIGHT = 0x16,
        BMP_BPP = 0x1C,
    };
#define BMP_NO_COMPRESSION 0
#define BMP_INFO_HEADER_SIZE 40
#define BMP_HEADER_SIZE 14
    struct raw_bmp_header
    {
        char Signature_B;
        char Signature_M;

        uint32_t file_size;

        uint32_t reserved;

        uint32_t data_offset;

        uint32_t header_size;

        uint32_t width;
        uint32_t height;

        uint16_t planes;

        uint16_t bpp;

        uint32_t compression;
        uint32_t compressed_img_size;

        uint32_t pixel_per_metter_width;
        uint32_t pixel_per_metter_height;

        uint32_t used_color_count;
        uint32_t main_colors;
    } __attribute__((packed));

    class img_bmp
    {
        raw_bmp_header *header;
        uint8_t *data;
        uint8_t *pix_data;
        unsigned int width;
        unsigned int height;
        unsigned int bit_per_pixel;

    public:
        img_bmp(const char *path);
        uint8_t *get_pix_data()
        {
            return pix_data;
        }
        uint32_t get_width()
        {
            return header->width;
        }
        uint32_t get_height()
        {
            return header->height;
        }
    };
} // namespace gui

#endif // IMG_BMP_H
