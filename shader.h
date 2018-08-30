#pragma once
#include "render_resource.h"

struct Renderer;

RRHandle shader_load(Renderer* r, const char* filename);