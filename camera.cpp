#include "camera.h"

Camera camera_create()
{
    return {
        .pos = vec3_zero,
        .rot = quat_identity()
    };
}