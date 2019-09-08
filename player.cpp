#include "player.h"
#include "keyboard.h"
#include "time.h"
#include "mouse.h"
#include "debug.h"
#include "log.h"

void Player::update()
{
    Vec3 d = {};
    float dt = time_dt();
    float walk_speed = 4.0f;

    if (key_held(KEY_A))
        d.x -= 1;
    if (key_held(KEY_D))
        d.x += 1;
    if (key_held(KEY_W))
        d.y += 1;
    if (key_held(KEY_S))
        d.y -= 1;

    if (len(d) != 0)
    {
        let rotated_d = rotate_vec3(this->camera.rot, d);
        rotated_d.z = 0;
        let normalized_d = normalize(rotated_d);
        let final_d = normalized_d * walk_speed * dt;
        this->entity.move(final_d);
    }

    if (key_went_down(KEY_SPACE))
        this->entity.add_force({0, 0, 20});

    this->camera.pos = this->entity.get_position();
    let mouse_sens = dt * 0.01f;
    let diff = mouse_get_delta() * mouse_sens;
    this->yaw -= diff.x;
    this->pitch -= diff.y;
    this->pitch = clamp(this->pitch, -PI/3, PI/3);
    
    Quat yawq = quat_from_axis_angle(vec3_up, this->yaw);
    Vec3 local_sideways = rotate_vec3(yawq, {1, 0, 0});
    Quat pitchq = quat_from_axis_angle(local_sideways, this->pitch);
    this->camera.rot = pitchq * yawq;
    debug_set_camera(this->camera);
}