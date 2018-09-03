#include "lua_entity.h"
#include "lua.hpp"
#include "entity.h"
#include "lua_helpers.h"

static int create(lua_State* L)
{
    LuaValue l_vec = lua_get_vec3(L, 1);

    if (!l_vec.valid)
        Error("ERROR in entity.create: Expected Vec3 in argument 1.");

    LuaValue l_q = lua_get_quat(L, 2);

    if (!l_q.valid)
        Error("ERROR in entity.create: Expected Quat in argument 2.");
    
    LuaValue l_rrh = lua_get_integer(L, 3); // ok to be zero
    EntityHandle e = entity_create(l_vec.vec3_val, l_q.quat_val, {(unsigned)l_rrh.int_val});
    lua_pushnumber(L, (lua_Number)e);
    return 1;
}

static int destroy(lua_State* L)
{
    LuaValue h_int = lua_get_integer(L, 1);

    if (!h_int.valid)
        Error("ERROR in entity.destroy: Expected EntityHandle in argument 1.");

    entity_destroy((size_t)h_int.int_val);
    return 0;
}


static int set_position(lua_State* L)
{
    LuaValue l_h = lua_get_integer(L, 1);

    if (!l_h.valid)
        Error("ERROR in entity.set_position: Expected EntityHandle in argument 1.");

    LuaValue l_vec = lua_get_vec3(L, 2);

    if (!l_vec.valid)
        Error("ERROR in entity.set_position: Expected Vec3 in argument 2.");

    entity_set_position(l_h.int_val, l_vec.vec3_val);
    return 0;
}

static int set_rotation(lua_State* L)
{
    LuaValue l_h = lua_get_integer(L, 1);

    if (!l_h.valid)
        Error("ERROR in entity.set_rotation: Expected EntityHandle in argument 1.");

    LuaValue l_quat = lua_get_quat(L, 2);

    if (!l_quat.valid)
        Error("ERROR in entity.set_rotation: Expected Quat in argument 2.");

    entity_set_rotation(l_h.int_val, l_quat.quat_val);
    return 0;
}

static int set_collider(lua_State* L)
{
    LuaValue l_h = lua_get_integer(L, 1);

    if (!l_h.valid)
        Error("ERROR in entity.set_collider: Expected EntityHandle in argument 1.");

    LuaValue l_ch = lua_get_integer(L, 2);

    if (!l_ch.valid)
        Error("ERROR in entity.set_collider: Expected ColliderHandle in argument 2.");

    entity_set_collider((size_t)l_h.int_val, {(unsigned)l_ch.int_val});
    return 0;
}

static int get_position(lua_State* L)
{
    LuaValue l_h = lua_get_integer(L, 1);

    if (!l_h.valid)
        Error("ERROR in entity.get_position: Expected EntityHandle in argument 1.");

    lua_push_vec3(L, entity_get_position(l_h.int_val));
    return 1;
}

static int get_rotation(lua_State* L)
{
    LuaValue l_h = lua_get_integer(L, 1);

    if (!l_h.valid)
        Error("ERROR in entity.get_rotation: Expected EntityHandle in argument 1.");

    lua_push_quat(L, entity_get_rotation(l_h.int_val));
    return 1;
}

static int move(lua_State* L)
{
    LuaValue l_h = lua_get_integer(L, 1);

    if (!l_h.valid)
        Error("ERROR in entity.move: Expected EntityHandle in argument 1.");

    LuaValue l_vec = lua_get_vec3(L, 2);

    if (!l_vec.valid)
        Error("ERROR in entity.move: Expected Vec3 in argument 2.");

    entity_set_position(l_h.int_val, entity_get_position(l_h.int_val) + l_vec.vec3_val);
    return 0;
}

static int intersects(lua_State* L)
{
    LuaValue l_h1 = lua_get_integer(L, 1);

    if (!l_h1.valid)
        Error("ERROR in entity.intersects: Expected EntityHandle in argument 1.");

    LuaValue l_h2 = lua_get_integer(L, 2);

    if (!l_h2.valid)
        Error("ERROR in entity.intersects: Expected EntityHandle in argument 2.");

    lua_pushboolean(L, entity_intersects(l_h1.int_val, l_h2.int_val));
    return 1;
}

static const struct luaL_Reg lib [] = {
    {"create", create},
    {"destroy", destroy},
    {"set_collider", set_collider},
    {"set_position", set_position},
    {"set_rotation", set_rotation},
    {"get_position", get_position},
    {"get_rotation", get_rotation},
    {"move", move},
    {"intersects", intersects},
    {NULL, NULL}
};

void lua_entity_init(lua_State* L)
{
    luaL_register(L, "entity", lib);
}