#include "mouse.h"
#include "math.h"

struct Mouse
{
    Vector2i delta;
};

static Mouse mouse_state;

void mouse_init()
{
    memzero(&mouse_state, Mouse);
}

void mouse_add_delta(const Vector2i& delta)
{
    mouse_state.delta += delta;
}

void mouse_end_of_frame()
{
    mouse_state.delta = {0, 0};
}

const Vector2i& mouse_movement_delta()
{
    return mouse_state.delta;
}
