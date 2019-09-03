#pragma once

fwd_handle(RenderResourceHandle);
fwd_handle(PhysicsResourceHandle);
fwd_struct(EntityInt);
fwd_struct(HandlePool);

typedef Handle WorldEntityHandle;

struct World
{
    RenderResourceHandle render_world;
    PhysicsResourceHandle physics_world;
    EntityInt* entities;
    u32 entities_num;
    HandlePool* entity_handle_pool;
};
