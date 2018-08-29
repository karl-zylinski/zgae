#pragma once

struct Vec2i;

void mouse_init();
void mouse_add_delta(const Vec2i& delta);
void mouse_end_of_frame();

const Vec2i& mouse_movement_delta();