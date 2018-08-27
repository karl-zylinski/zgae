#include "game_main.h"
#include "render_world.h"
#include "memory.h"
#include "keyboard.h"
#include "mouse.h"
#include "renderer.h"
#include "lua_renderer.h"
#include "lua_render_world.h"
#include "lua_keyboard.h"
#include "render_object.h"
#include "mesh.h"
#include "lua.hpp"
#include <cmath>

struct GameState
{
    lua_State* lua_state;
};

static GameState state;

static Vector3 get_spherical_coords(unsigned lat_number, unsigned long_number, unsigned latitude_bands, unsigned longitude_bands)
{
    float theta = lat_number * PI / latitude_bands;
    float sin_theta = sinf(theta);
    float cos_theta = cosf(theta);

    float phi = long_number * 2 * PI / longitude_bands;
    float sin_phi = sinf(phi);
    float cos_phi = cosf(phi);

    float x = cos_phi * sin_theta;
    float y = cos_theta;
    float z = sin_phi * sin_theta;

    return { x, y, z };
}

static Mesh create_sphere(float radius)
{
    unsigned latitude_bands = 16;
    unsigned longitude_bands = latitude_bands;
    Mesh m = {};
    m.num_vertices = (latitude_bands - 1)*longitude_bands + 2;
    m.num_indices = (latitude_bands - 2)*longitude_bands * 6 + 2*longitude_bands*3;
    m.vertices = (Vertex*)zalloc_zero(m.num_vertices * sizeof(Vertex));
    m.indices = (unsigned*)zalloc_zero(m.num_indices * sizeof(unsigned));
    unsigned cur_vertex = 0;
    unsigned cur_index = 0;

    Vector3 top_coord{0, 1, 0};
    m.vertices[0].position = top_coord * radius;
    m.vertices[0].normal = top_coord;
    m.vertices[0].color = color_random();
    ++cur_vertex;

    for (unsigned long_number = 0; long_number < longitude_bands; long_number++)
    {
        m.indices[cur_index] = 0;

        if (long_number + 2 > longitude_bands)
        {
            m.indices[cur_index + 1] = 1;
            m.indices[cur_index + 2] = longitude_bands;
        }
        else
        {
            m.indices[cur_index + 1] = long_number + 2;
            m.indices[cur_index + 2] = long_number + 1;
        }
        
        cur_index += 3;
    }

    for (unsigned lat_number = 1; lat_number < latitude_bands; lat_number++)
    {
        for (unsigned long_number = 0; long_number < longitude_bands; long_number++)
        {
            Vector3 coords = get_spherical_coords(lat_number, long_number, latitude_bands, longitude_bands);
            m.vertices[cur_vertex].position = coords * radius;
            m.vertices[cur_vertex].normal = coords;
            m.vertices[cur_vertex].color = color_random();

            if (lat_number == latitude_bands - 1)
            {
                if (long_number == longitude_bands - 1)
                {
                    m.indices[cur_index] = cur_vertex;
                    m.indices[cur_index + 1] = cur_vertex - longitude_bands + 1;
                    m.indices[cur_index + 2] = m.num_vertices - 1;
                }
                else
                {
                    m.indices[cur_index] = cur_vertex;
                    m.indices[cur_index + 1] = cur_vertex + 1;
                    m.indices[cur_index + 2] = m.num_vertices - 1;
                }

                cur_index += 3;
            }
            else
            {
                if (long_number == longitude_bands - 1)
                {
                    m.indices[cur_index] = cur_vertex;
                    m.indices[cur_index + 1] = cur_vertex - longitude_bands + 1;
                    m.indices[cur_index + 2] = cur_vertex + longitude_bands;
                    m.indices[cur_index + 3] = cur_vertex - longitude_bands + 1;
                    m.indices[cur_index + 4] = cur_vertex + 1;
                    m.indices[cur_index + 5] = cur_vertex + longitude_bands;
                }
                else if (lat_number < latitude_bands - 1)
                {
                    m.indices[cur_index] = cur_vertex;
                    m.indices[cur_index + 1] = cur_vertex + 1;
                    m.indices[cur_index + 2] = cur_vertex + longitude_bands;
                    m.indices[cur_index + 3] = cur_vertex + 1;
                    m.indices[cur_index + 4] = cur_vertex + longitude_bands + 1;
                    m.indices[cur_index + 5] = cur_vertex + longitude_bands;
                }

                cur_index += 6;
            }
            cur_vertex += 1;
        }
    }


    Vector3 bottom_coord{0, -1, 0};
    m.vertices[m.num_vertices - 1].position = bottom_coord * radius;
    m.vertices[m.num_vertices - 1].normal = bottom_coord;
    m.vertices[m.num_vertices - 1].color = color_random();
    ++cur_vertex;

    Assert(cur_vertex == m.num_vertices, "Num vertices does not match in sphere creation.");
    Assert(cur_index == m.num_indices, "Num indices does not match in sphere creation.");

    return m;
}

static void run_lua_func(lua_State* L, const char* func)
{
    lua_getglobal(L, func);
    
    if (lua_pcall(L, 0, 0, 0) != 0)
        Error("error running function");
}

void game_start(Renderer* renderer)
{
    memzero(&state, sizeof(GameState));
    lua_State* L = luaL_newstate();
    state.lua_state = L;
    luaL_openlibs(L);
    lua_renderer_init(L, renderer);
    lua_render_world_init(L);
    lua_keyboard_init(L);

    if (luaL_dofile(L, "game/class.lua") != 0 )
        Error("Failed running 'game/class.lua'");

    if (luaL_dofile(L, "game/main.lua") != 0 )
    {
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
        Error("Failed running 'game/main.lua'");
    }

    run_lua_func(L, "start");
}

void game_update(Renderer* renderer)
{
    run_lua_func(state.lua_state, "update");
}

void game_draw(Renderer* renderer)
{
    run_lua_func(state.lua_state, "draw");
}

void game_shutdown(Renderer* renderer)
{
    run_lua_func(state.lua_state, "shutdown");
}