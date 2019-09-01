#pragma once
#include "entity_types.h"

fwd_handle(PhysicsResourceHandle);
fwd_handle(RenderResourceHandle);

Entity entity_create(
    const Vec3& pos,
    const Quat& rot, 
    RenderResourceHandle render_world,
    RenderResourceHandle mesh,
    PhysicsResourceHandle physics_world,
    PhysicsResourceHandle collider);

void entity_move(Entity* e, const Vec3& d);