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
#include "world.h"

struct GameState
{
    EntityRef e1;
    EntityRef e2;
    EntityRef floor1;
    EntityRef floor2;
    Player player;

    RenderResourceHandle pipeline;
    World* world;
};

static GameState gs = {};


void game_init()
{
    info("Entering game_init()");
    let render_world = renderer_create_world();
    let physics_world = physics_world_create(render_world);
    gs.world = world_create(render_world, physics_world);

    gs.pipeline = renderer_resource_load("pipeline_default.pipeline");
    let box_render_mesh = renderer_resource_load("box.mesh");
    let floor_render_mesh = renderer_resource_load("floor.mesh");

    gs.world->physics_world = physics_world_create(gs.world->render_world);
    let box_physics_mesh = physics_resource_load("box.mesh");
    let floor_physics_mesh = physics_resource_load("floor.mesh");
    let box_collider = physics_collider_create(box_physics_mesh);
    let floor_collider = physics_collider_create(floor_physics_mesh);

    gs.e1 = entity_create(gs.world, {-4, 0, 5}, quat_identity());
    entity_set_render_mesh(&gs.e1, box_render_mesh);
    entity_set_physics_collider(&gs.e1, box_collider);
    gs.e2 = entity_create(gs.world, {4, 0, 9}, quat_identity());
    entity_set_render_mesh(&gs.e2, box_render_mesh);
    entity_set_physics_collider(&gs.e2, box_collider);
    gs.floor1 = entity_create(gs.world, {0, 0, -5}, quat_identity());
    entity_set_render_mesh(&gs.floor1, floor_render_mesh);
    entity_set_physics_collider(&gs.floor1, floor_collider);
    gs.floor2 = entity_create(gs.world, {2, 3, -10}, quat_identity());
    entity_set_render_mesh(&gs.floor2, floor_render_mesh);
    entity_set_physics_collider(&gs.floor2, floor_collider);

    entity_create_rigidbody(&gs.e1);
    entity_create_rigidbody(&gs.e2);

    gs.player = {
        .camera = camera_create(),
        .entity = gs.e1
    };
}

void game_update()
{
    player_update(&gs.player);
    physics_update_world(gs.world->physics_world);
    renderer_begin_frame(gs.pipeline);
    renderer_draw_world(gs.pipeline, gs.world->render_world, gs.player.camera.pos, gs.player.camera.rot);
    renderer_end_frame();
    renderer_present();
    keyboard_end_of_frame();
    mouse_end_of_frame();
}

void game_shutdown()
{
    info("Entering game_shutdown()");
}