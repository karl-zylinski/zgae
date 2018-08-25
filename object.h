#pragma once
#include "math.h"
#include "render_resource.h"

struct Object
{
    RRHandle geometry_handle;
    RRHandle lightmap_handle;
    RRHandle lightmap_patch_offset;
    unsigned id;
    Matrix4x4 world_transform;
    bool is_light;
};
