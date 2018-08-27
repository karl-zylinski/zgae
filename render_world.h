#pragma once

struct RenderObject;
struct RenderObjectResource;

struct RenderWorld
{
    RenderObjectResource* ror_lut; // Stretchy. For idx -> obj lookup
    size_t* active_objects; // Stretchy. For quick drawing, used by renderer
};

size_t render_world_add(RenderWorld* w, RenderObject* obj);
void render_world_remove(RenderWorld* w, size_t idx);
RenderObject* render_world_get(RenderWorld* w, size_t idx);
void render_world_destroy(RenderWorld* w);