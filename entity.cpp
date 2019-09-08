#include "entity.h"
#include "log.h"
#include "world.h"

Entity entity_create(
    World* w,
    const Vec3& pos,
    const Quat& rot)
{
    return { 
        .world = w,
        .handle = w->create_entity(pos, rot)
    };
}

#define get_internal() (this->world->lookup_entity(this->handle))

void Entity::move(const Vec3& d)
{
    let e = get_internal();
    e->pos += d;

    if (e->physics_object)
        physics_set_position(e->world->physics_world, e->physics_object, e->pos, e->rot);

    if (e->render_object)
        renderer_world_set_position_and_rotation(e->world->render_world, e->render_object, e->pos, e->rot);
}

void Entity::rotate(const Vec3& axis, float rad)
{
    let e = get_internal();
    let r = quat_from_axis_angle(axis, rad);
    e->rot *= r;

    if (e->physics_object)
        physics_set_position(e->world->physics_world, e->physics_object, e->pos, e->rot);

    if (e->render_object)
        renderer_world_set_position_and_rotation(e->world->render_world, e->render_object, e->pos, e->rot);
}

void Entity::create_rigidbody(f32 mass)
{
    let e = get_internal();
    check(e->physics_object, "Trying to create rigidbody on entity with no physics representation.");
    check(e->physics_rigidbody == NULL, "Trying to add rigidbody to entity twice");
    e->physics_rigidbody = physics_create_rigidbody(this, mass);
}

const Vec3& Entity::get_position() const
{
    return get_internal()->pos;
}

void Entity::set_render_mesh(RenderResourceHandle mesh)
{
    let e = get_internal();
    e->render_object = renderer_create_object(e->world->render_world, mesh, e->pos, e->rot);
}

void Entity::set_physics_collider(PhysicsResourceHandle collider)
{
    let e = get_internal();
    e->physics_object = physics_create_object(e->world->physics_world, collider, e->render_object, e->pos, e->rot);
}

void Entity::set_position(const Vec3& pos)
{
    let e = get_internal();
    e->pos = pos;

    if (e->physics_object)
        physics_set_position(e->world->physics_world, e->physics_object, e->pos, e->rot);

    if (e->render_object)
        renderer_world_set_position_and_rotation(e->world->render_world, e->render_object, e->pos, e->rot);
}

void Entity::set_rotation(const Quat& rot)
{
    let e = get_internal();
    e->rot = rot;

    if (e->physics_object)
        physics_set_position(e->world->physics_world, e->physics_object, e->pos, e->rot);
    
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

void Entity::add_torque(const Vec3& pivot, const Vec3& point, const Vec3& force)
{
    let e = get_internal();

    if (!e->physics_rigidbody)
        return;

    physics_add_torque(e->world->physics_world, e->physics_rigidbody, pivot, point, force);
}

RenderWorldObjectHandle Entity::get_render_object() const
{
    return get_internal()->render_object;
}

PhysicsObjectHandle Entity::get_physics_object() const
{
    return get_internal()->physics_object;
}