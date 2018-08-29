#include "render_object.h"
#include "array.h"

static RenderObjectResource* D_ror_lut = nullptr;
static unsigned long long object_id_counter = 1;

RenderObjectHandle render_object_create()
{
    RenderObject obj = {};
    obj.id = object_id_counter++;
    obj.world_transform = mat4_identity();

    for (size_t i = 0; i < array_num(D_ror_lut); ++i)
    {
        if (D_ror_lut[i].used == false)
        {
            D_ror_lut[i].ro = obj;
            D_ror_lut[i].used = true;
            return {i};
        }
    }

    size_t idx = array_num(D_ror_lut);
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