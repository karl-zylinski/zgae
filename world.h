#pragma once
#include "object.h"
#include "dynamic_array.h"

struct World
{
    DynamicArray<Object> objects;
};

inline World world_create(Allocator* allocator)
{
    World w = {};
    w.objects = dynamic_array_create<Object>(allocator);
    return w;
}

inline void world_destroy(World* w)
{
    dynamic_array_destroy(&w->objects);
}