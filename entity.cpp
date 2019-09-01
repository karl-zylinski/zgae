#include "entity.h"
#include "renderer.h"
#include "physics.h"
#include "math.h"

Entity entity_create(
    const Vec3& pos,
    const Quat& rot, 
    RenderResourceHandle render_world,
    RenderResourceHandle mesh,
    PhysicsResourceHandle physics_world,
    PhysicsResourceHandle collider)
{
    let render_world_handle = renderer_world_add(render_world, mesh, pos, rot);
    let physics_world_handle = physics_world_add(physics_world, collider, render_world_handle, pos, rot);

    return {
        .position = pos,
        .rotation = rot,
        .physics_world = physics_world,
        .render_object = render_world_handle,
        .physics_object = physics_world_handle
    };
}

void entity_move(Entity* e, const Vec3& d)
{
    e->position += d;
    physics_world_set_position(e->physics_world, e->physics_object, e->position, e->rotation);
}