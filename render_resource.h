#pragma once
#include "image.h"
#include "color.h"

struct RRHandle
{
    unsigned h;
};

struct RenderTarget
{
    PixelFormat pixel_format;
    unsigned width;
    unsigned height;
    bool clear_depth_stencil : 1;
    bool clear : 1;
    Color clear_color;
    RRHandle render_resource;
};

struct MappedTexture
{
    void* data;
    RRHandle texture;
};