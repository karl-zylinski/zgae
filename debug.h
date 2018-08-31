#pragma once

struct Renderer;
struct Vec3;
struct Color;

typedef void(*ForcePumpWindowFunc)();

void debug_init(Renderer* r, ForcePumpWindowFunc force_pump_window_func);
void debug_clear_frame();
void debug_present();
void debug_force_pump_window();
void debug_draw_mesh(const Vec3* vertices, unsigned num_vertices, const Color& color);
const Color& debug_get_random_color(unsigned idx);
void debug_shutdown();