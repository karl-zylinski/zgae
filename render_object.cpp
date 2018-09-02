#include "render_object.h"
#include "array.h"

static RenderObjectResource* D_ror_lut = nullptr;

RenderObjectHandle render_object_create(RRHandle geometry, const Vec3& position, const Quat& rotation)
{
    RenderObject obj = {};
    obj.geometry = geometry;
    obj.world_transform = mat4_from_rotation_and_translation(rotation, position);

    for (size_t i = 0; i < array_size(D_ror_lut); ++i)
    {
        if (D_ror_lut[i].used == false)
        {
            D_ror_lut[i].ro = obj;
            D_ror_lut[i].used = true;
            return {i};
        }
    }

    size_t idx = array_size(D_ror_lut);
    RenderObjectResource ror = {};
    ror.used = true;
    ror.ro = obj;
    array_push(D_ror_lut, ror);
    return {idx};
}

void render_object_destroy(RenderObjectHandle h)
{
    D_ror_lut[h.h].used = false;
}

void render_object_set_position_and_rotation(RenderObjectHandle h, const Vec3& pos, const Quat& rot)
{
    D_ror_lut[h.h].ro.world_transform = mat4_from_rotation_and_translation(rot, pos);
}

RenderObject* render_object_get(RenderObjectHandle h)
{
    Assert(D_ror_lut[h.h].used, "Trying to get unused RenderObject");
    return &D_ror_lut[h.h].ro;
}

RenderObjectResource* render_object_get_lut()
{
    return D_ror_lut;
}

void render_object_deinit_lut()
{
    array_destroy(D_ror_lut);
}