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
#include "window_state.h"
#include "shader.h"

struct GameState
{
    lua_State* lua_state;
};

static GameState _state;
static Renderer* _renderer;
static WindowState* _window_state;

static void run_lua_func(lua_State* L, const char* func)
{
    lua_getglobal(L, func);
    
    if (lua_pcall(L, 0, 0, 0) != 0)
    {
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
        Error("error running function");
    }
}


static void key_pressed_callback(Key key)
{
    keyboard_pressed(key);
}

static void key_released_callback(Key key)
{
    keyboard_released(key);
}

static void mouse_moved_callback(const Vec2i& delta)
{
    mouse_add_delta(delta);
}

static void window_resized_callback(unsigned width, unsigned height)
{
    _renderer->resize_back_buffer(width, height);
}

void game_start(WindowState* window_state, Renderer* renderer)
{
    _window_state = window_state;
    _renderer = renderer;
    _window_state->key_released_callback = key_released_callback;
    _window_state->key_pressed_callback = key_pressed_callback;
    _window_state->mouse_moved_callback = mouse_moved_callback;
    _window_state->resized_callback = window_resized_callback;
    RRHandle default_shader = shader_load(_renderer, "shader_default.shader");
    _renderer->set_shader(default_shader);
    keyboard_init();
    mouse_init();
    memzero(&_state, sizeof(GameState));
    lua_State* L = luaL_newstate();
    _state.lua_state = L;
    luaL_openlibs(L);
    lua_renderer_init(L, _renderer);
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

void game_do_frame()
{
    _renderer->pre_frame();
    run_lua_func(_state.lua_state, "update");
    run_lua_func(_state.lua_state, "draw");
    _renderer->present();
    keyboard_end_of_frame();
    mouse_end_of_frame();
}

void game_shutdown()
{
    run_lua_func(_state.lua_state, "shutdown");
    render_object_deinit_lut();
    physics_shutdown();
}