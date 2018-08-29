#include "lua_mouse.h"
#include "lua.hpp"
#include "lua_helpers.h"
#include "mouse.h"

static int get_delta(lua_State* L)
{
    Vec2i md = mouse_movement_delta();
    lua_createtable(L, 0, 2);
    lua_pushstring(L, "x");
    lua_pushnumber(L, md.x);
    lua_settable(L, -3);
    lua_pushstring(L, "y");
    lua_pushnumber(L, md.y);
    lua_settable(L, -3);
    /*lua_getglobal(L, "Vec2");
    int lax = lua_getmetatable(L, -1);
    (void)lax;
    lua_setmetatable(L, -2);
    lua_getfield(L, -1, "__index");
    lua_setfield(L, -3, "__index");
    lua_pop(L, 1);*/
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