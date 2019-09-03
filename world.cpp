#include "world.h"
#include "handle_pool.h"
#include "handle.h"
#include "memory.h"
#include "entity_types.h"

static const u32 handle_type = 1;

World* create_world(RenderResourceHandle render_world, PhysicsResourceHandle physics_world)
{
    let hp = handle_pool_create(2, "WorldEntityHandle");
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
    memzero(this->entities + handle_index(weh), sizeof(EntityInt));
    handle_pool_return(this->entity_handle_pool, weh);
}

WorldEntityHandle World::create_entity(const Vec3& pos, const Quat& rot)
{
    WorldEntityHandle h = handle_pool_borrow(this->entity_handle_pool, handle_type);

    u32 num_needed_entities = handle_index(h) + 1;
    if (num_needed_entities > this->entities_num)
    {
        this->entities = (EntityInt*)memra_zero_added(this->entities, num_needed_entities * sizeof(EntityInt), this->entities_num * sizeof(EntityInt));
        this->entities_num = num_needed_entities;
    }
    let e = this->entities + handle_index(h);
    memzero(e, sizeof(EntityInt));
    e->pos = pos;
    e->rot = rot;
    e->world = this;
    e->handle = h;
    return h;
}

EntityInt* World::lookup_entity(WorldEntityHandle e)
{
    return this->entities + handle_index(e);
}