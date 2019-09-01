#include "game_root.h"
#include "entity.h"
#include "player.h"
#include "physics.h"
#include "renderer.h"
#include "math.h"
#include "camera.h"
#include "keyboard.h"
#include "mouse.h"
#include "debug.h"

struct GameState
{
    Entity e1;
    Entity e2;
    Entity floor1;
    Entity floor2;
    Player player;

    RenderResourceHandle pipeline;
    RenderResourceHandle render_world;
    PhysicsResourceHandle physics_world;
};

static GameState gs = {};

void game_init()
{
    info("Entering game_init()");
    gs.render_world = renderer_create_world();
    gs.pipeline = renderer_resource_load("pipeline_default.pipeline");
    let box_render_mesh = renderer_resource_load("box.mesh");
    let floor_render_mesh = renderer_resource_load("floor.mesh");

    gs.physics_world = physics_world_create(gs.render_world);
    let box_physics_mesh = physics_resource_load("box.mesh");
    let floor_physics_mesh = physics_resource_load("floor.mesh");
    let box_collider = physics_collider_create(box_physics_mesh);
    let floor_collider = physics_collider_create(floor_physics_mesh);

    gs.e1 = entity_create({-4, 0, 5}, quat_identity(), gs.render_world, box_render_mesh, gs.physics_world, box_collider);
    gs.e2 = entity_create({4, 0, 9}, quat_identity(), gs.render_world, box_render_mesh, gs.physics_world, box_collider);
    gs.floor1 = entity_create({0, 0, -5}, quat_identity(), gs.render_world, floor_render_mesh, gs.physics_world, floor_collider);
    gs.floor2 = entity_create({2, 3, -10}, quat_identity(), gs.render_world, floor_render_mesh, gs.physics_world, floor_collider);

    entity_create_rigidbody(&gs.e1);
    entity_create_rigidbody(&gs.e2);

    gs.player = {
        .camera = camera_create(),
        .entity = &gs.e1
    };
}

void game_update()
{
    player_update(&gs.player);
    physics_update_world(gs.physics_world);
    renderer_begin_frame(gs.pipeline);
    renderer_draw_world(gs.pipeline, gs.render_world, gs.player.camera.pos, gs.player.camera.rot);
    renderer_end_frame();
    renderer_present();
    keyboard_end_of_frame();
    mouse_end_of_frame();
}

void game_shutdown()
{
    info("Entering game_shutdown()");
}