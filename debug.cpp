#include "debug.h"
#include "camera.h"
#include "log.h"
#include "math.h"
#include "renderer.h"

static Camera g_debug_cam;

void debug_set_camera(const Camera& c)
{
    g_debug_cam = c;
}

const Camera& debug_get_camera()
{
    return g_debug_cam;
}

void debug_draw(const Vec3* vertices, u32 vertices_num, const Vec4* colors, PrimitiveTopology topology)
{
    renderer_debug_draw(vertices, vertices_num, colors, topology, g_debug_cam.pos, g_debug_cam.rot);
}

void print_vec3(const Vec3& v)
{
    info("(%f, %f, %f)", v.x, v.y, v.z);
}