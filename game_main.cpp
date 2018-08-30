#include "game_main.h"
#include "keyboard.h"
#include "mouse.h"
#include "lua_renderer.h"
#include "lua_render_world.h"
#include "lua_keyboard.h"
#include "lua_mouse.h"
#include "lua_render_object.h"
#include "lua_time.h"
#include "lua_physics.h"
#include "renderer.h"
#include "render_object.h"
#include "physics.h"
#include "lua.hpp"
#include "debug.h"

struct GameState
{
    lua_State* lua_state;
};

static GameState state;

static void run_lua_func(lua_State* L, const char* func)
{
    lua_getglobal(L, func);
    
    if (lua_pcall(L, 0, 0, 0) != 0)
    {
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
        Error("error running function");
    }
}

void game_start(Renderer* renderer)
{
    debug_init(renderer);
    keyboard_init();
    mouse_init();
    memzero(&state, sizeof(GameState));
    lua_State* L = luaL_newstate();
    state.lua_state = L;
    luaL_openlibs(L);
    lua_renderer_init(L, renderer);
    lua_render_world_init(L);
    lua_physics_init(L);
    lua_keyboard_init(L);
    lua_mouse_init(L);
    lua_render_object_init(L);
    lua_time_init(L);
    luaL_dostring(L, "package.path = \"./game\"..\"/?.lua;\"..\"./game\"..\"/?/init.lua;\"..package.path");
    luaL_dostring(L, "package.cpath = \"./game\"..\"/?.so;\"..package.cpath");

    const char* files_to_run[] = {
        "game/helpers.lua",
        "game/class.lua",
        "game/math.lua",
        "game/main.lua",
        NULL
    };

    const char** to_run = files_to_run;
    while (*to_run != NULL)
    {
        if (luaL_dofile(L, *to_run) != 0)
        {
            fprintf(stderr, "%s\n", lua_tostring(L, -1));
            Error("Failed running script");
        }

        ++to_run;
    }

    run_lua_func(L, "start");
}

static void update(Renderer* renderer)
{
    run_lua_func(state.lua_state, "update");
}

static void draw(Renderer* renderer)
{
    run_lua_func(state.lua_state, "draw");
}

void game_do_frame(Renderer* renderer)
{
    renderer->pre_frame();
    update(renderer);
    draw(renderer);
    renderer->present();
    keyboard_end_of_frame();
    mouse_end_of_frame();
}

void game_shutdown(Renderer* renderer)
{
    run_lua_func(state.lua_state, "shutdown");
    render_object_deinit_lut();
    physics_shutdown();
}