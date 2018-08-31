#include "debug.h"
#include "renderer.h"
#include "memory.h"

static Renderer* _renderer = nullptr;
static unsigned _num_random_colors = 128;
static Color* _random_colors = nullptr;
static ForcePumpWindowFunc _force_pump_window_func = nullptr;

void debug_init(Renderer* r, ForcePumpWindowFunc force_pump_window_func)
{
    _force_pump_window_func = force_pump_window_func;
    _renderer = r;
    _random_colors = (Color*)zalloc(_num_random_colors*sizeof(Color));

    for (unsigned i = 0; i < _num_random_colors; ++i)
        _random_colors[i] = color_random();
}

void debug_clear_frame()
{
    _renderer->pre_frame();
}

void debug_present()
{
    _renderer->present();
}

void debug_force_pump_window()
{
    _force_pump_window_func();
}

void debug_draw_mesh(const Vec3* vertices, unsigned num_vertices, const Color& color)
{
    _renderer->draw_debug_mesh(vertices, num_vertices, color);
}

const Color& debug_get_random_color(unsigned idx)
{
    Assert(idx < _num_random_colors, "Out of debug random colors, try increasing _num_random_colors");
    return _random_colors[idx];
}

void debug_shutdown()
{
    zfree(_random_colors);
}
