#pragma once

struct Renderer;

void game_start(Renderer* renderer);
void game_update(Renderer* renderer);
void game_do_frame(Renderer* renderer);
void game_shutdown(Renderer* renderer);