#include "debug.h"
#include "camera.h"

static Camera g_debug_cam;

void debug_set_camera(const Camera& c)
{
    g_debug_cam = c;
}

const Camera& debug_get_camera()
{
    return g_debug_cam;
}