#include "camera.h"
#include "math.h"

Camera create_camera()
{
    return {
        .pos = vec3_zero,
        .rot = quat_identity()
    };
}