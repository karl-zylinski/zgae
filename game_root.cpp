#include "game_root.h"
#include "player.h"
#include "time.h"
#include "keyboard.h"
#include "mouse.h"
#include "log.h"
#include "world.h"
#include <stdlib.h>
#include "renderer.h"
#include "physics.h"
#include "memory.h"

struct GameState
{
    Entity floor1;
    Entity floor2;
    Player player;

    u32 pipeline_idx;
    World* world;
    PhysicsCollider box_collider;
    u32 box_render_mesh_idx;
    PhysicsCollider big_box_collider;
    u32 big_box_render_mesh_idx;
};

static GameState gs = {};


Entity spawn_entity_at(World* w, u32 mesh_idx, const PhysicsCollider& collider, const Vec3& p, const Quat& r, const Vec3& vel, f32 mass, const PhysicsMaterial& pm, bool rigidbody)
{
    let e = entity_create(w, p, r);

    if (mesh_idx)
        e.set_render_mesh(mesh_idx);

    e.set_physics_collider(collider, pm);
    
    if (rigidbody)
        e.create_rigidbody(mass, vel);

    return e;
}

void game_init()
{
    info("Entering game_init()");
    srand (time_since_start());
    let render_world = renderer_create_world();
    let physics_world = physics_create_world();
    gs.world = create_world(render_world, physics_world);

    gs.pipeline_idx = renderer_load_pipeline("pipeline_default.pipeline");
    gs.box_render_mesh_idx = renderer_load_mesh("box.mesh");
    gs.big_box_render_mesh_idx = renderer_load_mesh("box.mesh");
    let floor_render_mesh = renderer_load_mesh("floor.mesh");

    let box_physics_mesh = physics_load_mesh("box.mesh");
    let big_box_physics_mesh = physics_load_mesh("box.mesh");
    let floor_physics_mesh = physics_load_mesh("floor.mesh");
    gs.box_collider = physics_create_collider(box_physics_mesh);
    gs.big_box_collider = physics_create_collider(big_box_physics_mesh);
    let floor_collider = physics_create_collider(floor_physics_mesh);

    PhysicsMaterial floor_material = {
        .elasticity = 0,
        .friction = 0.2f
    };

    gs.floor1 = spawn_entity_at(gs.world, floor_render_mesh, floor_collider, {0, 0, -5}, quat_identity(), vec3_zero, 10000, floor_material, false);
    gs.floor2 = spawn_entity_at(gs.world, floor_render_mesh, floor_collider, {-0.5, 15, -10}, quat_identity(), vec3_zero, 10000, floor_material, false);

    PhysicsMaterial player_material = {
        .elasticity = 0.3f,
        .friction = 0.3f
    };

    gs.player = {
        .camera = camera_create(),
        .entity = spawn_entity_at(gs.world, 0, gs.box_collider, {-0, 1, -2}, quat_identity(), vec3_zero, 75, player_material, false)
    };
}

static f32 time_until_spawn = 1.0f;
static u32 num_spawned = 0;

bool game_update()
{
    if (key_went_down(KEY_ESCAPE))
        return false;

    renderer_begin_frame(gs.pipeline_idx);
    gs.world->update();

    time_until_spawn -= time_dt();

    PhysicsMaterial box_material = {
        .elasticity = 0.0f,
        .friction = 0.4f
    };

    if (time_until_spawn <= 0.0f && num_spawned < 1)
    {
        num_spawned++;
        time_until_spawn = 2.0f;
        f32 x = ((rand() % 10000)-5000)/10000.0f;
        //f32 y = ((rand() % 10000)-5000)/10000.0f + 10;
        f32 y = 6;
        f32 z = 1.0f;
        /*f32 rx = (rand() % 628)/100;
        f32 ry = (rand() % 628)/100;
        f32 rz = (rand() % 628)/100;
        f32 rr = (rand() % 628)/100;*/
        f32 rx = 0 ;
        f32 ry = 0 ;
        f32 rz = 0 ;
        f32 rr = 0 ;
        spawn_entity_at(gs.world, gs.big_box_render_mesh_idx, gs.big_box_collider, {x, y, z}, quat_from_axis_angle({rx, ry, rz}, rr), {0, 0, 0},  100, box_material, true);
    }

    gs.player.update();
    renderer_draw_world(gs.pipeline_idx, gs.world->render_world, gs.player.camera.pos, gs.player.camera.rot);
    renderer_present();
    keyboard_end_of_frame();
    mouse_end_of_frame();

    return true;
}

void game_shutdown()
{
    info("Entering game_shutdown()");
    renderer_destroy_world(gs.world->render_world);
    physics_destroy_world(gs.world->physics_world);
    destroy_world(gs.world);
}