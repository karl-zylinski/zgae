#pragma once
#include "window_state.h"

struct WindowsWindow
{
    void* handle;
    WindowState state;
};

void windows_create_window(WindowsWindow* w, const char* title, unsigned width, unsigned height);
void windows_process_all_window_messsages();
