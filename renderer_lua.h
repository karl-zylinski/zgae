#pragma once

struct lua_State;
struct Renderer;
struct Camera;

void renderer_lua_init(lua_State* L, Renderer* r, Camera* tc);