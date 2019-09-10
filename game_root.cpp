#include "game_root.h"
#include "player.h"
#include "time.h"
#include "keyboard.h"
#include "mouse.h"
#include "log.h"
#include "world.h"
#include <stdlib.h>

struct GameState
{
    Entity floor1;
    Entity floor2;
    Player player;

    RenderResourceHandle pipeline;
    World* world;
    PhysicsResourceHandle box_collider;
    RenderResourceHandle box_mesh;
    PhysicsResourceHandle big_box_collider;
    RenderResourceHandle big_box_mesh;
};

static GameState gs = {};


Entity spawn_entity_at(World* w, RenderResourceHandle mesh, PhysicsResourceHandle collider, const Vec3& p, const Quat& r, const Vec3& vel, f32 mass, const PhysicsMaterial& pm, bool rigidbody)
{
    let e = entity_create(w, p, r);
    e.set_render_mesh(mesh);
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
    let physics_world = physics_create_world(render_world);
    gs.world = create_world(render_world, physics_world);

    gs.pipeline = renderer_load_resource("pipeline_default.pipeline");
    gs.box_mesh = renderer_load_resource("box.mesh");
    gs.big_box_mesh = renderer_load_resource("box.mesh");
    let floor_render_mesh = renderer_load_resource("floor.mesh");

    gs.world->physics_world = physics_create_world(gs.world->render_world);
    let box_physics_mesh = physics_load_resource("box.mesh");
    let big_box_physics_mesh = physics_load_resource("box.mesh");
    let floor_physics_mesh = physics_load_resource("floor.mesh");
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
        .entity = spawn_entity_at(gs.world, 0, gs.box_collider, {-0, 1, -2}, quat_identity(), vec3_zero,  75, player_material, false)
    };
}

static f32 time_until_spawn = 1.0f;
static u32 num_spawned = 0;

bool game_update()
{
    if (key_went_down(KEY_ESCAPE))
        return false;

    renderer_begin_frame(gs.pipeline);
    gs.world->update();

    time_until_spawn -= time_dt();

    PhysicsMaterial box_material = {
        .elasticity = 0.0f,
        .friction = 0.4f
    };

    if (time_until_spawn <= 0.0f && num_spawned < 10)
    {
        num_spawned++;
        time_until_spawn = 2.0f;
        f32 x = ((rand() % 10000)-5000)/10000.0f;
        //f32 y = ((rand() % 10000)-5000)/10000.0f + 10;
        f32 y = 9.5;
        f32 z = 1.0f;
        /*f32 rx = (rand() % 628)/100;
        f32 ry = (rand() % 628)/100;
        f32 rz = (rand() % 628)/100;
        f32 rr = (rand() % 628)/100;*/
        f32 rx = 0 ;
        f32 ry = 0 ;
        f32 rz = 0 ;
        f32 rr = 0 ;
        spawn_entity_at(gs.world, gs.big_box_mesh, gs.big_box_collider, {x, y, z}, quat_from_axis_angle({rx, ry, rz}, rr), {0, 0, 0},  100, box_material, true);
    }

    gs.player.update();
    renderer_draw_world(gs.pipeline, gs.world->render_world, gs.player.camera.pos, gs.player.camera.rot);
    renderer_present();
    keyboard_end_of_frame();
    mouse_end_of_frame();

    return true;
}

void game_shutdown()
{
    info("Entering game_shutdown()");
    destroy_world(gs.world);
}