#include "debug.h"
#include "renderer.h"

static Renderer* _renderer = nullptr;

void debug_init(Renderer* r)
{
    _renderer = r;
}

void debug_draw_mesh(const Vec3* vertices, unsigned num_vertices, const Color& color)
{
    _renderer->draw_debug_mesh(vertices, num_vertices, color);
}
