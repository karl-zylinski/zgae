#pragma once

struct lua_State;
struct Renderer;

void renderer_lua_init(lua_State* L, Renderer* r);