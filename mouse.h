#pragma once
#include "math.h"

void mouse_moved(u32 dx, u32 dy);
void mouse_end_of_frame();
Vec2i mouse_get_delta();