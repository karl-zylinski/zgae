#pragma once

#include "key.h"

void keyboard_init();
void keyboard_pressed(Key key);
void keyboard_released(Key key);
void keyboard_end_of_frame();

bool key_is_held(Key key);
bool key_was_presssed(Key key);
bool key_was_released(Key key);
