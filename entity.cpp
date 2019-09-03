#include "entity.h"
#include "renderer.h"
#include "math.h"
#include "debug.h"
#include "world.h"
#include "physics.h"

Entity entity_create(
    World* w,
    const Vec3& pos,
    const Quat& rot)
{
    return { 
        .world = w,
        .handle = world_create_entity(w, pos, rot)
    };
}

#define get_internal() (world_lookup_entity(this->world, this->handle))

void Entity::move(const Vec3& d)
{
    let e = get_internal();
    e->pos += d;

    if (e->physics_object)
        physics_world_set_position(e->world->physics_world, e->physics_object, e->pos, e->rot);

    if (e->render_object)
        renderer_world_set_position_and_rotation(e->world->render_world, e->render_object, e->pos, e->rot);
}


void Entity::rotate(const Vec3& axis, float rad)
{
    let e = get_internal();
    let r = quat_from_axis_angle(axis, rad);
    e->rot *= r;

    if (e->physics_object)
        physics_world_set_position(e->world->physics_world, e->physics_object, e->pos, e->rot);

    if (e->render_object)
        renderer_world_set_position_and_rotation(e->world->render_world, e->render_object, e->pos, e->rot);
}

void Entity::create_rigidbody(f32 mass)
{
    let e = get_internal();
    check(e->physics_object, "Trying to create rigidbody on entity with no physics representation.");
    check(e->physics_rigidbody == NULL, "Trying to add rigidbody to entity twice");
    e->physics_rigidbody = physics_add_rigidbody(this, mass);
}

const Vec3& Entity::get_position() const
{
    return get_internal()->pos;
}

void Entity::set_render_mesh(RenderResourceHandle mesh)
{
    let e = get_internal();
    e->render_object = renderer_world_add(e->world->render_world, mesh, e->pos, e->rot);
}

void Entity::set_physics_collider(PhysicsResourceHandle collider)
{
    let e = get_internal();
    e->physics_object = physics_world_add(e->world->physics_world, collider, e->render_object, e->pos, e->rot);
}

void Entity::set_position(const Vec3& pos)
{
    let e = get_internal();
    e->pos = pos;

    if (e->physics_object)
        physics_world_set_position(e->world->physics_world, e->physics_object, e->pos, e->rot);

    if (e->render_object)
        renderer_world_set_position_and_rotation(e->world->render_world, e->render_object, e->pos, e->rot);
}

void Entity::set_rotation(const Quat& rot)
{
    let e = get_internal();
    e->rot = rot;

    if (e->physics_object)
        physics_world_set_position(e->world->physics_world, e->physics_object, e->pos, e->rot);
    
    if (e->render_object)
        renderer_world_set_position_and_rotation(e->world->render_world, e->render_object, e->pos, e->rot);
}

void Entity::add_force(const Vec3& f)
{
    let e = get_internal();

    if (!e->physics_rigidbody)
        return;

    physics_add_force(e->world->physics_world, e->physics_rigidbody, f);
}

EntityInt* Entity::deref()
{
    return get_internal();
}
