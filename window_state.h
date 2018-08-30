#pragma once

#include "key.h"

struct Vec2i;

typedef void(*WindowKeyPressedCallback)(Key key);
typedef void(*WindowKeyReleasedCallback)(Key key);
typedef void(*WindowMouseMovedCallback)(const Vec2i& delta);
typedef void(*WindowResizedCallback)(unsigned width, unsigned height);

struct WindowState
{
    bool closed;
    unsigned width;
    unsigned height;
    WindowKeyPressedCallback key_pressed_callback;
    WindowKeyReleasedCallback key_released_callback;
    WindowMouseMovedCallback mouse_moved_callback;
    WindowResizedCallback resized_callback;
};
