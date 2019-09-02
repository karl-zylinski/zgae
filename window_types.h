#pragma once

struct GenericWindowInfo
{
    void* display;
    u64 handle;
};

enum WindowType : u32
{
    WINDOW_TYPE_X11
};
