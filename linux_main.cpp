#include "linux_xcb_window.h"
#include "window_types.h"
#include "renderer.h"
#include "keyboard_types.h"
#include "debug.h"
#include "memory.h"
#include "math.h"
#include "time.h"
#include <time.h>
#include "keyboard.h"
#include <execinfo.h>
#include <math.h>
#include "obj_loader.h"
#include "gjk_epa.h"
#include "physics.h"
#include "entity.h"
#include "camera.h"
#include "camera_first_person.h"
#include "player.h"
#include "mouse.h"

static f32 get_cur_time_seconds()
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    u32 secs = t.tv_sec;
    return (f32)secs + ((f32)t.tv_nsec)/1000000000.0f;
}

static bool window_resize_handled = true;
static f32 window_resized_at = 0;
static u32 window_resize_w = 0;
static u32 window_resize_h = 0;

static void handle_window_resize(u32 w, u32 h)
{
    window_resize_handled = false;
    window_resized_at = time_since_start();
    window_resize_w = w;
    window_resize_h = h;
}

static Backtrace get_backtrace(u32 backtrace_size)
{
    if (backtrace_size > 32)
        backtrace_size = 32;

    static void* backtraces[32];
    u32 bt_size = backtrace(backtraces, backtrace_size);
    char** bt_symbols = backtrace_symbols(backtraces, bt_size);

    return {
        .function_calls = bt_symbols,
        .function_calls_num = bt_size
    };
}


int main()
{
    info("Starting ZGAE");
    debug_init(get_backtrace);
    memory_init();
    keyboard_init();
    XcbWindow* win = linux_xcb_window_create("ZGAE", 640, 480);
    renderer_init(WINDOW_TYPE_XCB, win);
    RenderResourceHandle render_world = renderer_create_world();
    physics_init();

    WindowCallbacks wc = {};
    wc.key_pressed_callback = &keyboard_key_pressed;
    wc.key_released_callback = &keyboard_key_released;
    wc.focus_lost_callback = &keyboard_reset;
    wc.window_resized_callback = &handle_window_resize;
    wc.mouse_moved_callback = &mouse_moved;
    linux_xcb_window_update_callbacks(win, wc);

    let pipeline = renderer_resource_load("pipeline_default.pipeline");
    let box_render_mesh = renderer_resource_load("box.mesh");
    let floor_render_mesh = renderer_resource_load("floor.mesh");

    let physics_world = physics_world_create(render_world);
    let box_physics_mesh = physics_resource_load("box.mesh");
    let floor_physics_mesh = physics_resource_load("floor.mesh");
    let box_collider = physics_collider_create(box_physics_mesh);
    let floor_collider = physics_collider_create(floor_physics_mesh);

    let e1 = entity_create({-4, 0, 5}, quat_identity(), render_world, box_render_mesh, physics_world, box_collider);
    (void)e1;
    let e2 = entity_create({4, 0, 9}, quat_identity(), render_world, box_render_mesh, physics_world, box_collider);
    (void)e2;

    let floor = entity_create({0, 0, -5}, quat_identity(), render_world, floor_render_mesh, physics_world, floor_collider);
    (void)floor;

    entity_create_rigidbody(&e1);
    entity_create_rigidbody(&e2);

    Player player = {
        .camera = camera_create(),
        .entity = &e1
    };


    info("Starting timers");
    
    f32 start_time = get_cur_time_seconds();
    f32 last_frame_time = start_time;

    info("Entering main loop");

    while (linux_xcb_window_is_open(win) && !key_held(KC_ESCAPE))
    {
        f32 cur_time = get_cur_time_seconds();
        f32 dt = cur_time - last_frame_time;
        f32 since_start = cur_time - start_time;
        last_frame_time = cur_time;
        set_frame_timers(dt, since_start);

        renderer_wait_for_new_frame();
        linux_xcb_window_process_all_events(win);

        if (window_resize_handled == false && time_since_start() > window_resized_at + 0.5f)
        {
            window_resize_handled = true;
            renderer_surface_resized(window_resize_w, window_resize_h);
        }

        player_update(&player);
        physics_update_world(physics_world);
        renderer_begin_frame(pipeline);
        renderer_draw_world(pipeline, render_world, player.camera.pos, player.camera.rot);
        renderer_end_frame();
        renderer_present();
        keyboard_end_of_frame();
        mouse_end_of_frame();
    }

    info("Main loop exited, shutting down");
    physics_shutdown();
    renderer_shutdown();
    linux_xcb_window_destroy(win);
    memory_check_leaks();
    info("Shutdown finished");
    return 0;
}