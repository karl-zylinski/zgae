#include "entity.h"
#include "render_object.h"
#include "physics.h"
#include "array.h"

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

EntityHandle entity_create(const Vec3& position, const Quat& rotation, RRHandle geometry)
{
    Entity e = {};
    e.position = position;
    e.rotation = rotation;

    if (geometry.h)
        e.render_object = render_object_create(geometry, position, rotation);

    for (size_t i = 0; i < array_size(D_entities); ++i)
    {
        if (D_entities[i].free)
        {
            D_entities[i].e = e;
            D_entities[i].free = false;
            return EntityHandle{i};
        }
    }

    size_t idx = array_size(D_entities);
    EntityResource er = {};
    er.e = e;
    array_push(D_entities, er);
    return EntityHandle{idx};
}

void entity_destroy(EntityHandle eh)
{
    size_t h = eh.h;
    if (!h)
        return;

    if (D_entities[h].e.render_object.h)
        render_object_destroy(D_entities[h].e.render_object);

    D_entities[h].free = true;
}

void entity_set_position(EntityHandle eh, const Vec3& pos)
{
    size_t h = eh.h;
    if (!h || D_entities[h].free)
        return;

    Entity* e = &D_entities[h].e;
    e->position = pos;

    if (e->collider.h)
    {
        physics_set_collider_position(e->collider, e->position);
    }

    if (e->render_object.h)
        render_object_set_position_and_rotation(e->render_object, e->position, e->rotation);
}

void entity_set_rotation(EntityHandle eh, const Quat& rot)
{
    size_t h = eh.h;

    if (!h || D_entities[h].free)
        return;

    Entity* e = &D_entities[h].e;
    e->rotation = rot;

    if (e->collider.h)
    {
        physics_set_collider_rotation(e->collider, e->rotation);
    }

    if (e->render_object.h)
        render_object_set_position_and_rotation(e->render_object, e->position, e->rotation);
}

const Vec3& entity_get_position(EntityHandle eh)
{
    size_t h = eh.h;

    if (!h || D_entities[h].free)
        return vec3_zero;

    return D_entities[h].e.position;
}

const Quat& entity_get_rotation(EntityHandle eh)
{
    size_t h = eh.h;

    if (!h || D_entities[h].free)
        return quat_identity;

    return D_entities[h].e.rotation;
}

void entity_set_collider(EntityHandle eh, ColliderHandle ch)
{
    size_t h = eh.h;

    if (!h || D_entities[h].free)
        return;

    Entity* e = &D_entities[h].e;
    Assert(e->collider.h == 0, "PLZ IMPLEMENT");
    e->collider = ch;
    physics_set_collider_position(ch, e->position);
    physics_set_collider_rotation(ch, e->rotation);
}

RenderObjectHandle entity_get_render_object(EntityHandle e)
{
    return D_entities[e.h].e.render_object;
}

ColliderHandle entity_get_collider(EntityHandle e)
{
    return D_entities[e.h].e.collider;
}

bool entity_intersects(EntityHandle eh1, EntityHandle eh2)
{
    if (!eh1.h || D_entities[eh1.h].free || !eh2.h || D_entities[eh2.h].free)
        return false;


    Entity* e1 = &D_entities[eh1.h].e;
    Entity* e2 = &D_entities[eh2.h].e;

    if (!e1->collider.h || !e2->collider.h)
        return false;

    return physics_intersects(e1->collider, e2->collider);
}

void entity_shutdown()
{
    array_destroy(D_entities);
}