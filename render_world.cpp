#pragma once
#include "render_world.h"
#include "render_object.h"
#include "array.h"

void render_world_add(RenderWorld* w, RenderObjectHandle h)
{
    array_push(w->D_objects, h);
} 

void render_world_remove(RenderWorld* w, RenderObjectHandle h)
{
    for (size_t i = 0; i < array_size(w->D_objects); ++i)
    {
        if (w->D_objects[i].h == h.h)
        {
            array_remove(w->D_objects, i);
            break;
        }
    }
}

void render_world_destroy(RenderWorld* w)
{
    array_destroy(w->D_objects);
}

void render_world_get_objects_to_render(const RenderWorld* w, RenderObject* const * ros)
{
    RenderObjectResource* ror_lut = render_object_get_lut();

    for (size_t i = 0; i < array_size(w->D_objects); ++i)
    {
        RenderObjectResource ror = ror_lut[w->D_objects[i].h];
        if (ror.used && ror.ro.geometry.h != 0)
            array_push(*ros, ror.ro);
    }
}