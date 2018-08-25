#include "config.h"
#include "windows_window.h"
#include "renderer_direct3d.h"
#include "memory.h"
#include "world.h"
#include "camera.h"
#include "test_world.h"
#include "keyboard.h"
#include "mouse.h"

static void key_pressed_callback(Key key)
{
    keyboard_pressed(key);
}

static void key_released_callback(Key key)
{
    keyboard_released(key);
}

static void mouse_moved_callback(const Vector2i& delta)
{
    mouse_add_delta(delta);
}

static void process_input(Camera* camera)
{
    Matrix4x4 move = matrix4x4_identity();

    if (key_is_held(Key::W))
    {
        move.w.z += 0.0005f;
    }
    if (key_is_held(Key::S))
    {
        move.w.z -= 0.0005f;
    }
    if (key_is_held(Key::A))
    {
        move.w.x -= 0.0005f;
    }
    if (key_is_held(Key::D))
    {
        move.w.x += 0.0005f;
    }

    Quaternion rotation = camera->rotation;
    Vector2i mouse_movement = mouse_movement_delta();
    if (mouse_movement.x != 0 || mouse_movement.y != 0)
    {
        rotation = quaternion_rotate_y(rotation, mouse_movement.x * 0.001f);
        rotation = quaternion_rotate_x(rotation, mouse_movement.y * 0.001f);
    }

    camera->rotation = rotation;
    Matrix4x4 camera_test_mat = matrix4x4_from_rotation_and_translation(camera->rotation, vector3_zero);
    Matrix4x4 movement_rotated = move * camera_test_mat;
    camera->position += *(Vector3*)&movement_rotated.w.x;
}

int main()
{
    void* temp_memory_block = VirtualAlloc(nullptr, TempMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    Assert(temp_memory_block != nullptr, "Failed allocating temp memory.");
    temp_memory_blob_init(temp_memory_block, TempMemorySize);

    void* permanent_memory_block = VirtualAlloc(nullptr, PermanentMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    Assert(permanent_memory_block != nullptr, "Failed allocating permanent memory.");
    permanent_memory_blob_init(permanent_memory_block, PermanentMemorySize);
    
    keyboard_init();
    mouse_init();

    WindowsWindow win = {};
    windows_create_window(&win, G_program_name, G_default_window_width, G_default_window_height);
    win.state.key_released_callback = key_released_callback;
    win.state.key_pressed_callback = key_pressed_callback;
    win.state.mouse_moved_callback = mouse_moved_callback;
    Renderer renderer = {};
    renderer.init(win.handle);

    Allocator alloc = create_heap_allocator();
    World world = {};
    world_init(&world, &alloc);
    create_test_world(&world, &renderer);

    Camera camera = camera_create_projection();
    RRHandle default_shader = renderer.load_shader("shader.shader");
    renderer.set_shader(default_shader);
    renderer.disable_scissor();
    
    while(win.state.closed == false)
    {
        windows_process_all_window_messsages();
        process_input(&camera);
        renderer.pre_draw_frame();
        renderer.draw_frame(world, camera, DrawLights::DrawLights);
        renderer.present();
        keyboard_end_of_frame();
        mouse_end_of_frame();
    }

    world_destroy(&world);
    renderer.shutdown();
    heap_allocator_check_clean(&alloc);
}