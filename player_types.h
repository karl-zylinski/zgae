#pragma once
#include "camera_types.h"
#include "entity_types.h"

struct Player
{
    Camera camera;
    f32 yaw;
    f32 pitch;
    EntityRef entity;
};