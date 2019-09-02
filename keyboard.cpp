#include "keyboard.h"
#include "keyboard_types.h"
#include "memory.h"

static bool keys_held[(u32)KC_NUM];
static bool keys_went_down[(u32)KC_NUM];
static bool keys_went_up[(u32)KC_NUM];

void keyboard_init()
{
    keyboard_reset();
}

void keyboard_key_pressed(Keycode key)
{
    if (key == KC_UNKNOWN)
        return;

    keys_went_down[(u32)key] = true;
    keys_held[(u32)key] = true;
}

void keyboard_key_released(Keycode key)
{
    if (key == KC_UNKNOWN)
        return;

    keys_went_up[(u32)key] = true;
    keys_held[(u32)key] = false;
}

void keyboard_end_of_frame()
{
    memzero(keys_went_down, sizeof(keys_went_down));
    memzero(keys_went_up, sizeof(keys_went_up));
}

void keyboard_reset()
{
    memzero(keys_held, sizeof(keys_held));
    memzero(keys_went_down, sizeof(keys_went_down));
    memzero(keys_went_up, sizeof(keys_went_up));
}

bool key_held(Keycode key)
{
    return keys_held[(u32)key];
}

bool key_went_down(Keycode key)
{
    return keys_went_down[(u32)key];
}

bool key_went_up(Keycode key)
{
    return keys_went_up[(u32)key];
}