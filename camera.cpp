#include "camera.h"
#include <math.h>

Matrix4x4 camera_calc_view_matrix(const Camera& c)
{
    return matrix4x4_inverse(matrix4x4_from_rotation_and_translation(c.rotation, c.position));
}

Camera camera_create_projection()
{
    float near_plane = 0.01f;
    float far_plane = 1000.0f;
    float fov = 90.0f;
    float aspect = 1.0f;
    float y_scale = 1.0f / tanf((3.14f / 180.0f) * fov / 2);
    float x_scale = y_scale / aspect;

    Camera c = {};
    c.projection_matrix = {
        x_scale, 0, 0, 0,
        0, y_scale, 0, 0,
        0, 0, far_plane/(far_plane-near_plane), 1,
        0, 0, (-far_plane * near_plane) / (far_plane - near_plane), 0 
    };
    c.rotation = quaternion_identity();
    return c;
}