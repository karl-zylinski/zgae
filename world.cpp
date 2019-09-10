#include "world.h"
#include "handle_pool.h"
#include "handle.h"
#include "memory.h"
#include "entity.h"
#include "log.h"
#include "dynamic_array.h"

static const u32 handle_type = 1;

World* create_world(RenderWorld* render_world, PhysicsWorld* physics_world)
{
    let hp = handle_pool_create(HANDLE_POOL_TYPE_WORLD_ENTITY);
    handle_pool_set_type(hp, handle_type, "Entity");
    let w = mema_zero_t(World);
    w->render_world = render_world;
    w->physics_world = physics_world;
    w->entity_handle_pool = hp;
    return w;
}

void destroy_world(World* w)
{
    for (u32 i = 0; i < da_num(w->entities); ++i)
    {
        let e = w->entities + i;

        if (!e->handle)
            continue;

        w->destroy_entity(e->handle);
    }

    handle_pool_destroy(w->entity_handle_pool);
    da_free(w->entities);
    memf(w);
}

void World::destroy_entity(WorldEntityHandle weh)
{
    check(handle_pool_is_valid(this->entity_handle_pool, weh), "Invalid Handle when trying to destroy entity");
    memzero(this->entities + handle_index(weh), sizeof(EntityInt));
    handle_pool_return(this->entity_handle_pool, weh);
}

WorldEntityHandle World::create_entity(const Vec3& pos, const Quat& rot)
{
    let handle = handle_pool_borrow(this->entity_handle_pool, handle_type);
    let idx = handle_index(handle);

    EntityInt ei = {
        .pos = pos,
        .rot = rot,
        .world = this,
        .handle = handle
    };

    if (idx < da_num(this->entities))
    {
        this->entities[idx] = ei;
        return handle;
    }

    check(idx == da_num(this->entities), "Mismatch between entity index and num entities");
    da_push(this->entities, ei);
    return handle;
}

EntityInt* World::lookup_entity(WorldEntityHandle e)
{
    check_slow(handle_pool_is_valid(this->entity_handle_pool, e), "Invalid Handle when trying to lookup entity");
    return this->entities + handle_index(e);
}

void World::update()
{
    physics_update_world(this->physics_world);

    for (u32 i = 0; i < da_num(this->entities); ++i)
    {
        let e = this->entities + i;
        if (e->physics_rigidbody)
        {
            let pos = physics_get_position(e->world->physics_world, e->physics_object);
            let rot = physics_get_rotation(e->world->physics_world, e->physics_object);

            e->pos = pos;
            e->rot = rot;

            if (e->render_object_idx)
                renderer_world_set_position_and_rotation(e->world->render_world, e->render_object_idx, e->pos, e->rot);
        }
    }
}