#include "render_world_lua.h"
#include "lua.hpp"
#include "memory.h"
#include "render_world.h"
#include "render_object.h"

static Allocator* alloc;

struct LuaValue
{
    bool valid;
    union {
        long long int_val;
        const char* str_val;
        double float_val;
        void* ptr_val;
    };
};

static LuaValue lua_get_integer(lua_State* L, unsigned arg)
{
    if (!lua_isnumber(L, arg))
    {
        return {false};
    }

    return {true, lua_tointeger(L, arg)};
}

static LuaValue lua_get_ptr(lua_State* L, unsigned arg)
{
    if (!lua_isuserdata(L, arg))
    {
        return {false};
    }

    LuaValue lv = {};
    lv.valid = true;
    lv.ptr_val = lua_touserdata(L, arg);
    return lv;
}

static int create(lua_State* L)
{
    RenderWorld* rw = (RenderWorld*)alloc->alloc(sizeof(RenderWorld));
    render_world_init(rw, alloc);
    lua_pushlightuserdata(L, rw);
    return 1;
}

static int destroy(lua_State* L)
{
    RenderWorld* rw = (RenderWorld*) lua_touserdata(L, 1);
    render_world_destroy(rw);
    alloc->dealloc(rw);
    return 0;
}

static int add(lua_State* L)
{
    LuaValue l_rw = lua_get_ptr(L, 1);

    if (!l_rw.valid)
        Error("ERROR in render_world.add: Expected render world handle in argument 1.");

    RenderWorld* rw = (RenderWorld*)l_rw.ptr_val;
    LuaValue l_int = lua_get_integer(L,2);

    if (!l_int.valid)
        Error("ERROR in render_world.add: Expected geometry handle in argument 2.");

    RenderObject ro = {};
    ro.world_transform = matrix4x4_identity();
    ro.geometry_handle = {(unsigned)l_int.int_val};
    unsigned h = render_world_add(rw, &ro);
    lua_pushnumber(L, h);
    return 1;
}

static const struct luaL_Reg lib [] = {
    {"create", create},
    {"destroy", destroy},
    {"add", add},
    {NULL, NULL}
};

void render_world_lua_init(lua_State* L, Allocator* a)
{
    alloc = a;
    luaL_register(L, "render_world", lib);
}