#include "lua_render_object.h"
#include "lua.hpp"
#include "render_object.h"
#include "lua_helpers.h"

static int create(lua_State* L)
{
    RenderObjectHandle h = render_object_create();
    lua_pushnumber(L, (lua_Number)h.h);
    return 1;
}

static int destroy(lua_State* L)
{
    LuaValue h_int = lua_get_integer(L, 1);

    if (!h_int.valid)
        Error("ERROR in render_object.destroy: Expected RenderObject handle in argument 1.");

    RenderObjectHandle h = {(size_t)h_int.int_val};
    render_object_destroy(h);
    return 0;
}

static int set_geometry(lua_State* L)
{
    LuaValue l_roh = lua_get_integer(L, 1);

    if (!l_roh.valid)
        Error("ERROR in render_object.set_geometry: Expected RenderObjectHandle in argument 1.");

    RenderObjectHandle h = {(size_t)l_roh.int_val};

    LuaValue l_rrh = lua_get_integer(L, 2);

    if (!l_rrh.valid)
        Error("ERROR in render_object.set_geometry: Expected RenderResourceHandle in argument 2.");

    RRHandle rrh = {(unsigned)l_rrh.int_val};
    RenderObject* ro = render_object_get(h);
    ro->geometry_handle = rrh;
    return 0;
}

static int set_position(lua_State* L)
{
    LuaValue l_roh = lua_get_integer(L, 1);

    if (!l_roh.valid)
        Error("ERROR in render_object.set_position: Expected RenderObject handle in argument 1.");

    RenderObjectHandle h = {(size_t)l_roh.int_val};

    LuaValue l_vec = lua_get_vec3(L, 2);

    if (!l_vec.valid)
        Error("ERROR in render_object.set_position: Expected Vec3 in argument 2.");

    Vec3 pos = l_vec.vec3_val;
    RenderObject* ro = render_object_get(h);
    ro->position = pos;
    ro->world_transform = mat4_from_rotation_and_translation(ro->rotation, ro->position);
    return 0;
}

static int set_rotation(lua_State* L)
{
    LuaValue l_roh = lua_get_integer(L, 1);

    if (!l_roh.valid)
        Error("ERROR in render_object.set_rotation: Expected RenderObject handle in argument 1.");

    RenderObjectHandle h = {(size_t)l_roh.int_val};

    LuaValue l_quat = lua_get_quat(L, 2);

    if (!l_quat.valid)
        Error("ERROR in render_object.set_rotation: Expected Quat in argument 2.");

    Quat rot = l_quat.quat_val;
    RenderObject* ro = render_object_get(h);
    ro->rotation = rot;
    ro->world_transform = mat4_from_rotation_and_translation(ro->rotation, ro->position);
    return 0;
}

static const struct luaL_Reg lib [] = {
    {"create", create},
    {"destroy", destroy},
    {"set_geometry", set_geometry},
    {"set_position", set_position},
    {"set_rotation", set_rotation},
    {NULL, NULL}
};

void lua_render_object_init(lua_State* L)
{
    luaL_register(L, "render_object", lib);
}