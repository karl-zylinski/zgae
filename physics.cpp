#include "physics.h"
#include "memory.h"
#include "handle_hash_map.h"
#include "handle_pool.h"
#include "str.h"
#include "debug.h"
#include "handle.h"
#include "path.h"
#include "math.h"
#include "dynamic_array.h"
#include "time.h"
#include "gjk_epa.h"
#include "file.h"
#include "jzon.h"
#include "obj_loader.h"
#include "renderer.h"
#include "entity_types.h"

struct PhysicsResource
{
    hash64 name_hash;
    PhysicsResourceHandle handle; // type is in the handle, get with handle_type(handle)
    void* data;
};

struct PhysicsState
{
    PhysicsResource* resources;
    u32 resources_num;
    HandleHashMap* resource_name_to_handle;
    HandlePool* resource_handle_pool;
};

struct PhysicsResourceMesh
{
    Vec3* vertices;
    u32 vertices_num;
};

struct PhysicsResourceCollider
{
    PhysicsResourceHandle mesh;
};

struct PhysicsWorldObject
{
    bool used;
    PhysicsResourceHandle collider;
    RenderWorldObjectHandle render_handle;
    Vec3 position;
    Quat rotation;
};

struct Rigidbody
{
    bool used;
    PhysicsWorldObjectHandle object;
    Vec3 velocity;
    Entity* entity;
};

struct PhysicsResourceWorld
{
    PhysicsWorldObject* objects; // dynamic
    u32* free_object_indices; // dynamic, holes in objects
    Rigidbody* rigidbodies; // dynamic
    u32* free_rigidbody_indices; // dynamic, holes in rigidbodies
    RenderResourceHandle render_handle;
};

static PhysicsState* ps = NULL;

static const char* physics_resoruce_type_names[] =
{
    "invalid", "mesh", "collider", "world"
};

#define get_resource(t, h) ((t*)(ps->resources[handle_index(h)]).data)

void physics_init()
{
    check(ps == NULL, "Trying to init physics twice");
    ps = mema_zero_t(PhysicsState);

    ps->resource_handle_pool = handle_pool_create(1, "PhysicsResourceHandle");
    ps->resource_name_to_handle = handle_hash_map_create();

    for (u32 s = 1; s < PHYSICS_RESOURCE_TYPE_NUM; ++s)
        handle_pool_set_type(ps->resource_handle_pool, s, physics_resoruce_type_names[s]);
}

static PhysicsResourceType resource_type_from_str(const char* str)
{
    i32 idx = str_eql_arr(str, physics_resoruce_type_names, sizeof(physics_resoruce_type_names)/sizeof(physics_resoruce_type_names[0]));
    check(idx > 0 && idx < PHYSICS_RESOURCE_TYPE_NUM, "Invalid physics resource type");
    return (PhysicsResourceType)idx;
}

static PhysicsResourceHandle add_resource(hash64 name_hash, PhysicsResourceType type, void* data)
{
    let handle = handle_pool_borrow(ps->resource_handle_pool, (u32)type);
    u32 num_needed_resources = handle_index(handle) + 1;
    if (num_needed_resources > ps->resources_num)
    {
        ps->resources = (PhysicsResource*)memra_zero_added(ps->resources, num_needed_resources * sizeof(PhysicsResource), ps->resources_num * sizeof(PhysicsResource));
        ps->resources_num = num_needed_resources;
    }
    PhysicsResource* r = ps->resources + handle_index(handle);
    memzero_p(r);
    r->name_hash = name_hash;
    r->handle = handle;
    r->data = data;
    return handle;
}

PhysicsResourceHandle physics_resource_load(const char* filename)
{
    let name_hash = str_hash(filename);
    let existing = handle_hash_map_get(ps->resource_name_to_handle, name_hash);

    if (existing != HANDLE_INVALID)
        return existing;

    const char* ext = path_ext(filename);
    let type = resource_type_from_str(ext);
    void* data = NULL;

    switch(type)
    {
        case PHYSICS_RESOURCE_TYPE_MESH: {
            FileLoadResult flr = file_load(filename, FILE_LOAD_MODE_NULL_TERMINATED);
            check(flr.ok, "Failed loading mesh from %s", filename);
            JzonParseResult jpr = jzon_parse((char*)flr.data);
            check(jpr.ok && jpr.output.is_table, "Outer object in %s isn't a table", filename);
            memf(flr.data);

            let jz_source = jzon_get(jpr.output, "source");
            check(jz_source && jz_source->is_string, "%s doesn't contain source field", filename);

            let obj_vertices = obj_load_vertices(jz_source->string_val);
            check(obj_vertices.ok, "Failed loading obj specified by %s in %s", jz_source->string_val, filename);
            jzon_free(&jpr.output);

            check(obj_vertices.ok, "Failed loading mesh from file %s", filename);

            let m = mema_zero_t(PhysicsResourceMesh);
            m->vertices = obj_vertices.vertices;
            m->vertices_num = obj_vertices.vertices_num;
            data = m;
        } break;

        default: error("Implement me!");
    }

    return add_resource(name_hash, type, data);
}

PhysicsWorldRigidbodyHandle physics_add_rigidbody(Entity* e)
{
    let w = get_resource(PhysicsResourceWorld, e->physics_world);

    if (da_num(w->free_rigidbody_indices) > 0)
    {
        u32 idx = da_pop(w->free_rigidbody_indices);
        w->rigidbodies[idx].object = e->physics_object;
        w->rigidbodies[idx].velocity = vec3_zero;
        w->rigidbodies[idx].entity = e;
        w->rigidbodies[idx].used = true;
        return idx;
    }

    let idx = da_num(w->rigidbodies);

    Rigidbody rb = {
        .object = e->physics_object,
        .entity = e,
        .used = true
    };

    da_push(w->rigidbodies, rb);
    return idx;
}

PhysicsResourceHandle physics_collider_create(PhysicsResourceHandle mesh)
{
    let c = mema_zero_t(PhysicsResourceCollider);
    c->mesh = mesh;
    return add_resource(0, PHYSICS_RESOURCE_TYPE_COLLIDER, c);
}

PhysicsResourceHandle physics_world_create(RenderResourceHandle render_handle)
{
    let w = mema_zero_t(PhysicsResourceWorld);
    w->render_handle = render_handle;
    return add_resource(0, PHYSICS_RESOURCE_TYPE_WORLD, w);
}

PhysicsWorldObjectHandle physics_world_add(PhysicsResourceHandle world, PhysicsResourceHandle collider, RenderWorldObjectHandle render_handle, const Vec3& pos, const Quat& rot)
{
    let w = get_resource(PhysicsResourceWorld, world);

   if (da_num(w->free_object_indices) > 0)
    {
        u32 idx = da_pop(w->free_object_indices);
        w->objects[idx].collider = collider;
        w->objects[idx].position = pos;
        w->objects[idx].rotation = rot;
        w->objects[idx].used = true;
        return idx;
    }

    let h = da_num(w->objects);

    PhysicsWorldObject wo = {
        .collider = collider,
        .position = pos,
        .rotation = rot,
        .render_handle = render_handle,
        .used = true
    };

    da_push(w->objects, wo);
    return h;
}

void physics_world_move(PhysicsResourceHandle world, PhysicsWorldObjectHandle obj, const Vec3& pos)
{
    let w = get_resource(PhysicsResourceWorld, world);
    w->objects[obj].position += pos;
    renderer_world_set_position_and_rotation(w->render_handle, w->objects[obj].render_handle, w->objects[obj].position, w->objects[obj].rotation);
}

void physics_world_set_position(PhysicsResourceHandle world, PhysicsWorldObjectHandle obj, const Vec3& pos, const Quat& rot)
{
    let w = get_resource(PhysicsResourceWorld, world);
    w->objects[obj].position = pos;
    w->objects[obj].rotation = rot;
    renderer_world_set_position_and_rotation(w->render_handle, w->objects[obj].render_handle, pos, rot);
}

void physics_update_world(PhysicsResourceHandle world)
{
    let w = get_resource(PhysicsResourceWorld, world);
    float dt = time_dt();

    for (u32 rigidbody_idx = 0; rigidbody_idx < da_num(w->rigidbodies); ++rigidbody_idx)
    {
        let rb = w->rigidbodies + rigidbody_idx;
        if (!rb->used)
            continue;

        Vec3 g = {0, 0, -1.82f*dt};
        rb->velocity += g;
        let rb_world_object_index = rb->object;
        let wo = w->objects + rb_world_object_index;
        physics_world_move(world, rb_world_object_index, rb->velocity);
        w->rigidbodies[rigidbody_idx].entity->position = w->objects[rb_world_object_index].position;
        let c1 = get_resource(PhysicsResourceCollider, wo->collider);
        let p1 = wo->position;
        //let r1 = w->objects[rigidbody_idx].rotation;
        let m1 = get_resource(PhysicsResourceMesh, c1->mesh);

        GjkShape s1 = {
            .vertices = mema_tn(Vec3, m1->vertices_num),
            .vertices_num = m1->vertices_num
        };

        for (u32 vi = 0; vi < s1.vertices_num; ++vi)
            s1.vertices[vi] = m1->vertices[vi] + p1;

        for (u32 world_object_index = 0; world_object_index < da_num(w->objects); ++world_object_index)
        {
            if (world_object_index == rb_world_object_index || !w->objects[world_object_index].used)
                continue;

            let c2 = get_resource(PhysicsResourceCollider, w->objects[world_object_index].collider);
            let p2 = w->objects[world_object_index].position;
            //let r2 = w->objects[world_object_index].rotation;
            let m2 = get_resource(PhysicsResourceMesh, c2->mesh);

            GjkShape s2 = {
                .vertices = mema_tn(Vec3, m2->vertices_num),
                .vertices_num = m2->vertices_num
            };

            for (u32 vi = 0; vi < s2.vertices_num; ++vi)
                s2.vertices[vi] = m2->vertices[vi] + p2;

            let coll = gjk_epa_intersect_and_solve(s1, s2);

            if (coll.colliding)
            {
                physics_world_move(world, rb_world_object_index, coll.solution);
                rb->velocity = vec3_zero;
                w->rigidbodies[rigidbody_idx].entity->position = w->objects[rb_world_object_index].position;
                w->rigidbodies[rigidbody_idx].entity->rotation = w->objects[rb_world_object_index].rotation;
            }

            memf(s2.vertices);
        }

        memf(s1.vertices);
    }
}

static void destroy_resource(PhysicsResourceHandle h)
{
    switch(handle_type(h))
    {
        case PHYSICS_RESOURCE_TYPE_MESH: {
            let m = get_resource(PhysicsResourceMesh, h);
            memf(m->vertices);
        } break;

        case PHYSICS_RESOURCE_TYPE_WORLD: {
            let w = get_resource(PhysicsResourceWorld, h);
            da_free(w->objects);
            da_free(w->free_object_indices);
            da_free(w->rigidbodies);
            da_free(w->free_rigidbody_indices);
        } break;
    }

    memf(ps->resources[handle_index(h)].data);
    handle_pool_return(ps->resource_handle_pool, h);

    if (ps->resources[handle_index(h)].name_hash)
        handle_hash_map_remove(ps->resource_name_to_handle, ps->resources[handle_index(h)].name_hash);
}

void physics_shutdown()
{
    for (u32 i = 0; i < ps->resources_num; ++i)
        destroy_resource(ps->resources[i].handle);

    memf(ps->resources);
    handle_hash_map_destroy(ps->resource_name_to_handle);
    handle_pool_destroy(ps->resource_handle_pool);
    memf(ps);
}