#include "lua_physics.h"
#include "lua.hpp"
#include "physics.h"
#include "lua_helpers.h"
#include "obj.h"
#include "memory.h"

static int add_mesh(lua_State* L)
{
    LuaValue l_filename = lua_get_string(L, 1);

    if (!l_filename.valid)
        Error("Trying to add physics mesh with invalid filename.");

    LoadedMesh lm = obj_load(l_filename.str_val);

    if (!lm.valid)
        Error("Failed loading mesh.");

    LuaValue l_vec = lua_get_vec3(L, 2);

    if (!l_vec.valid)
        Error("ERROR in physics.add_mesh: Expected Vector3 in argument 2.");

    LuaValue l_quat = lua_get_quat(L, 3);

    if (!l_quat.valid)
        Error("ERROR in physics.add_mesh: Expected Quaternion in argument 3.");

    PhysicsShapeHandle h = physics_add_mesh(lm.mesh, l_vec.vec3_val, l_quat.quat_val);
    zfree(lm.mesh.vertices);
    zfree(lm.mesh.indices);
    lua_pushnumber(L, (lua_Number)h.h);
    return 1;
}

static int intersect(lua_State* L)
{
    LuaValue h1_int = lua_get_integer(L, 1);

    if (!h1_int.valid)
        Error("ERROR in render_object.destroy: Expected PhysicsShapeHandle in argument 1.");

    PhysicsShapeHandle h1 = {(size_t)h1_int.int_val};

    LuaValue h2_int = lua_get_integer(L, 2);

    if (!h2_int.valid)
        Error("ERROR in render_object.destroy: Expected PhysicsShapeHandle in argument 2.");

    PhysicsShapeHandle h2 = {(size_t)h2_int.int_val};
    bool intersect = physics_intersect(h1, h2);
    lua_pushboolean(L, intersect);
    return 1;
}

static int set_shape_position(lua_State* L)
{
    LuaValue h_int = lua_get_integer(L, 1);

    if (!h_int.valid)
        Error("ERROR in physics.set_shape_position: Expected PhysicsShapeHandle in argument 1.");

    PhysicsShapeHandle h = {(size_t)h_int.int_val};
    LuaValue l_vec = lua_get_vec3(L, 2);

    if (!l_vec.valid)
        Error("ERROR in physics.set_shape_position: Expected Vector3 in argument 2.");

    physics_set_shape_position(h, l_vec.vec3_val);
    return 0;
}

static int set_shape_rotation(lua_State* L)
{
    LuaValue h_int = lua_get_integer(L, 1);

    if (!h_int.valid)
        Error("ERROR in physics.set_shape_rotation: Expected PhysicsShapeHandle in argument 1.");

    PhysicsShapeHandle h = {(size_t)h_int.int_val};
    LuaValue l_quat = lua_get_quat(L, 2);

    if (!l_quat.valid)
        Error("ERROR in physics.set_shape_rotation: Expected Quaternion in argument 2.");

    physics_set_shape_rotation(h, l_quat.quat_val);
    return 0;
}

static const struct luaL_Reg lib [] = {
    {"add_mesh", add_mesh},
    {"intersect", intersect},
    {"set_shape_position", set_shape_position},
    {"set_shape_rotation", set_shape_rotation},
    {NULL, NULL}
};

void lua_physics_init(lua_State* L)
{
    luaL_register(L, "physics", lib);
}