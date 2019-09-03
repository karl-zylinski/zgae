#include "game_root.h"
#include "entity.h"
#include "player.h"
#include "physics.h"
#include "time.h"
#include "renderer.h"
#include "math.h"
#include "camera.h"
#include "keyboard.h"
#include "keyboard_types.h"
#include "mouse.h"
#include "debug.h"
#include "world.h"
#include <stdlib.h>

struct GameState
{
    EntityRef floor1;
    EntityRef floor2;
    Player player;

    RenderResourceHandle pipeline;
    World* world;
    PhysicsResourceHandle box_collider;
    RenderResourceHandle box_mesh;
};

static GameState gs = {};


EntityRef spawn_entity_at(World* w, RenderResourceHandle mesh, PhysicsResourceHandle collider, const Vec3& p, const Quat& r, f32 mass, bool rigidbody)
{
    let e = entity_create(w, p, r);
    entity_set_render_mesh(&e, mesh);
    entity_set_physics_collider(&e, collider);
    
    if (rigidbody)
        entity_create_rigidbody(&e, mass);

    return e;
}

void game_init()
{
    info("Entering game_init()");
    srand (time_since_start());
    let render_world = renderer_create_world();
    let physics_world = physics_world_create(render_world);
    gs.world = world_create(render_world, physics_world);

    gs.pipeline = renderer_resource_load("pipeline_default.pipeline");
    gs.box_mesh = renderer_resource_load("box.mesh");
    let floor_render_mesh = renderer_resource_load("floor.mesh");

    gs.world->physics_world = physics_world_create(gs.world->render_world);
    let box_physics_mesh = physics_resource_load("box.mesh");
    let floor_physics_mesh = physics_resource_load("floor.mesh");
    gs.box_collider = physics_collider_create(box_physics_mesh);
    let floor_collider = physics_collider_create(floor_physics_mesh);

    gs.floor1 = spawn_entity_at(gs.world, floor_render_mesh, floor_collider, {0, 0, -5}, quat_identity(), 10000, false);
    gs.floor2 = spawn_entity_at(gs.world, floor_render_mesh, floor_collider, {-0.5, 15, -5.5}, quat_identity(), 10000, false);

    gs.player = {
        .camera = camera_create(),
        .entity = spawn_entity_at(gs.world, gs.box_mesh, gs.box_collider, {-2, 0, 10}, quat_identity(), 75, true)
    };
}

static f32 time_until_spawn = 0;

bool game_update()
{
    if (key_went_down(KC_ESCAPE))
        return false;

    physics_update_world(gs.world->physics_world);
    time_until_spawn -= time_dt();

    if (time_until_spawn <= 0)
    {
        time_until_spawn = 2.0f;
        f32 x = ((rand() % 10000)-5000)/1000.0f;
        f32 y = ((rand() % 10000)-5000)/1000.0f;
        f32 z = ((rand() % 10000)-5000)/1000.0f + 20.0f + (rand() % 10000)/10000.0f;
        f32 rx = (rand() % 628)/100;
        f32 ry = (rand() % 628)/100;
        f32 rz = (rand() % 628)/100;
        f32 rr = (rand() % 628)/100;
        spawn_entity_at(gs.world, gs.box_mesh, gs.box_collider, {x, y, z}, quat_from_axis_angle({rx, ry, rz}, rr), 10, true);
    }

    player_update(&gs.player);
    
    renderer_begin_frame(gs.pipeline);
    renderer_draw_world(gs.pipeline, gs.world->render_world, gs.player.camera.pos, gs.player.camera.rot);
    renderer_end_frame();
    renderer_present();
    keyboard_end_of_frame();
    mouse_end_of_frame();

    return true;
}

void game_shutdown()
{
    info("Entering game_shutdown()");
    world_destroy(gs.world);
}