#include "camera_first_person.h"
#include "camera.h"
#include "math.h"
#include "mouse.h"
#include "time.h"

void camera_first_person_update(Camera* c)
{
    let speed = time_dt() * 0.1f;
    let md = mouse_get_delta();
    Vec3 local_sideways = quat_transform_vec3(c->rot, {1, 0, 0});
    Quat yaw = quat_from_axis_angle(vec3_up, -md.x * speed);
    Quat pitch = quat_from_axis_angle(local_sideways, -md.y * speed);
    c->rot = yaw * pitch * c->rot;
}
