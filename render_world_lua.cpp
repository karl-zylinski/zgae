#include "render_world_lua.h"
#include "lua.hpp"
#include "memory.h"
#include "render_world.h"
#include "render_object.h"
#include "lauxlib.h"
#include "lualib.h"

static Allocator* alloc;

static int create(lua_State* L)
{
    RenderWorld* rw = (RenderWorld*)alloc->alloc(sizeof(RenderWorld));
    render_world_init(rw, alloc);
    lua_pushlightuserdata(L, rw);
    return 1;
}

static int add(lua_State* L)
{
    RenderWorld* rw = (RenderWorld*) lua_touserdata(L, 1);
    unsigned geometry_handle = (unsigned)luaL_checkinteger(L, 2);
    RenderObject ro = {};
    ro.geometry_handle = {geometry_handle};
    unsigned h = render_world_add(rw, &ro);
    lua_pushnumber(L, h);
    return 1;
}

static const struct luaL_Reg lib [] = {
    {"create", create},
    {"add", add},
    {NULL, NULL}
};

void render_world_lua_init(lua_State* L, Allocator* a)
{
    alloc = a;
    luaL_register(L, "render_world", lib);
}