#pragma once

fwd_enum(Keycode);

void keyboard_init();
void keyboard_key_pressed(Keycode key);
void keyboard_key_released(Keycode key);
void keyboard_end_of_frame();
void keyboard_reset();

bool key_held(Keycode key);
bool key_went_down(Keycode key);
bool key_went_up(Keycode key);