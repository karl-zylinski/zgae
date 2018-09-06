#pragma once
#include "render_resource.h"
#include "render_object.h"
#include "entity_handle.h"
#include "collider_handle.h"

struct RenderWorld;
struct Vec3;
struct Quat;

EntityHandle entity_create(const Vec3& position, const Quat& rotation, RRHandle geometry);
void entity_destroy(EntityHandle h);
void entity_set_position(EntityHandle h, const Vec3& pos);
void entity_set_rotation(EntityHandle h, const Quat& rot);
const Vec3& entity_get_position(EntityHandle h);
const Quat& entity_get_rotation(EntityHandle h);
void entity_set_collider(EntityHandle eh, ColliderHandle ch);
bool entity_intersects(EntityHandle eh1, EntityHandle eh2);
RenderObjectHandle entity_get_render_object(EntityHandle e);
ColliderHandle entity_get_collider(EntityHandle e);
void entity_init();
void entity_shutdown();