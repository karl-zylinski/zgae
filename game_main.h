#pragma once

struct Renderer;
struct WindowState;

void game_start(WindowState* window, Renderer* renderer);
void game_update();
void game_do_frame();
void game_shutdown();