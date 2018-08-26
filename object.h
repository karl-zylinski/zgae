#pragma once
#include "math.h"
#include "render_resource.h"

struct Object
{
    RRHandle geometry_handle;
    Matrix4x4 world_transform;
    bool is_light;
};
