#pragma once
#include "render_world.h"
#include "render_object.h"
#include "memory.h"

static void grow(RenderWorld* w)
{
    unsigned old_cap = w->capacity;
    w->capacity = old_cap*2;
    RenderObject* old_objs = w->objects;
    w->objects = (RenderObject*)zalloc_zero(sizeof(RenderObject) * w->capacity);
    memcpy(w->objects, old_objs, old_cap*sizeof(RenderObject));
    zfree(old_objs);
}

void render_world_init(RenderWorld* w)
{
    w->capacity = 100;
    w->objects = (RenderObject*)zalloc_zero(sizeof(RenderObject) * w->capacity);
}

static unsigned find_free_idx_or_grow(RenderWorld* w)
{
    for (unsigned i = 0; i < w->capacity; ++i)
    {
        if (w->objects[i].used == false)
            return i;
    }

    unsigned idx = w->capacity;
    grow(w);
    return idx;
}

unsigned render_world_add(RenderWorld* w, RenderObject* obj)
{
    unsigned idx = find_free_idx_or_grow(w);
    memcpy(w->objects + idx, obj, sizeof(RenderObject));
    (w->objects + idx)->used = true;
    return idx;
} 

void render_world_remove(RenderWorld* w, unsigned idx)
{
    if (idx < 0 || idx >= w->capacity)
        Error("Trying to remove non-existing render object!");

    RenderObject* obj = w->objects + idx;

    Assert(obj->used, "Trying to remove unused object!");
    memzero(obj, sizeof(RenderObject));
}

RenderObject* render_world_get(RenderWorld* w, unsigned idx)
{
    RenderObject* obj = w->objects + idx;
    Assert(obj->used, "Trying to remove unused object!");
    return w->objects + idx;
}

void render_world_destroy(RenderWorld* w)
{
    zfree(w->objects);
}