#pragma once
#include "math.h"
#include "render_resource.h"

struct RenderObject
{
    unsigned long long id;
    Matrix4x4 world_transform;
    Vector3 position;
    Quaternion rotation;
    RRHandle geometry_handle;
    bool is_light;
};

struct RenderObjectResource
{
    RenderObject ro;
    bool used;
};

struct RenderObjectHandle
{
    size_t h;
};

RenderObjectHandle render_object_create();
void render_object_destroy(RenderObjectHandle h);
RenderObject* render_object_get(RenderObjectHandle h);
RenderObjectResource* render_object_get_lut();
void render_object_deinit_lut();