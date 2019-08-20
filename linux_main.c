#include "linux_xcb_window.h"
#include "window_types.h"
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
#include "keyboard.h"

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

static bool window_resize_handled = true;
static float window_resized_at = 0;
static uint32_t window_resize_w = 0;
static uint32_t window_resize_h = 0;

static void handle_window_resize(uint32_t w, uint32_t h)
{
    window_resize_handled = false;
    window_resized_at = time_since_start();
    window_resize_w = w;
    window_resize_h = h;
}

int main()
{
    info("Starting ZGAE");
    memory_init();
    keyboard_init();
    linux_xcb_window_t* win = linux_xcb_window_create("ZGAE", 640, 480);
    renderer_state_t* rs = renderer_create(WINDOW_TYPE_XCB, win);

    window_callbacks_t wc = {};
    wc.key_pressed_callback = &keyboard_key_pressed;
    wc.key_released_callback = &keyboard_key_released;
    wc.focus_lost_callback = &keyboard_reset;
    wc.window_resized_callback = &handle_window_resize;
    linux_xcb_window_update_callbacks(win, &wc);

    renderer_resource_handle_t ph = pipeline_load(rs, "pipeline_default.pipeline");
    renderer_resource_handle_t gh = renderer_load_geometry(rs, cube, sizeof(cube) / sizeof(geometry_vertex_t));

    info("Starting timers");
    
    float start_time = get_cur_time_seconds();
    float last_frame_time = start_time;
    float framerate_timer = 0;
    uint32_t frames = 0;

    info("Entering main loop");

    float xpos = 0;
    float ypos = 0;

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
            (void)fps;
            //info("%f fps", fps);
            framerate_timer = time_since_start();
            frames = 0;
        }

        if (key_is_held(KC_A))
            xpos -= time_dt();
        if (key_is_held(KC_D))
            xpos += time_dt();
        if (key_is_held(KC_W))
            ypos += time_dt();
        if (key_is_held(KC_S))
            ypos -= time_dt();

        vec3_t camera_pos = {0, -4, 0};//{2.5, -4, 1.5};
        quat_t camera_rot = quat_identity(); //{-0.3333, 0, 0.3333, 0.6667};
        mat4_t camera_matrix = mat4_from_rotation_and_translation(&camera_rot, &camera_pos);

        mat4_t view_matrix = mat4_inverse(&camera_matrix);

        mat4_t model_matrix = mat4_identity();
        model_matrix.w.x = xpos;
        model_matrix.w.y = ypos;
        model_matrix.x.x = 1 + 0.5*sin(time_since_start());

        const window_state_t* ws = linux_xcb_window_get_state(win);
        mat4_t proj_matrix = mat4_create_projection_matrix((float)ws->width, (float)ws->height);
        mat4_t proj_view_matrix = mat4_mul(&view_matrix, &proj_matrix);
        mat4_t mvp_matrix = mat4_mul(&model_matrix, &proj_view_matrix);

        renderer_wait_for_new_frame(rs);
        linux_xcb_window_process_all_events(win);

        if (window_resize_handled == false && time_since_start() > window_resized_at + 0.5f)
        {
            window_resize_handled = true;
            renderer_surface_resized(rs, window_resize_w, window_resize_h);
        }

        renderer_update_constant_buffer(rs, ph, 0, &mvp_matrix, sizeof(mvp_matrix));
        renderer_draw(rs, ph, gh);
        renderer_present(rs);
        keyboard_end_of_frame();
    }

    info("Main loop exited, shutting down");
    renderer_destroy(rs);
    linux_xcb_window_destroy(win);
    memory_check_leaks();
    info("Shutdown finished");
    return 0;
}