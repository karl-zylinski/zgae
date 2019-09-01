#include "entity.h"
#include "renderer.h"
#include "physics.h"
#include "math.h"
#include "debug.h"
#include "world_types.h"
#include "dynamic_array.h"
#include "world.h"

EntityRef entity_create(
    World* w,
    const Vec3& pos,
    const Quat& rot)
{
    let world_entity_handle = world_create_entity(w, pos, rot);
    return {
        .world = w,
        .handle = world_entity_handle
    };
}

void entity_move(EntityRef* er, const Vec3& d)
{
    let e = world_lookup_entity(er->world, er->handle);
    e->pos += d;

    if (e->physics_object)
        physics_world_set_position(e->world->physics_world, e->physics_object, e->pos, e->rot);
}

void entity_create_rigidbody(EntityRef* er)
{
    let e = world_lookup_entity(er->world, er->handle);
    check(e->physics_object, "Trying to create rigidbody on entity with no physics representation.");
    check(e->physics_rigidbody == NULL, "Trying to add rigidbody to entity twice");
    e->physics_rigidbody = physics_add_rigidbody(e);
}

Vec3 entity_get_position(const EntityRef& er)
{
    return world_lookup_entity(er.world, er.handle)->pos;
}

void entity_set_render_mesh(EntityRef* er, RenderResourceHandle mesh)
{
    let e = world_lookup_entity(er->world, er->handle);
    e->render_object = renderer_world_add(er->world->render_world, mesh, e->pos, e->rot);
}

void entity_set_physics_collider(EntityRef* er, PhysicsResourceHandle collider)
{
    let e = world_lookup_entity(er->world, er->handle);
    e->physics_object = physics_world_add(er->world->physics_world, collider, e->render_object, e->pos, e->rot);
}