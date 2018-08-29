#include "lua_physics.h"
#include "lua.hpp"
#include "physics.h"
#include "lua_helpers.h"
#include "obj.h"
#include "memory.h"

static int create_collider(lua_State* L)
{

    return 1;
}

static int create_mesh_collider(lua_State* L)
{
    LuaValue l_filename = lua_get_string(L, 1);

    if (!l_filename.valid)
        Error("Trying to add physics mesh with invalid filename.");

    LoadedMesh lm = obj_load(l_filename.str_val);

    if (!lm.valid)
        Error("Failed loading mesh.");

    ColliderHandle h = physics_create_mesh_collider(lm.mesh);
    zfree(lm.mesh.vertices);
    zfree(lm.mesh.indices);
    lua_pushnumber(L, (lua_Number)h.h);
    return 1;
}

static int intersect(lua_State* L)
{
    LuaValue h1_int = lua_get_integer(L, 1);

    if (!h1_int.valid)
        Error("ERROR in render_object.intersect: Expected ColliderHandle in argument 1.");

    ColliderHandle h1 = {(size_t)h1_int.int_val};

    LuaValue h2_int = lua_get_integer(L, 2);

    if (!h2_int.valid)
        Error("ERROR in render_object.intersect: Expected ColliderHandle in argument 2.");

    ColliderHandle h2 = {(size_t)h2_int.int_val};
    bool intersect = physics_intersect(h1, h2);
    lua_pushboolean(L, intersect);
    return 1;
}

static int set_collider_position(lua_State* L)
{
    LuaValue h_int = lua_get_integer(L, 1);

    if (!h_int.valid)
        Error("ERROR in physics.set_collider_position: Expected ColliderHandle in argument 1.");

    ColliderHandle h = {(size_t)h_int.int_val};
    LuaValue l_vec = lua_get_vec3(L, 2);

    if (!l_vec.valid)
        Error("ERROR in physics.set_collider_position: Expected Vector3 in argument 2.");

    physics_set_collider_position(h, l_vec.vec3_val);
    return 0;
}

static int set_collider_rotation(lua_State* L)
{
    LuaValue h_int = lua_get_integer(L, 1);

    if (!h_int.valid)
        Error("ERROR in physics.set_collider_rotation: Expected ColliderHandle in argument 1.");

    ColliderHandle h = {(size_t)h_int.int_val};
    LuaValue l_quat = lua_get_quat(L, 2);

    if (!l_quat.valid)
        Error("ERROR in physics.set_collider_rotation: Expected Quaternion in argument 2.");

    physics_set_collider_rotation(h, l_quat.quat_val);
    return 0;
}

static const struct luaL_Reg lib [] = {
    {"create_mesh_collider", create_mesh_collider},
    {"intersect", intersect},
    {"set_collider_position", set_collider_position},
    {"set_collider_rotation", set_collider_rotation},
    {NULL, NULL}
};

void lua_physics_init(lua_State* L)
{
    luaL_register(L, "physics", lib);
}