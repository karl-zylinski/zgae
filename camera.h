#pragma once
#include "math.h"

struct Camera
{
    Vec3 pos;
    Quat rot;
};

Camera camera_create();