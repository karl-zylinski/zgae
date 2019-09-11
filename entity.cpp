#include "entity.h"
#include "log.h"
#include "world.h"
#include "physics.h"
#include "renderer.h"

Entity entity_create(
    World* w,
    const Vec3& pos,
    const Quat& rot)
{
    return { 
        .world = w,
        .idx = w->create_entity(pos, rot)
    };
}

#define get_internal() (this->world->entities + this->idx)

void Entity::move(const Vec3& d)
{
    let e = get_internal();
    e->pos += d;

    if (e->physics_object_idx)
        physics_set_position(e->world->physics_world, e->physics_object_idx, e->pos, e->rot);

    if (e->render_object_idx)
        renderer_world_set_position_and_rotation(e->world->render_world, e->render_object_idx, e->pos, e->rot);
}

void Entity::rotate(const Vec3& axis, f32 rad)
{
    let e = get_internal();
    let r = quat_from_axis_angle(axis, rad);
    e->rot *= r;

    if (e->physics_object_idx)
        physics_set_position(e->world->physics_world, e->physics_object_idx, e->pos, e->rot);

    if (e->render_object_idx)
        renderer_world_set_position_and_rotation(e->world->render_world, e->render_object_idx, e->pos, e->rot);
}

void Entity::rotate(const Quat& q)
{
    let e = get_internal();
    e->rot *= q;

    if (e->physics_object_idx)
        physics_set_position(e->world->physics_world, e->physics_object_idx, e->pos, e->rot);

    if (e->render_object_idx)
        renderer_world_set_position_and_rotation(e->world->render_world, e->render_object_idx, e->pos, e->rot);
}

void Entity::create_rigidbody(f32 mass, const Vec3& velocity)
{
    let e = get_internal();
    check(e->physics_object_idx, "Trying to create rigidbody on entity with no physics representation.");
    check(!e->physics_rigidbody_idx, "Trying to add rigidbody to entity twice");
    e->physics_rigidbody_idx = physics_create_rigidbody(e->world->physics_world, e->physics_object_idx, mass, velocity);
}

const Vec3& Entity::get_position() const
{
    return get_internal()->pos;
}

void Entity::set_render_mesh(u32 mesh_idx)
{
    let e = get_internal();
    e->render_object_idx = renderer_create_object(e->world->render_world, mesh_idx, e->pos, e->rot);
}

void Entity::set_physics_collider(const PhysicsCollider& collider, const PhysicsMaterial& pm)
{
    let e = get_internal();
    e->physics_object_idx = physics_create_object(e->world->physics_world, collider, e->render_object_idx, e->pos, e->rot, pm);
}

void Entity::set_position(const Vec3& pos)
{
    let e = get_internal();
    e->pos = pos;

    if (e->physics_object_idx)
        physics_set_position(e->world->physics_world, e->physics_object_idx, e->pos, e->rot);

    if (e->render_object_idx)
        renderer_world_set_position_and_rotation(e->world->render_world, e->render_object_idx, e->pos, e->rot);
}

void Entity::set_rotation(const Quat& rot)
{
    let e = get_internal();
    e->rot = rot;

    if (e->physics_object_idx)
        physics_set_position(e->world->physics_world, e->physics_object_idx, e->pos, e->rot);
    
    if (e->render_object_idx)
        renderer_world_set_position_and_rotation(e->world->render_world, e->render_object_idx, e->pos, e->rot);
}

void Entity::set_velocity(const Vec3& vel)
{
    let e = get_internal();

    if (!e->physics_rigidbody_idx)
    {
        info("Using set_velocity: Entity has no rigidbody");
        return;
    }

    physics_set_velocity(e->world->physics_world, e->physics_rigidbody_idx, vel);
}

void Entity::add_force(const Vec3& f)
{
    let e = get_internal();

    if (!e->physics_rigidbody_idx)
    {
        info("Trying to run add_force on Entity that has no rigidbody");
        return;
    }

    physics_add_force(e->world->physics_world, e->physics_rigidbody_idx, f);
}

void Entity::add_torque(const Vec3& pivot, const Vec3& point, const Vec3& force)
{
    let e = get_internal();

    if (!e->physics_rigidbody_idx)
        return;

    physics_add_torque(e->world->physics_world, e->physics_rigidbody_idx, pivot, point, force);
}

u32 Entity::get_render_object_idx() const
{
    return get_internal()->render_object_idx;
}

u32 Entity::get_physics_object_idx() const
{
    return get_internal()->physics_object_idx;
}