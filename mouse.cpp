#include "mouse.h"

struct MouseState
{
    Vec2i delta;
};

static MouseState ms = {};

void mouse_end_of_frame()
{
    ms.delta = {0, 0};
}

void mouse_moved(u32 dx, u32 dy)
{
    ms.delta.x += dx;
    ms.delta.y += dy;
}

Vec2i mouse_get_delta()
{
    return ms.delta;
}