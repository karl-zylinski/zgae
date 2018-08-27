#pragma once

struct RenderObject;
struct RenderWorld
{
    RenderObject* objects;
    unsigned capacity;
};

void render_world_init(RenderWorld* w);
unsigned render_world_add(RenderWorld* w, RenderObject* obj);
void render_world_remove(RenderWorld* w, unsigned idx);
RenderObject* render_world_get(RenderWorld* w, unsigned idx);
void render_world_destroy(RenderWorld* w);