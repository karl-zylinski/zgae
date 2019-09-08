#include "world.h"
#include "handle_pool.h"
#include "handle.h"
#include "memory.h"
#include "entity.h"
#include "log.h"

static const u32 handle_type = 1;

World* create_world(RenderResourceHandle render_world, PhysicsResourceHandle physics_world)
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
    for (u32 i = 0; i < w->entities_num; ++i)
    {
        let e = w->entities + i;

        if (!e->handle)
            continue;

        w->destroy_entity(e->handle);
    }

    handle_pool_destroy(w->entity_handle_pool);
    memf(w->entities);
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
    WorldEntityHandle h = handle_pool_borrow(this->entity_handle_pool, handle_type);
    u32 num_needed_entities = handle_index(h) + 1;
    if (num_needed_entities > this->entities_num)
    {
        this->entities = (EntityInt*)memra(this->entities, num_needed_entities * sizeof(EntityInt));
        this->entities_num = num_needed_entities;
    }
    this->entities[handle_index(h)] = {
        .pos = pos,
        .rot = rot,
        .world = this,
        .handle = h
    };
    return h;
}

EntityInt* World::lookup_entity(WorldEntityHandle e)
{
    check_slow(handle_pool_is_valid(this->entity_handle_pool, e), "Invalid Handle when trying to lookup entity");
    return this->entities + handle_index(e);
}