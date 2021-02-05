#include "img_bmp.h"
#include <kern/file.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
namespace gui
{
    img_bmp::img_bmp(const char *path)
    {
        sys::file f = sys::file(path);
        header = new raw_bmp_header();
        f.seek(0);
        f.read((uint8_t *)header, sizeof(raw_bmp_header));
        if (header->Signature_B != 'B' || header->Signature_M != 'M')
        {
            printf("error invalid bmp signature (%c & %c) for file %s with size %x \n", header->Signature_B, header->Signature_M, path, f.get_file_length());
            return;
        }
        printf("for %s \n with size %x \n width = %x \n height = %x \n", path, f.get_file_length(), header->width, header->height);

        int paddedRowSize = (int)(4 * ((header->width / 4) + 1)) * (header->bpp);
        if (header->width % 4 == 0)
        {
            paddedRowSize = (int)(((header->width))) * (header->bpp);
        }
        int unpaddedRowSize = (header->width) * (header->bpp);
        uint64_t total_size = unpaddedRowSize * header->height;
        pix_data = new uint8_t[total_size + 2];
        f.seek(header->data_offset);

        f.read(pix_data, unpaddedRowSize * header->height);
        f.close();
    }
} // namespace gui
