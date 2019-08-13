#pragma once

enum window_type;
struct renderer_state;

struct renderer_state* renderer_init(enum window_type wt, void* window_data);
void renderer_shutdown(struct renderer_state* rs);
//void renderer_load_shader(renderer_state_t* rs, const shader_intermediate_t* si);