#include "lua_render_world.h"
#include "lua.hpp"
#include "memory.h"
#include "render_world.h"
#include "render_object.h"
#include "lua_helpers.h"

static int create(lua_State* L)
{
    RenderWorld* rw = (RenderWorld*)zalloc_zero(sizeof(RenderWorld));
    lua_pushlightuserdata(L, rw);
    return 1;
}

static int destroy(lua_State* L)
{
    LuaValue l_rw = lua_get_ptr(L, 1);

    if (!l_rw.valid)
        Error("ERROR in render_world.destroy: Expected RenderWorld pointer in argument 1.");

    RenderWorld* rw = (RenderWorld*)l_rw.ptr_val;
    render_world_destroy(rw);
    zfree(rw);
    return 0;
}

static int add(lua_State* L)
{
    LuaValue l_rw = lua_get_ptr(L, 1);

    if (!l_rw.valid)
        Error("ERROR in render_world.add: Expected RenderWorld pointer in argument 1.");

    RenderWorld* rw = (RenderWorld*)l_rw.ptr_val;
    LuaValue l_int = lua_get_integer(L,2);

    if (!l_int.valid)
        Error("ERROR in render_world.add: Expected EntityHandle in argument 2.");

    EntityHandle e = {(size_t)l_int.int_val};
    render_world_add(rw, e);
    return 0;
}

static int remove(lua_State* L)
{
    LuaValue l_rw = lua_get_ptr(L, 1);

    if (!l_rw.valid)
        Error("ERROR in render_world.add: Expected RenderWorld pointer in argument 1.");

    RenderWorld* rw = (RenderWorld*)l_rw.ptr_val;
    LuaValue l_int = lua_get_integer(L,2);

    if (!l_int.valid)
        Error("ERROR in render_world.add: Expected EntityHandle in argument 2.");

    EntityHandle e = {(size_t)l_int.int_val};
    render_world_remove(rw, e);
    return 0;
}

static const struct luaL_Reg lib [] = {
    {"create", create},
    {"destroy", destroy},
    {"add", add},
    {"remove", remove},
    {NULL, NULL}
};

void lua_render_world_init(lua_State* L)
{
    luaL_register(L, "render_world", lib);
}