#pragma once
#include "camera_types.h"
#include "entity_types.h"

struct Player
{
    Camera camera;
    Entity* entity;
};