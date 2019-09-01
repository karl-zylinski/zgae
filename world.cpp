#include "world.h"
#include "handle_pool.h"
#include "handle.h"
#include "memory.h"
#include "entity.h"

static const u32 handle_type = 1;

World* world_create(RenderResourceHandle render_world, PhysicsResourceHandle physics_world)
{
    let hp = handle_pool_create(2, "WorldEntityHandle");
    handle_pool_set_type(hp, handle_type, "Entity");
    let w = mema_zero_t(World);
    w->render_world = render_world;
    w->physics_world = physics_world;
    w->entity_handle_pool = hp;
    return w;
}

WorldEntityHandle world_create_entity(World* w, const Vec3& pos, const Quat& rot)
{
    WorldEntityHandle h = handle_pool_borrow(w->entity_handle_pool, handle_type);

    u32 num_needed_entities = handle_index(h) + 1;
    if (num_needed_entities > w->entities_num)
    {
        w->entities = (Entity*)memra_zero_added(w->entities, num_needed_entities * sizeof(Entity), w->entities_num * sizeof(Entity));
        w->entities_num = num_needed_entities;
    }
    Entity* e = w->entities + handle_index(h);
    e->pos = pos;
    e->rot = rot;
    e->world = w;
    return h;
}

Entity* world_lookup_entity(World* w, WorldEntityHandle e)
{
    return w->entities + handle_index(e);
}