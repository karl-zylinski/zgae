#include "keyboard.h"

struct Keyboard
{
    bool held[Key::NumKeys];
    bool pressed[Key::NumKeys];
    bool released[Key::NumKeys];
};

static Keyboard keyboard_state;

void keyboard_init()
{
    memzero(&keyboard_state, sizeof(Keyboard));
}

void keyboard_pressed(Key key)
{
    keyboard_state.pressed[(unsigned)key] = true;
    keyboard_state.held[(unsigned)key] = true;
}

void keyboard_released(Key key)
{
    keyboard_state.released[(unsigned)key] = true;
    keyboard_state.held[(unsigned)key] = false;
}

void keyboard_end_of_frame()
{
    memset(keyboard_state.pressed, 0, sizeof(bool) * (unsigned)Key::NumKeys);
    memset(keyboard_state.released, 0, sizeof(bool) * (unsigned)Key::NumKeys);
}

bool key_is_held(Key key)
{
    return keyboard_state.held[(unsigned)key];
}

bool key_was_presssed(Key key)
{
    return keyboard_state.pressed[(unsigned)key];
}

bool key_was_released(Key key)
{
    return keyboard_state.released[(unsigned)key];
}