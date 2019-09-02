#pragma once
#include "entity_types.h"


EntityRef entity_create(
    World* w,
    const Vec3& pos,
    const Quat& rot);

void entity_move(EntityRef* er, const Vec3& d);
void entity_create_rigidbody(EntityRef* er, f32 mass);
void entity_set_render_mesh(EntityRef* er, RenderResourceHandle mesh);
void entity_set_physics_collider(EntityRef* er, PhysicsResourceHandle collider);
Vec3 entity_get_position(const EntityRef& er);
void entity_set_position(EntityRef* er, const Vec3& pos);
void entity_set_rotation(EntityRef* er, const Quat& rot);
void entity_add_force(EntityRef* er, const Vec3& f);
Entity* entity_deref(EntityRef* er);