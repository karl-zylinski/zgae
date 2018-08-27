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

    // TODO: get proj matrix from lua!!
    float near_plane = 0.01f;
    float far_plane = 1000.0f;
    float fov = 90.0f;
    float aspect = 1.0f;
    float y_scale = 1.0f / tanf((3.14f / 180.0f) * fov / 2);
    float x_scale = y_scale / aspect;
    Matrix4x4 proj = {
        x_scale, 0, 0, 0,
        0, y_scale, 0, 0,
        0, 0, far_plane/(far_plane-near_plane), 1,
        0, 0, (-far_plane * near_plane) / (far_plane - near_plane), 0 
    };
    renderer->draw_world(*rw, camera_rot, camera_pos, proj, DrawLights::DrawLights);
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