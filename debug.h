#pragma once

struct Renderer;
struct Vec3;
struct Color;

void debug_init(Renderer* r);
void debug_draw_mesh(const Vec3* vertices, unsigned num_vertices, const Color& color);