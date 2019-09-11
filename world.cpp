#include "world.h"
#include "memory.h"
#include "entity.h"
#include "dynamic_array.h"

World* create_world(RenderWorld* render_world, PhysicsWorld* physics_world)
{
    let w = mema_zero_t(World);
    w->render_world = render_world;
    w->physics_world = physics_world;
    da_push(w->entities, EntityInt{});
    return w;
}

void destroy_world(World* w)
{
    for (u32 i = 0; i < da_num(w->entities); ++i)
    {
        let e = w->entities + i;

        if (!e->idx)
            continue;

        w->destroy_entity(i);
    }

    da_free(w->entities);
    da_free(w->entities_free_idx);
    memf(w);
}

void World::destroy_entity(u32 entity_idx)
{
    memzero(this->entities + entity_idx, sizeof(EntityInt));
    da_push(this->entities_free_idx, entity_idx);
}

u32 World::create_entity(const Vec3& pos, const Quat& rot)
{
    let idx = da_num(this->entities_free_idx) > 0 ? da_pop(this->entities_free_idx) : da_num(this->entities);

    EntityInt ei = {
        .idx = idx,
        .pos = pos,
        .rot = rot,
        .world = this
    };

    da_insert(this->entities, ei, idx);
    return idx;
}

EntityInt* World::lookup_entity(u32 entity_idx)
{
    return this->entities + entity_idx;
}

void World::update()
{
    physics_update_world(this->physics_world);

    for (u32 i = 0; i < da_num(this->entities); ++i)
    {
        let e = this->entities + i;
        if (e->physics_rigidbody_idx)
        {
            let pos = physics_get_position(e->world->physics_world, e->physics_object_idx);
            let rot = physics_get_rotation(e->world->physics_world, e->physics_object_idx);

            e->pos = pos;
            e->rot = rot;

            if (e->render_object_idx)
                renderer_world_set_position_and_rotation(e->world->render_world, e->render_object_idx, e->pos, e->rot);
        }
    }
}