#pragma once

struct Vector2i;

void mouse_init();
void mouse_add_delta(const Vector2i& delta);
void mouse_end_of_frame();

const Vector2i& mouse_movement_delta();