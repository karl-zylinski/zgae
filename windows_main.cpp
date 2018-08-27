#include "config.h"
#include "windows_window.h"
#include "renderer_direct3d.h"
#include "memory.h"
#include "keyboard.h"
#include "mouse.h"
#include "game_main.h"
#include "renderer_direct3d.h"

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

int main()
{
    memory_init();
    keyboard_init();
    mouse_init();

    WindowsWindow win = {};
    windows_create_window(&win, G_program_name, G_default_window_width, G_default_window_height);
    win.state.key_released_callback = key_released_callback;
    win.state.key_pressed_callback = key_pressed_callback;
    win.state.mouse_moved_callback = mouse_moved_callback;
    RendererD3D renderer = {};
    renderer.init(win.handle);

    RRHandle default_shader = renderer.load_shader("shader.shader");
    renderer.set_shader(default_shader);
    
    game_start(&renderer);

    while(win.state.closed == false)
    {
        windows_process_all_window_messsages();
        game_update(&renderer);
        renderer.pre_draw_frame();
        game_draw(&renderer);
        renderer.present();
        keyboard_end_of_frame();
        mouse_end_of_frame();
    }

    game_shutdown(&renderer);
    renderer.shutdown();
    memory_shutdown();
}