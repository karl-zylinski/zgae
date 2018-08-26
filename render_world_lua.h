#pragma once

struct lua_State;
struct Allocator;

void render_world_lua_init(lua_State* L, Allocator* a);