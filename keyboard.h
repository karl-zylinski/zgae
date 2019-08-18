#pragma once

fwd_enum(keycode_t);

void keyboard_init();
void keyboard_key_pressed(keycode_t key);
void keyboard_key_released(keycode_t key);
void keyboard_end_of_frame();
void keyboard_reset();

bool key_is_held(keycode_t key);
bool key_went_down(keycode_t key);
bool key_went_up(keycode_t key);