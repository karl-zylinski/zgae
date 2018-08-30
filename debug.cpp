#include "debug.h"
#include "renderer.h"

static Renderer* G_renderer = nullptr;

void debug_init(Renderer* r)
{
    G_renderer = r;
}

void debug_draw_mesh(const Vec3* vertices, unsigned num_vertices, const Color& color)
{
    G_renderer->draw_debug_mesh(vertices, num_vertices, color);
}
