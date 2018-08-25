#pragma once
#include "object.h"
#include "dynamic_array.h"

struct World
{
    DynamicArray<Object> objects;
};

inline void world_init(World* w, Allocator* allocator)
{
    w->objects = dynamic_array_create<Object>(allocator);
}

inline void world_destroy(World* w)
{
    dynamic_array_destroy(&w->objects);
}