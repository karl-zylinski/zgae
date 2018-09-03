#pragma once
#include "math.h"
#include "render_resource.h"

struct RenderObject
{
    Mat4 world_transform;
    RRHandle geometry;
};

struct RenderObjectResource
{
    RenderObject ro;
    bool used;
};

struct RenderObjectHandle { size_t h; };

RenderObjectHandle render_object_create(RRHandle geomerty, const Vec3& position, const Quat& rotation);
void render_object_destroy(RenderObjectHandle h);
void render_object_set_position_and_rotation(RenderObjectHandle h, const Vec3& pos, const Quat& rot);
RenderObject* render_object_get(RenderObjectHandle h);
RenderObjectResource* render_object_get_lut();
void render_object_deinit_lut();