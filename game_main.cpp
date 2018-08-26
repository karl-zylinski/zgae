#include "game_main.h"
#include "world.h"
#include "camera.h"
#include "memory.h"
#include "keyboard.h"
#include "mouse.h"
#include "renderer.h"
#include "mesh.h"
#include <cmath>

struct GameState
{
    World world;
    Camera camera;
    Allocator def_alloc;
};

static GameState state;


static void process_input(Camera* camera)
{
    Matrix4x4 move = matrix4x4_identity();

    if (key_is_held(Key::W))
    {
        move.w.z += 0.0005f;
    }
    if (key_is_held(Key::S))
    {
        move.w.z -= 0.0005f;
    }
    if (key_is_held(Key::A))
    {
        move.w.x -= 0.0005f;
    }
    if (key_is_held(Key::D))
    {
        move.w.x += 0.0005f;
    }

    Quaternion rotation = camera->rotation;
    Vector2i mouse_movement = mouse_movement_delta();
    if (mouse_movement.x != 0 || mouse_movement.y != 0)
    {
        rotation = quaternion_rotate_y(rotation, mouse_movement.x * 0.001f);
        rotation = quaternion_rotate_x(rotation, mouse_movement.y * 0.001f);
    }

    camera->rotation = rotation;
    Matrix4x4 camera_test_mat = matrix4x4_from_rotation_and_translation(camera->rotation, vector3_zero);
    Matrix4x4 movement_rotated = move * camera_test_mat;
    camera->position += *(Vector3*)&movement_rotated.w.x;
}

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

static Mesh create_sphere(Allocator* alloc, float radius)
{
    unsigned latitude_bands = 16;
    unsigned longitude_bands = latitude_bands;
    Mesh m = {};
    m.num_vertices = (latitude_bands - 1)*longitude_bands + 2;
    m.num_indices = (latitude_bands - 2)*longitude_bands * 6 + 2*longitude_bands*3;
    m.vertices = (Vertex*)alloc->alloc_zero(m.num_vertices * sizeof(Vertex));
    m.indices = (unsigned*)alloc->alloc_zero(m.num_indices * sizeof(unsigned));
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


void game_start(Renderer* renderer)
{
    memzero(&state, GameState);
    state.def_alloc = create_heap_allocator();
    world_init(&state.world, &state.def_alloc);
    state.camera = camera_create_projection();
    state.camera.position = Vector3{0, 0, -5};
    Mesh m = create_sphere(&state.def_alloc, 1);
    Object sph = {};
    sph.geometry_handle = renderer->load_mesh(&m);
    sph.world_transform = matrix4x4_identity();
    state.world.objects.add(sph);
}

void game_update(Renderer* renderer)
{
    process_input(&state.camera);
}

void game_draw(Renderer* renderer)
{
    renderer->draw_frame(state.world, state.camera, DrawLights::DrawLights);
}

void game_shutdown(Renderer* renderer)
{
    world_destroy(&state.world);
    //heap_allocator_check_clean(&state.def_alloc);
}