#pragma once

fwd_struct(Vec3);
fwd_struct(Quat);
fwd_struct(EntityInt);
fwd_struct(PhysicsWorld);
fwd_struct(RenderWorld);

struct World
{
    void destroy_entity(u32 entity_idx);
    u32 create_entity(const Vec3& pos, const Quat& rot);
    EntityInt* lookup_entity(u32 entity_idx);
    void update();

    RenderWorld* render_world;
    PhysicsWorld* physics_world;
    EntityInt* entities; // dynamic
    u32* entities_free_idx; // dynamic
};

World* create_world(RenderWorld* render_world, PhysicsWorld* physics_world);
void destroy_world(World* w);
