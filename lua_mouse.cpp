#include "lua_mouse.h"
#include "lua.hpp"
#include "lua_helpers.h"
#include "mouse.h"

static int get_delta(lua_State* L)
{
    Vec2i md = mouse_movement_delta();
    lua_push_vec2i(L, md);
    return 1;
}

static const struct luaL_Reg lib [] = {
    {"get_delta", get_delta},
    {NULL, NULL}
};

void lua_mouse_init(lua_State* L)
{
    luaL_register(L, "mouse", lib);
}