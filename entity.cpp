#include "entity.h"
#include "render_object.h"
#include "physics.h"
#include "array.h"
#include "render_world.h"

struct Entity
{
    Vec3 position;
    Quat rotation;
    ColliderHandle collider;
    RenderObjectHandle render_object;
};

struct EntityResource
{
    Entity e;
    bool free;
};

static EntityResource* D_entities = nullptr;

void entity_init()
{
    array_push(D_entities, {});
}

EntityHandle entity_create(RenderWorld* rw, const Vec3& position, const Quat& rotation, RRHandle geometry)
{
    Entity e = {};
    e.position = position;
    e.rotation = rotation;

    if (geometry.h && rw)
    {
        e.render_object = render_object_create(geometry, position, rotation);
        render_world_add(rw, e.render_object);
    }

    for (size_t i = 0; i < array_size(D_entities); ++i)
    {
        if (D_entities[i].free)
        {
            D_entities[i].e = e;
            D_entities[i].free = false;
            return i;
        }
    }

    size_t idx = array_size(D_entities);
    EntityResource er = {};
    er.e = e;
    array_push(D_entities, er);
    return idx;
}

void entity_destroy(EntityHandle h)
{
    if (!h)
        return;

    Error("Implement removal from render world");
    D_entities[h].free = true;
}

void entity_set_position(EntityHandle h, const Vec3& pos)
{
    if (!h || D_entities[h].free)
        return;

    Entity* e = &D_entities[h].e;
    e->position = pos;

    if (e->render_object.h)
        render_object_set_position_and_rotation(e->render_object, e->position, e->rotation);
}

void entity_set_rotation(EntityHandle h, const Quat& rot)
{
    if (!h || D_entities[h].free)
        return;

    Entity* e = &D_entities[h].e;
    e->rotation = rot;

    if (e->render_object.h)
        render_object_set_position_and_rotation(e->render_object, e->position, e->rotation);
}

const Vec3& entity_get_position(EntityHandle h)
{
    if (!h || D_entities[h].free)
        return vec3_zero;

    return D_entities[h].e.position;
}

const Quat& entity_get_rotation(EntityHandle h)
{
    if (!h || D_entities[h].free)
        return quat_identity;

    return D_entities[h].e.rotation;
}

void entity_set_collider(EntityHandle h, ColliderHandle ch)
{
    if (!h || D_entities[h].free)
        return;

    Entity* e = &D_entities[h].e;
    Assert(e->collider.h == 0, "PLZ IMPLEMENT");
    e->collider = ch;
    physics_set_collider_position(ch, e->position);
    physics_set_collider_rotation(ch, e->rotation);
}

bool entity_intersects(EntityHandle eh1, EntityHandle eh2)
{
    if (!eh1 || D_entities[eh1].free || !eh2 || D_entities[eh2].free)
        return false;


    Entity* e1 = &D_entities[eh1].e;
    Entity* e2 = &D_entities[eh2].e;

    if (!e1->collider.h || !e2->collider.h)
        return false;

    return physics_intersects(e1->collider, e2->collider);
}

void entity_shutdown()
{
    array_destroy(D_entities);
}