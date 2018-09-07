#include "lua_time.h"
#include "time.h"
#include "lua.hpp"
#include "lua_helpers.h"

static int dt(lua_State* L)
{
    lua_pushnumber(L, time_dt());
    return 1;
}

static int since_start(lua_State* L)
{
    lua_pushnumber(L, time_since_start());
    return 1;
}

static const struct LuaCFunction funcs[] = {
    {"time_dt", dt},
    {"time_since_start", since_start},
    {NULL, NULL}
};

void lua_time_init(lua_State* L)
{
    register_lua_functions(L, funcs);
}