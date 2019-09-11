#include "physics.h"
#include "memory.h"
#include "idx_hash_map.h"
#include "str.h"
#include "log.h"
#include "path.h"
#include "time.h"
#include "gjk_epa.h"
#include "file.h"
#include "jzon.h"
#include "obj_loader.h"
#include "render_resource.h"
#include "debug.h"
#include "camera.h"
#include <math.h>
#include "dynamic_array.h"

struct PhysicsMesh
{
    u32 idx;
    Vec3* vertices;
    u32 vertices_num;
};

struct PhysicsState
{
    PhysicsMesh* meshes; // dynamic
    u32* meshes_free_idx; // dynamic
};

struct PhysicsObject
{
    u32 idx;
    PhysicsCollider collider;
    u32 rigidbody_idx;
    u32 render_object_idx;
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
    u32 idx;
    u32 object_idx;
    Vec3 velocity;
    Vec3 angular_velocity;
    f32 mass;
};

struct PhysicsWorld
{
    PhysicsObject* objects; // dynamic
    u32* objects_free_idx; // dynamic
    Rigidbody* rigidbodies; // dynamic
    u32* rigidbodies_free_idx; // dynamic
};

static PhysicsState ps = {};
static bool inited = false;

void physics_init()
{
    check(!inited, "Trying to init physics twice");
    inited = true;

    da_push(ps.meshes, PhysicsMesh{}); // dummy
}

u32 physics_load_mesh(const char* filename)
{
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

    let idx = da_num(ps.meshes_free_idx) > 0 ? da_pop(ps.meshes_free_idx) : da_num(ps.meshes);
    PhysicsMesh m  = {
        .idx = idx,
        .vertices = obj_vertices.vertices,
        .vertices_num = obj_vertices.vertices_num
    };
    da_insert(ps.meshes, m, idx);
    return idx;
}

void physics_destroy_mesh(u32 mesh_idx)
{
    let m = ps.meshes + mesh_idx;
    memf(m->vertices);
    memzero(m, sizeof(PhysicsMesh));
}

u32 physics_create_rigidbody(PhysicsWorld* w, u32 object_idx,  f32 mass, const Vec3& velocity)
{
    check(mass > 0, "Mass must be in range (0, inf)");
    let o = w->objects + object_idx;
    check(!o->rigidbody_idx, "Trying to create rigidbody for physics object that already has one");
    let idx = da_num(w->rigidbodies_free_idx) > 0 ? da_pop(w->rigidbodies_free_idx) : da_num(w->rigidbodies);

    Rigidbody r = {
        .idx = idx,
        .mass = mass,
        .object_idx = object_idx,
        .velocity = velocity
    };

    o->rigidbody_idx = idx;
    da_insert(w->rigidbodies, r, idx);
    return idx;
}

void physics_set_velocity(PhysicsWorld* w, u32 rigidbody_idx, const Vec3& vel)
{
    let rb = w->rigidbodies + rigidbody_idx;
    rb->velocity = vel;
}

void physics_add_force(PhysicsWorld* w, u32 rigidbody_idx, const Vec3& f)
{
    let rb = w->rigidbodies + rigidbody_idx;
    let acc = f/rb->mass;
    rb->velocity += acc;
}

void physics_add_torque(PhysicsWorld* w, u32 rigidbody_idx, const Vec3& pivot, const Vec3& point, const Vec3& force)
{
    let rb = w->rigidbodies + rigidbody_idx;

    // add moment of inertia

    let arm = point - pivot;
    let larm = len(arm);
    rb->angular_velocity += cross(arm, force) * (1/(larm * larm * rb->mass));
}

PhysicsCollider physics_create_collider(u32 mesh_idx)
{
    return { .mesh_idx = mesh_idx };
}

PhysicsWorld* physics_create_world()
{
    let w = mema_zero_t(PhysicsWorld);
    da_push(w->objects, PhysicsObject{}); // zero-dummy
    da_push(w->rigidbodies, Rigidbody{}); // zero-dummy
    return w;
}

u32 physics_create_object(PhysicsWorld* w, const PhysicsCollider& collider, u32 render_object_idx, const Vec3& pos, const Quat& rot, const PhysicsMaterial& pm)
{
    let idx = da_num(w->objects_free_idx) > 0 ? da_pop(w->objects_free_idx) : da_num(w->objects);

    PhysicsObject o = {
        .idx = idx,
        .collider = collider,
        .pos = pos,
        .rot = rot,
        .render_object_idx = render_object_idx,
        .material = pm
    };

    da_insert(w->objects, o, idx);
    return idx;
}

void physics_set_position(PhysicsWorld* w, u32 object_idx, const Vec3& pos, const Quat& rot)
{
    let o = w->objects + object_idx;
    o->pos = pos;
    o->rot = rot;
}

void physics_update_world(PhysicsWorld* w)
{
    float dt = time_dt();
    //float t = time_since_start();

    for (u32 rigidbody_idx = 0; rigidbody_idx < da_num(w->rigidbodies); ++rigidbody_idx)
    {
        let rb = w->rigidbodies + rigidbody_idx;
        if (!rb->idx)
            continue;

        // Update rigidbody state.
        Vec3 g = {0, 0, -9.82f};

        physics_add_force(w, rigidbody_idx, g * rb->mass * dt);
        let wo = w->objects + rb->object_idx;

        for (u32 world_object_index = 0; world_object_index < da_num(w->objects); ++world_object_index)
        {
            if (world_object_index == rb->object_idx || !w->objects[world_object_index].idx)
                continue;

            // TODO: cache the shapes etc

            let c1 = wo->collider;
            let p1 = wo->pos;
            let r1 = wo->rot;
            let m1 = ps.meshes + c1.mesh_idx;

            GjkShape s1 = {
                .vertices = mema_tn(Vec3, m1->vertices_num),
                .vertices_num = m1->vertices_num
            };

            for (u32 vi = 0; vi < s1.vertices_num; ++vi)
                s1.vertices[vi] = rotate_vec3(r1, m1->vertices[vi]) + p1;

            let wo_colliding_with = w->objects + world_object_index;

            let c2 = wo_colliding_with->collider;
            let p2 = wo_colliding_with->pos;
            let r2 = wo_colliding_with->rot;
            let m2 = ps.meshes + c2.mesh_idx;

            GjkShape s2 = {
                .vertices = mema_tn(Vec3, m2->vertices_num),
                .vertices_num = m2->vertices_num
            };

            for (u32 vi = 0; vi < s2.vertices_num; ++vi)
                s2.vertices[vi] = rotate_vec3(r2, m2->vertices[vi]) + p2;

            let coll = gjk_epa_intersect_and_solve(s1, s2);

            if (coll.colliding)
            {
                let collding_with_rigidbody = wo_colliding_with->rigidbody_idx != 0;
                let sol = coll.solution;
                //wo->pos += sol;
                let m = rb->mass;
                let vel = rb->velocity;
                let avel = rb->angular_velocity;
                let r = wo->pos - coll.contact_point;
                let vel_cp = vel + cross(avel, r);

                #define SOLUTION_THRES 0.0001f
                if (!collding_with_rigidbody)
                {
                    if (dot(vel_cp, sol) < 0 && len(sol) > SOLUTION_THRES)
                    {
                        let n = normalize(sol);
                        let vel_cp_normal = project(vel_cp, n);
                        let vel_in_surface_dir = vel_cp - vel_cp_normal;
                        let friction = fmin(wo->material.friction + wo_colliding_with->material.friction, 1);

                        let normal_f = -m*vel_cp_normal;
                        let friction_f = -(len(m*g*dt)*friction*vel_in_surface_dir);

                        let f = normal_f + friction_f;
                        let t = cross(vel_cp_normal, f);

                        physics_add_force(w, rigidbody_idx, f);
                        //physics_add_torque(world, rb->handle, wo->pos, coll.contact_point, f/10);
                        (void)t;
                    }
                    else
                        wo->pos += sol;
                }
                else
                {
                    wo->pos += sol;
                }
            }

            memf(s1.vertices);
            memf(s2.vertices);
        }

        // Move rigidbody according to velocties
        wo->pos += rb->velocity * dt;
        wo->rot *= quat_from_axis_angle(rb->angular_velocity, dt);
    }
}

void physics_destroy_world(PhysicsWorld* w)
{
    da_free(w->objects);
    da_free(w->objects_free_idx);
    da_free(w->rigidbodies);
    da_free(w->rigidbodies_free_idx);
    memf(w);
}

void physics_shutdown()
{
    for (u32 i = 0; i < da_num(ps.meshes); ++i)
        if (ps.meshes[i].idx)
            physics_destroy_mesh(i);

    da_free(ps.meshes);
    da_free(ps.meshes_free_idx);
}

const Vec3& physics_get_position(PhysicsWorld* w, u32 object_idx)
{
    return w->objects[object_idx].pos;
}

const Quat& physics_get_rotation(PhysicsWorld* w, u32 object_idx)
{
    return w->objects[object_idx].rot;
}