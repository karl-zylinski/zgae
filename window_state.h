#pragma once

#include "key.h"

struct Vector2i;

typedef void(*WindowKeyPressedCallback)(Key key);
typedef void(*WindowKeyReleasedCallback)(Key key);
typedef void(*WindowMouseMovedCalledback)(const Vector2i& delta);

struct WindowState
{
    bool closed;
    WindowKeyPressedCallback key_pressed_callback;
    WindowKeyReleasedCallback key_released_callback;
    WindowMouseMovedCalledback mouse_moved_callback;
};
