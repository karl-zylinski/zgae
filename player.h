#pragma once
#include "camera.h"
#include "entity.h"

struct Player
{
    void update();

    Camera camera;
    f32 yaw;
    f32 pitch;
    Entity entity;
};