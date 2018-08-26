#include "renderer_lua.h"
#include "obj.h"
#include "memory.h"
#include "lua.hpp"
#include "renderer.h"

static Renderer* renderer;

static int load_obj_geometry(lua_State* L)
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

void renderer_lua_init(lua_State* L, Renderer* r)
{
    renderer = r;
    lua_pushcfunction(L, load_obj_geometry);
    lua_setglobal(L, "load_obj_geometry");
}