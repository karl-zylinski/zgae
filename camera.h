#pragma once

#include "math.h"

struct Camera
{
    Matrix4x4 projection_matrix;
    Quaternion rotation;
    Vector3 position;
};

Matrix4x4 camera_calc_view_matrix(const Camera& c);
Camera camera_create_projection();
Camera camera_create_uv_rendering();