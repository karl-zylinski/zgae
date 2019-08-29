#pragma once

fwd_enum(KeyCode);

void keyboard_init();
void keyboard_key_pressed(KeyCode key);
void keyboard_key_released(KeyCode key);
void keyboard_end_of_frame();
void keyboard_reset();

bool key_held(KeyCode key);
bool key_went_down(KeyCode key);
bool key_went_up(KeyCode key);