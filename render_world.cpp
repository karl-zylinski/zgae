#pragma once
#include "render_world.h"
#include "render_object.h"
#include "memory.h"
#include "array.h"

static unsigned long long object_id_counter = 1;

size_t render_world_add(RenderWorld* w, RenderObject* obj)
{
    if (obj->id == 0)
        obj->id = object_id_counter++;

    for (size_t i = 0; i < array_num(w->ror_lut); ++i)
    {
        if (w->ror_lut[i].used == false)
        {
            memcpy(w->ror_lut + i, obj, sizeof(RenderObject));
            w->ror_lut[i].used = true;
            array_push(w->active_objects, i);
            return i;
        }
    }
    size_t idx = array_num(w->ror_lut);
    RenderObjectResource ror = {};
    ror.used = true;
    ror.ro = *obj;
    array_push(w->ror_lut, ror);
    array_push(w->active_objects, idx);
    return idx;
} 

void render_world_remove(RenderWorld* w, size_t idx)
{
    if (idx < 0 || idx >= array_num(w->ror_lut))
        Error("Trying to remove non-existing render object!");
    RenderObjectResource* ror = w->ror_lut + idx;
    Assert(ror->used, "Trying to remove unused object!");
    for (size_t i = 0; i < array_num(w->active_objects); ++i)
    {
        if (w->active_objects[i] == idx)
        {
            array_remove(w->active_objects, i);
            break;
        }
    }
    memzero(ror, sizeof(RenderObjectResource));
}

RenderObject* render_world_get(RenderWorld* w, size_t idx)
{
    RenderObjectResource* ror = w->ror_lut + idx;
    Assert(ror->used, "Trying to remove unused object!");
    return &ror->ro;
}

void render_world_destroy(RenderWorld* w)
{
    array_destroy(w->ror_lut);
    array_destroy(w->active_objects);
}