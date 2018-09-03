#pragma once

struct RenderObjectHandle;
struct RenderObject;
typedef size_t EntityHandle;

struct RenderWorld
{
    RenderObjectHandle* D_objects;
};

void render_world_add(RenderWorld* w, RenderObjectHandle h);
void render_world_add(RenderWorld* w, EntityHandle h);
void render_world_remove(RenderWorld* w, RenderObjectHandle h);
void render_world_remove(RenderWorld* w, EntityHandle h);
void render_world_destroy(RenderWorld* w);
void render_world_get_objects_to_render(const RenderWorld* w, RenderObject* const* ros);