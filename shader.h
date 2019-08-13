#pragma once
#include <stdint.h>

#define renderer_resource_t uint32_t
struct renderer_state;

renderer_resource_t shader_load(struct renderer_state* rs, const char* filename);