#pragma once
#include "math.h"
#include "render_resource.h"

struct RenderObject
{
    bool used;
    Matrix4x4 world_transform;
    RRHandle geometry_handle;
    bool is_light;
};


