#pragma once

struct Allocator;

enum struct PixelFormat
{
    R8G8B8A8_UINT,
    R8G8B8A8_UINT_NORM,
    R32_FLOAT,
    R32G32B32A32_FLOAT,
    R32_UINT,
    R8_UINT_NORM
};

unsigned pixel_size(PixelFormat pf);

struct Image
{
    unsigned width;
    unsigned height;
    PixelFormat pixel_format;
    unsigned char* data;
};

void image_init_data(Image* i, Allocator* alloc);
unsigned image_size(PixelFormat pf, unsigned size_x, unsigned size_y);
unsigned image_size(const Image& image);
