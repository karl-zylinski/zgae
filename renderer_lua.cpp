#include "renderer_lua.h"
#include "obj.h"
#include "memory.h"
#include "lua.hpp"
#include "renderer.h"

static Renderer* renderer;
static Camera* temp_cam;

static int load_geometry_obj(lua_State* L)
{
    const char* filename = luaL_checkstring(L, 1);
    Allocator ta = create_temp_allocator();
    LoadedMesh lm = obj_load(&ta, filename);

    if (!lm.valid)
        Error("OHN OES");

    RRHandle h = renderer->load_mesh(&lm.mesh);
    lua_pushnumber(L, h.h);
    return 1;
}

static int draw_world(lua_State* L)
{
    RenderWorld* rw = (RenderWorld*) lua_touserdata(L, 1);
    renderer->draw_world(*rw, *temp_cam, DrawLights::DrawLights);
    return 0;
}

static const struct luaL_Reg lib [] = {
    {"draw_world", draw_world},
    {"load_geometry_obj", load_geometry_obj},
    {NULL, NULL}
};

void renderer_lua_init(lua_State* L, Renderer* r, Camera* tc)
{
    renderer = r;
    temp_cam = tc;
    luaL_register(L, "renderer", lib);
}