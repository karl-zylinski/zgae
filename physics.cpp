#include "physics.h"
#include "memory.h"
#include "handle_hash_map.h"
#include "handle_pool.h"
#include "str.h"
#include "log.h"
#include "handle.h"
#include "path.h"
#include "time.h"
#include "gjk_epa.h"
#include "file.h"
#include "jzon.h"
#include "obj_loader.h"
#include "world.h"
#include "renderer.h"
#include "render_resource.h"
#include "debug.h"
#include "camera.h"
#include <string.h>
#include <math.h>
#include "dynamic_array.h"

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
    PhysicsMaterial material;
    Vec3 pos;
    Quat rot;
};

struct RecentCollision
{
    f32 time;
    u32 object_idx;
    Vec3 contact_point;
    Vec3 solution;
    Vec3 rigidbody_pos;
};

struct Rigidbody
{
    bool used;
    PhysicsObjectHandle object_handle;
    PhysicsRigidbodyHandle handle;
    Vec3 velocity;
    Vec3 angular_velocity;
    f32 mass;
};

struct PhysicsResourceWorld
{
    HandlePool* object_handle_pool;
    HandlePool* rigidbody_handle_pool;
    PhysicsWorldObject* objects;
    Rigidbody* rigidbodies; // dynamic
    u32 objects_num;
    RenderResourceHandle render_handle;
};

static PhysicsState ps = {};
static bool inited = false;

static const char* physics_resoruce_type_names[] =
{
    "invalid", "mesh", "collider", "world"
};

static u32 handle_index_checked(const HandlePool* hp, Handle h)
{
    check(handle_pool_is_valid(hp, h), "Invalid Handle passed");
    return handle_index(h);
}

#define get_resource(t, h) ((t*)ps.resources[handle_index_checked(ps.resource_handle_pool,h)].data)
#define get_object(w, h) ((PhysicsWorldObject*)(w->objects + handle_index_checked(w->object_handle_pool, h)))
#define get_rigidbody(w, h) ((Rigidbody*)(w->rigidbodies + handle_index_checked(w->rigidbody_handle_pool, h)))

void physics_init()
{
    check(!inited, "Trying to init physics twice");
    inited = true;

    ps.resource_handle_pool = handle_pool_create(HANDLE_POOL_TYPE_PHYSICS_RESOURCE);
    ps.resource_name_to_handle = handle_hash_map_create();

    for (u32 s = 1; s < PHYSICS_RESOURCE_TYPE_NUM; ++s)
        handle_pool_set_type(ps.resource_handle_pool, s, physics_resoruce_type_names[s]);
}

static PhysicsResourceType resource_type_from_str(const char* str)
{
    i32 idx = str_eql_arr(str, physics_resoruce_type_names, sizeof(physics_resoruce_type_names)/sizeof(physics_resoruce_type_names[0]));
    check(idx > 0 && idx < PHYSICS_RESOURCE_TYPE_NUM, "Invalid physics resource type");
    return (PhysicsResourceType)idx;
}

static PhysicsResourceHandle add_resource(hash64 name_hash, PhysicsResourceType type, void* data)
{
    let handle = handle_pool_borrow(ps.resource_handle_pool, (u32)type);
    u32 num_needed_resources = handle_index(handle) + 1;
    if (num_needed_resources > ps.resources_num)
    {
        let old_num = ps.resources_num;
        let new_num = num_needed_resources ? num_needed_resources * 2 : 1;
        ps.resources = (PhysicsResource*)memra_zero_added(ps.resources, new_num * sizeof(PhysicsResource), old_num * sizeof(PhysicsResource));
        ps.resources_num = new_num;
    }
    PhysicsResource* r = ps.resources + handle_index(handle);
    memzero_p(r);
    r->name_hash = name_hash;
    r->handle = handle;
    r->data = data;
    return handle;
}

PhysicsResourceHandle physics_load_resource(const char* filename)
{
    let name_hash = str_hash(filename);
    let existing = handle_hash_map_get(ps.resource_name_to_handle, name_hash);

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

PhysicsRigidbodyHandle physics_create_rigidbody(PhysicsResourceHandle world, PhysicsObjectHandle object_handle,  f32 mass, const Vec3& velocity)
{
    check(mass > 0, "Mass must be in range (0, inf)");
    let w = get_resource(PhysicsResourceWorld, world);
    let handle = handle_pool_borrow(w->rigidbody_handle_pool);
    let idx = handle_index(handle);

    Rigidbody r = {
        .mass = mass,
        .object_handle = object_handle,
        .used = true,
        .handle = handle,
        .velocity = velocity
    };

    if (idx < da_num(w->rigidbodies))
    {
        w->rigidbodies[idx] = r;
        return handle;
    }

    check(idx == da_num(w->rigidbodies), "Mismatch between handle index and num rigidbodies");
    da_push(w->rigidbodies, r);
    return handle;
}

void physics_set_velocity(PhysicsResourceHandle world, PhysicsRigidbodyHandle rigidbody_handle, const Vec3& vel)
{
    let w = get_resource(PhysicsResourceWorld, world);
    let rb = get_rigidbody(w, rigidbody_handle);
    rb->velocity = vel;
}


void physics_add_linear_impulse(PhysicsResourceHandle world, PhysicsRigidbodyHandle rigidbody_handle, const Vec3& force, f32 time)
{
    let w = get_resource(PhysicsResourceWorld, world);
    let rb = get_rigidbody(w, rigidbody_handle);
    check(rb->mass != 0, "OH NO");
    let acc = (force*time)/rb->mass;
    rb->velocity += acc;
}

void physics_add_torque(PhysicsResourceHandle world, PhysicsRigidbodyHandle rigidbody_handle, const Vec3& pivot, const Vec3& point, const Vec3& force)
{
    let w = get_resource(PhysicsResourceWorld, world);
    let rb = get_rigidbody(w, rigidbody_handle);

    // add moment of inertia

    let arm = point - pivot;
    let larm = len(arm);
    rb->angular_velocity += cross(arm, force) * (1/(larm * larm * rb->mass)) * time_dt() * 100;
}

PhysicsResourceHandle physics_create_collider(PhysicsResourceHandle mesh)
{
    let c = mema_zero_t(PhysicsResourceCollider);
    c->mesh = mesh;
    return add_resource(0, PHYSICS_RESOURCE_TYPE_COLLIDER, c);
}

PhysicsResourceHandle physics_create_world(RenderResourceHandle render_handle)
{
    let w = mema_zero_t(PhysicsResourceWorld);
    w->render_handle = render_handle;
    w->object_handle_pool = handle_pool_create(HANDLE_POOL_TYPE_PHYSICS_OBJECT);
    w->rigidbody_handle_pool = handle_pool_create(HANDLE_POOL_TYPE_RIGIDBODY);
    return add_resource(0, PHYSICS_RESOURCE_TYPE_WORLD, w);
}

PhysicsObjectHandle physics_create_object(PhysicsResourceHandle world, PhysicsResourceHandle collider, RenderWorldObjectHandle render_handle, const Vec3& pos, const Quat& rot, const PhysicsMaterial& pm)
{
    let w = get_resource(PhysicsResourceWorld, world);
    let h = handle_pool_borrow(w->object_handle_pool);  // TODO: This pattern keeps repeating,
    u32 num_needed_objects = handle_index(h) + 1;       // make it general and put in handle_pool.h?
    if (num_needed_objects > w->objects_num)            // Perhaps with macro for type etc
    {
        let old_num = w->objects_num;
        let new_num = num_needed_objects ? num_needed_objects * 2 : 1;
        w->objects = (PhysicsWorldObject*)memra_zero_added(w->objects, new_num * sizeof(PhysicsWorldObject), old_num * sizeof(PhysicsWorldObject));
        w->objects_num = new_num;
    }
    let o = get_object(w, h);
    memzero(o, sizeof(PhysicsWorldObject));
    o->collider = collider;
    o->pos = pos;
    o->rot = rot;
    o->render_handle = render_handle;
    o->used = true;
    o->material = pm;
    return h;
}

void physics_set_position(PhysicsResourceHandle world, PhysicsObjectHandle obj_handle, const Vec3& pos, const Quat& rot)
{
    let w = get_resource(PhysicsResourceWorld, world);
    let o = get_object(w, obj_handle);
    o->pos = pos;
    o->rot = rot;
}

void physics_update_world(PhysicsResourceHandle world)
{
    let w = get_resource(PhysicsResourceWorld, world);
    float dt = time_dt();
    //float t = time_since_start();

    for (u32 rigidbody_idx = 0; rigidbody_idx < da_num(w->rigidbodies); ++rigidbody_idx)
    {
        let rb = w->rigidbodies + rigidbody_idx;
        if (!rb->used)
            continue;

        // Update rigidbody state.
        Vec3 g = {0, 0, -9.82f};

        physics_add_linear_impulse(world, rb->handle, g * rb->mass, dt);
        let wo = get_object(w, rb->object_handle);

        // Move rigidbody according to velocties
        wo->pos += rb->velocity * dt;
        wo->rot *= quat_from_axis_angle(rb->angular_velocity, dt);


        for (u32 world_object_index = 0; world_object_index < w->objects_num; ++world_object_index)
        {
            if (world_object_index == handle_index(rb->object_handle) || !w->objects[world_object_index].used)
                continue;

            // TODO: cache the shapes etc

            let c1 = get_resource(PhysicsResourceCollider, wo->collider);
            let p1 = wo->pos;
            let r1 = wo->rot;
            let m1 = get_resource(PhysicsResourceMesh, c1->mesh);

            GjkShape s1 = {
                .vertices = mema_tn(Vec3, m1->vertices_num),
                .vertices_num = m1->vertices_num
            };

            for (u32 vi = 0; vi < s1.vertices_num; ++vi)
                s1.vertices[vi] = rotate_vec3(r1, m1->vertices[vi]) + p1;

            let c = debug_get_camera();
            renderer_debug_draw(s1.vertices, s1.vertices_num, NULL, PRIMITIVE_TOPOLOGY_LINE_STRIP, c.pos, c.rot);

            let c2 = get_resource(PhysicsResourceCollider, w->objects[world_object_index].collider);
            let p2 = w->objects[world_object_index].pos;
            let r2 = w->objects[world_object_index].rot;
            let m2 = get_resource(PhysicsResourceMesh, c2->mesh);

            GjkShape s2 = {
                .vertices = mema_tn(Vec3, m2->vertices_num),
                .vertices_num = m2->vertices_num
            };

            for (u32 vi = 0; vi < s2.vertices_num; ++vi)
                s2.vertices[vi] = rotate_vec3(r2, m2->vertices[vi]) + p2;

            renderer_debug_draw(s2.vertices, s2.vertices_num, NULL, PRIMITIVE_TOPOLOGY_LINE_STRIP, c.pos, c.rot);


            let coll = gjk_epa_intersect_and_solve(s1, s2);

            if (coll.colliding)
            {
                wo->pos += coll.solution;

                // The first part here is just getting out of collision and using friction etc.
                #define SOLUTION_THRES 0.0001f
                if (dot(rb->velocity, coll.solution) < 0 && len(coll.solution) > SOLUTION_THRES)
                {
                    Vec3 vel_in_sol_dir = project(rb->velocity, coll.solution);
                    let vel_in_surface_dir = rb->velocity - vel_in_sol_dir;
                    rb->velocity += -vel_in_sol_dir * (1 + wo->material.elasticity); // cancel velocty + bounce
                    let friction = fmin(wo->material.friction + w->objects[world_object_index].material.friction, 1);
                    rb->velocity -= vel_in_surface_dir * friction * dt; // friction
                    rb->angular_velocity -= rb->angular_velocity * friction * dt; // angular friction
                }

                let cp = coll.contact_point + coll.solution;
                let p = wo->pos;

                // Rotation

                let arm = p - cp;
                let larm = len(arm);
                let torque = cross(arm, rb->mass*g);
                info("%f %f %f", torque.x, torque.y, torque.z);
                let inertia = (larm * larm * rb->mass);
                rb->angular_velocity += (torque * dt) / inertia;

                Vec3 dbg_pts[] = {cp, p, p + rb->mass*g};
                Vec4 dbg_colors[] = {vec4_red, vec4_green, vec4_red};

                let c = debug_get_camera();
                renderer_debug_draw(dbg_pts, 3, dbg_colors, PRIMITIVE_TOPOLOGY_LINE_STRIP, c.pos, c.rot);
            }

            memf(s1.vertices);
            memf(s2.vertices);
        }
    }
}

static void destroy_resource(PhysicsResourceHandle h)
{
    check(handle_pool_is_valid(ps.resource_handle_pool, h), "When trying to destroy PhysicsResource; the passed Handle was invalid");

    switch(handle_type(h))
    {
        case PHYSICS_RESOURCE_TYPE_MESH: {
            let m = get_resource(PhysicsResourceMesh, h);
            memf(m->vertices);
        } break;

        case PHYSICS_RESOURCE_TYPE_WORLD: {
            let w = get_resource(PhysicsResourceWorld, h);
            memf(w->objects);
            da_free(w->rigidbodies);
            handle_pool_destroy(w->object_handle_pool);
            handle_pool_destroy(w->rigidbody_handle_pool);
        } break;
    }

    memf(ps.resources[handle_index(h)].data);
    handle_pool_return(ps.resource_handle_pool, h);

    if (ps.resources[handle_index(h)].name_hash)
        handle_hash_map_remove(ps.resource_name_to_handle, ps.resources[handle_index(h)].name_hash);
}

void physics_shutdown()
{
    for (u32 i = 0; i < ps.resources_num; ++i)
    {
        let h = ps.resources[i].handle;

        if (!h)
            continue;

        destroy_resource(h);
    }

    memf(ps.resources);
    handle_hash_map_destroy(ps.resource_name_to_handle);
    handle_pool_destroy(ps.resource_handle_pool);
}

const Vec3& physics_get_position(PhysicsResourceHandle world, PhysicsObjectHandle obj)
{
    let w = get_resource(PhysicsResourceWorld, world);
    let o = get_object(w, obj);
    return o->pos;
}

const Quat& physics_get_rotation(PhysicsResourceHandle world, PhysicsObjectHandle obj)
{
    let w = get_resource(PhysicsResourceWorld, world);
    let o = get_object(w, obj);
    return o->rot;
}