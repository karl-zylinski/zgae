#include "keyboard.h"
#include "keycode_types.h"
#include "memory.h"
#include "debug.h"

static bool keys_held[KC_NUM];
static bool keys_went_down[KC_NUM];
static bool keys_went_up[KC_NUM];

void keyboard_init()
{
    keyboard_reset();
}

void keyboard_key_pressed(keycode_t key)
{
    if (key == KC_UNKNOWN)
        return;

    keys_went_down[key] = true;
    keys_held[key] = true;
}

void keyboard_key_released(keycode_t key)
{
    if (key == KC_UNKNOWN)
        return;

    keys_went_down[key] = true;
    keys_held[key] = false;
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

bool key_is_held(keycode_t key)
{
    return keys_held[key];
}

bool key_went_down(keycode_t key)
{
    return keys_went_down[key];
}

bool key_went_up(keycode_t key)
{
    return keys_went_up[key];
}