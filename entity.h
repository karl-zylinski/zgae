#pragma once
#include "entity_types.h"

fwd_struct(World);

EntityRef entity_create(
    World* w,
    const Vec3& pos,
    const Quat& rot);

void entity_move(EntityRef* er, const Vec3& d);
void entity_create_rigidbody(EntityRef* er);
void entity_set_render_mesh(EntityRef* er, RenderResourceHandle mesh);
void entity_set_physics_collider(EntityRef* er, PhysicsResourceHandle collider);
Vec3 entity_get_position(const EntityRef& er);