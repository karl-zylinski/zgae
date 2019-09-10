#pragma once

fwd_struct(Vec3);
fwd_struct(Quat);
fwd_handle(RenderResourceHandle);
fwd_handle(PhysicsResourceHandle);
fwd_struct(EntityInt);
fwd_struct(HandlePool);
fwd_struct(PhysicsWorld);
fwd_struct(RenderWorld);
fwd_handle(WorldEntityHandle);

struct World
{
    void destroy_entity(WorldEntityHandle weh);
    WorldEntityHandle create_entity(const Vec3& pos, const Quat& rot);
    EntityInt* lookup_entity(WorldEntityHandle e);
    void update();

    RenderWorld* render_world;
    PhysicsWorld* physics_world;
    EntityInt* entities; // dynamic
    HandlePool* entity_handle_pool;
};

World* create_world(RenderWorld* render_world, PhysicsWorld* physics_world);
void destroy_world(World* w);
