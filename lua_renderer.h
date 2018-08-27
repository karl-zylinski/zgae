#pragma once

struct lua_State;
struct Renderer;

void lua_renderer_init(lua_State* L, Renderer* r);