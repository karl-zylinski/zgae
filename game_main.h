#pragma once

struct Renderer;

void game_start(Renderer* renderer);
void game_update(Renderer* renderer);
void game_draw(Renderer* renderer);
void game_shutdown(Renderer* renderer);