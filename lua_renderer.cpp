#include "lua_renderer.h"
#include "obj.h"
#include "memory.h"
#include "lua.hpp"
#include "renderer.h"
#include <cmath>
#include "lua_helpers.h"

static Renderer* renderer;

static int load_geometry_obj(lua_State* L)
{
    LuaValue l_str = lua_get_string(L, 1);

    if (!l_str.valid)
        Error("ERROR in renderer.load_geometry_obj: Expected string in argument 1.");

    LoadedMesh lm = obj_load(l_str.str_val);

    if (!lm.valid)
        Error("OHN OES");

    RRHandle h = renderer->load_mesh(&lm.mesh);
    zfree(lm.mesh.vertices);
    zfree(lm.mesh.indices);
    lua_pushnumber(L, h.h);
    return 1;
}


static int draw_world(lua_State* L)
{
    LuaValue l_rw = lua_get_ptr(L, 1);

    if (!l_rw.valid)
        Error("ERROR in renderer.draw_world: Expected RenderWorld pointer in argument 1.");

    RenderWorld* rw = (RenderWorld*)l_rw.ptr_val;
    LuaValue lq = lua_get_quat(L, 2);

    if (!lq.valid)
        Error("ERROR in renderer.draw_world: Expected Quat in argument 2.");

    LuaValue lv3 = lua_get_vec3(L, 3);

    if (!lv3.valid)
        Error("ERROR in renderer.draw_world: Expected Vec3 in argument 3.");

    Quat camera_rot = lq.quat_val;
    Vec3 camera_pos = lv3.vec3_val;

    renderer->draw_world(*rw, camera_rot, camera_pos);
    return 0;
}

static const struct luaL_Reg lib [] = {
    {"draw_world", draw_world},
    {"load_geometry_obj", load_geometry_obj},
    {NULL, NULL}
};

void lua_renderer_init(lua_State* L, Renderer* r)
{
    renderer = r;
    luaL_register(L, "renderer", lib);
}