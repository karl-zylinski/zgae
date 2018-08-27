#include "image.h"
#include "memory.h"

unsigned pixel_size(PixelFormat pf)
{
    switch (pf)
    {
        case PixelFormat::R8G8B8A8_UINT:
            return 4;
        case PixelFormat::R8G8B8A8_UINT_NORM:
            return 4;
        case PixelFormat::R32_FLOAT:
            return 4;
        case PixelFormat::R32G32B32A32_FLOAT:
            return 16;
        case PixelFormat::R32_UINT:
            return 4;
        case PixelFormat::R8_UINT_NORM:
            return 1;
        default:
            Error("Unknown pixel format."); return 0;
    }
}

void image_init(Image* i, unsigned width, unsigned height, PixelFormat pf)
{
    i->data = (unsigned char*)zalloc(image_size(pf, width, height));
}

void image_destroy(Image* i)
{
    zfree(i->data);
}

unsigned image_size(PixelFormat pf, unsigned size_x, unsigned size_y)
{
    return size_x * size_y * pixel_size(pf);
}

unsigned image_size(const Image& image)
{
    return image_size(image.pixel_format, image.width, image.height);
}
