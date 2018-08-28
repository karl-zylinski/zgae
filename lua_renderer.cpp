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
    const char* filename = luaL_checkstring(L, 1);
    LoadedMesh lm = obj_load(filename);

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
    RenderWorld* rw = (RenderWorld*) lua_touserdata(L, 1);

    LuaValue lq = lua_get_quat(L, 2);

    if (!lq.valid)
        Error("ERROR in renderer.draw_world: Expected Quaternion in argument 2.");

    LuaValue lv3 = lua_get_vec3(L, 3);

    if (!lv3.valid)
        Error("ERROR in renderer.draw_world: Expected Vector3 in argument 3.");

    Quaternion camera_rot = lq.quat_val;
    Vector3 camera_pos = lv3.vec3_val;

    renderer->draw_world(*rw, camera_rot, camera_pos, DrawLights::DrawLights);
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