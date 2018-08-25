#include <windows.h>
#include "test_world.h"
#include "object.h"
#include "world.h"
#include "renderer_direct3d.h"
#include "mesh.h"
#include "obj.h"
#include "file.h"
#include "memory.h"

static Object create_scaled_box(Renderer* renderer, const Mesh& m, const Vector3& scale, const Vector3& pos, const Color& color, unsigned id, bool is_light)
{
    Vertex* scaled_vertices = m.vertices.clone_raw();
    memcpy(scaled_vertices, m.vertices.data, m.vertices.num * sizeof(Vertex));

    for (unsigned i = 0; i < m.vertices.num; ++i)
    {
        scaled_vertices[i].position = scaled_vertices[i].position * scale;
        scaled_vertices[i].color = color;
    }

    RRHandle box_geometry_handle = renderer->load_geometry(scaled_vertices, m.vertices.num, m.indices.data, m.indices.num);
    Object obj = {};
    obj.geometry_handle = box_geometry_handle;
    obj.world_transform = matrix4x4_identity();
    obj.id = id;
    obj.is_light = is_light;
    memcpy(&obj.world_transform.w.x, &pos.x, sizeof(Vector3));

    return obj;
}

void create_test_world(World* world, Renderer* renderer)
{
    Allocator ta = create_temp_allocator();
    LoadedMesh lm = obj_load(&ta, "box.wobj");

    if (!lm.valid)
        return;

    float floor_width = 6;
    float floor_depth = 8;
    float floor_thickness = 0.3f;
    float floor_to_cieling = 2;
    float pillar_width = 0.4f;

    world->objects.add(create_scaled_box(renderer, lm.mesh, {floor_width, floor_thickness, floor_depth}, {0, 0, 0}, color_random(), 4, false));
    world->objects.add(create_scaled_box(renderer, lm.mesh, {pillar_width, floor_to_cieling, pillar_width}, {-1, (floor_thickness + floor_to_cieling) / 2, 1}, color_random(), 12, false));
    world->objects.add(create_scaled_box(renderer, lm.mesh, {pillar_width, floor_to_cieling, pillar_width}, {-1, (floor_thickness + floor_to_cieling) / 2, -1}, color_random(), 123, false));
    world->objects.add(create_scaled_box(renderer, lm.mesh, {floor_width, floor_thickness, floor_depth}, {0, floor_thickness + floor_to_cieling, 0}, color_random(), 145, false));
    //world->objects.add(create_scaled_box(renderer, lm.mesh, {floor_width, floor_thickness, floor_depth}, {0, floor_thickness + floor_to_cieling - 15, 0}, color::random(), 12333))

    //world->objects.add(create_scaled_box(renderer, lm.mesh, {2,2,2}, {0, 0, 0}, color::random(), 145, false));

    //world->objects.add(create_scaled_box(renderer, lm.mesh, {1,1,1}, vector3::lookdir * 5, color::random(), 145));

    //world::add_object(world, create_scaled_box(renderer, lm.mesh, {1, 1, 1}, {-10, 0, 0}, color::random()));

    if (lm.valid)
    {
        world->objects.add(create_scaled_box(renderer, lm.mesh, {10, 10, 10}, {-20, 25, -19}, {1,1,1,1}, 10000, true));
    }
}
