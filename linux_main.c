#include "linux_xcb_window.h"
#include "window.h"
#include "renderer.h"
#include "keycode_types.h"
#include "debug.h"
#include "memory.h"
#include "jzon.h"
#include "pipeline.h"
#include "geometry_types.h"
#include "math.h"
#include "time.h"
#include <time.h>
#include <math.h>

void key_pressed(keycode_t k)
{
    info("pressed: %d", (uint32_t)k);
}

void key_released(keycode_t k)
{
    info("pressed: %d", (uint32_t)k);
}

#define XYZ1(_x_, _y_, _z_) {(_x_), (_y_), (_z_), 1.f}
static geometry_vertex_t cube[] = {
    // red face
    {XYZ1(-1, -1, 1), XYZ1(1.f, 0.f, 0.f)},
    {XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 0.f)},
    {XYZ1(1, -1, 1), XYZ1(1.f, 0.f, 0.f)},
    {XYZ1(1, -1, 1), XYZ1(1.f, 0.f, 0.f)},
    {XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 0.f)},
    {XYZ1(1, 1, 1), XYZ1(1.f, 0.f, 0.f)},
    // green face
    {XYZ1(-1, -1, -1), XYZ1(0.f, 1.f, 0.f)},
    {XYZ1(1, -1, -1), XYZ1(0.f, 1.f, 0.f)},
    {XYZ1(-1, 1, -1), XYZ1(0.f, 1.f, 0.f)},
    {XYZ1(-1, 1, -1), XYZ1(0.f, 1.f, 0.f)},
    {XYZ1(1, -1, -1), XYZ1(0.f, 1.f, 0.f)},
    {XYZ1(1, 1, -1), XYZ1(0.f, 1.f, 0.f)},
    // blue face
    {XYZ1(-1, 1, 1), XYZ1(0.f, 0.f, 1.f)},
    {XYZ1(-1, -1, 1), XYZ1(0.f, 0.f, 1.f)},
    {XYZ1(-1, 1, -1), XYZ1(0.f, 0.f, 1.f)},
    {XYZ1(-1, 1, -1), XYZ1(0.f, 0.f, 1.f)},
    {XYZ1(-1, -1, 1), XYZ1(0.f, 0.f, 1.f)},
    {XYZ1(-1, -1, -1), XYZ1(0.f, 0.f, 1.f)},
    // yellow face
    {XYZ1(1, 1, 1), XYZ1(1.f, 1.f, 0.f)},
    {XYZ1(1, 1, -1), XYZ1(1.f, 1.f, 0.f)},
    {XYZ1(1, -1, 1), XYZ1(1.f, 1.f, 0.f)},
    {XYZ1(1, -1, 1), XYZ1(1.f, 1.f, 0.f)},
    {XYZ1(1, 1, -1), XYZ1(1.f, 1.f, 0.f)},
    {XYZ1(1, -1, -1), XYZ1(1.f, 1.f, 0.f)},
    // magenta face
    {XYZ1(1, 1, 1), XYZ1(1.f, 0.f, 1.f)},
    {XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 1.f)},
    {XYZ1(1, 1, -1), XYZ1(1.f, 0.f, 1.f)},
    {XYZ1(1, 1, -1), XYZ1(1.f, 0.f, 1.f)},
    {XYZ1(-1, 1, 1), XYZ1(1.f, 0.f, 1.f)},
    {XYZ1(-1, 1, -1), XYZ1(1.f, 0.f, 1.f)},
    // cyan face
    {XYZ1(1, -1, 1), XYZ1(0.f, 1.f, 1.f)},
    {XYZ1(1, -1, -1), XYZ1(0.f, 1.f, 1.f)},
    {XYZ1(-1, -1, 1), XYZ1(0.f, 1.f, 1.f)},
    {XYZ1(-1, -1, 1), XYZ1(0.f, 1.f, 1.f)},
    {XYZ1(1, -1, -1), XYZ1(0.f, 1.f, 1.f)},
    {XYZ1(-1, -1, -1), XYZ1(0.f, 1.f, 1.f)},
};

static float get_cur_time_seconds()
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    uint32_t secs = t.tv_sec;
    return (float)secs + ((float)t.tv_nsec)/1000000000.0f;
}

int main()
{
    info("Starting ZGAE");
    memory_init();
    
    linux_xcb_window_t* win = linux_xcb_window_create("ZGAE", 640, 480);

    window_callbacks_t wc = {};
    wc.key_pressed_callback = &key_pressed;
    wc.key_released_callback = &key_released;
    linux_xcb_window_update_callbacks(win, &wc);

    renderer_state_t* renderer_state = renderer_create(WINDOW_TYPE_XCB, win);
    renderer_resource_handle_t ph = pipeline_load(renderer_state, "pipeline_default.pipeline");
    renderer_resource_handle_t gh = renderer_load_geometry(renderer_state, cube, sizeof(cube) / sizeof(geometry_vertex_t));
    
    mat4_t proj_matrix = mat4_create_projection_matrix((float)640, (float)480);

    info("Starting timers");
    
    float start_time = get_cur_time_seconds();
    float last_frame_time = start_time;
    float framerate_timer = 0;
    uint32_t frames = 0;

    info("Entering main loop");

    while (linux_xcb_window_is_open(win))
    {
        float cur_time = get_cur_time_seconds();
        float dt = cur_time - last_frame_time;
        float since_start = cur_time - start_time;
        last_frame_time = cur_time;
        set_frame_timers(dt, since_start);
        frames = frames + 1;

        if (time_since_start() > (framerate_timer + 2))
        {
            float d = time_since_start() - framerate_timer;
            float fps = ((float)frames)/d;
            info("%f fps", fps);
            framerate_timer = time_since_start();
            frames = 0;
        }

        vec3_t camera_pos = {2.5 + cos(time_since_start()), -4 + sin(time_since_start()), 1.5};
        quat_t camera_rot = {-0.3333, 0, 0.3333, 0.6667};
        mat4_t camera_matrix = mat4_from_rotation_and_translation(&camera_rot, &camera_pos);

        mat4_t view_matrix = mat4_inverse(&camera_matrix);

        mat4_t model_matrix = mat4_identity();
        mat4_t proj_view_matrix = mat4_mul(&view_matrix, &proj_matrix);
        mat4_t mvp_matrix = mat4_mul(&proj_view_matrix, &model_matrix);

        renderer_wait_for_new_frame(renderer_state);
        renderer_update_constant_buffer(renderer_state, ph, 0, &mvp_matrix, sizeof(mvp_matrix));
        linux_xcb_window_process_all_events(win);
        renderer_draw(renderer_state, ph, gh);
        renderer_present(renderer_state);
    }

    info("Main loop exited, shutting down");
    renderer_destroy(renderer_state);
    linux_xcb_window_destroy(win);
    memory_check_leaks();
    info("Shutdown finished");
    return 0;
}