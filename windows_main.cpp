#include "config.h"
#include "windows_window.h"
#include "renderer_direct3d.h"
#include "memory.h"

int main()
{
    void* temp_memory_block = VirtualAlloc(nullptr, TempMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    Assert(temp_memory_block != nullptr, "Failed allocating temp memory.");
    temp_memory_blob_init(temp_memory_block, TempMemorySize);

    void* permanent_memory_block = VirtualAlloc(nullptr, PermanentMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    Assert(permanent_memory_block != nullptr, "Failed allocating permanent memory.");
    permanent_memory_blob_init(permanent_memory_block, PermanentMemorySize);
    
    WindowsWindow win = {};
    windows_create_window(&win, G_program_name, G_default_window_width, G_default_window_height);
    Renderer renderer = {};
    renderer.init(win.handle);

    while(win.state.closed == false)
    {
        windows_process_all_window_messsages();
    }
}