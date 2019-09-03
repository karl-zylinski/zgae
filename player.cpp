#include "player.h"
#include "entity.h"
#include "keyboard.h"
#include "keyboard_types.h"
#include "time.h"
#include "math.h"
#include "mouse.h"
#include "debug.h"

void player_update(Player* p)
{
    Vec3 d = {};
    float dt = time_dt();
    float walk_speed = 4.0f;

    if (key_held(KC_A))
        d.x -= 1;
    if (key_held(KC_D))
        d.x += 1;
    if (key_held(KC_W))
        d.y += 1;
    if (key_held(KC_S))
        d.y -= 1;

    if (len(d) != 0)
    {
        let rotated_d = rotate_vec3(p->camera.rot, d);
        rotated_d.z = 0;
        let normalized_d = normalize(rotated_d);
        let final_d = normalized_d * walk_speed * dt;
        p->entity.move(final_d);
    }

    if (key_went_down(KC_SPACE))
        p->entity.add_force({0, 0, 20});

    p->camera.pos = p->entity.get_position();
    let mouse_sens = dt * 0.01f;
    let diff = mouse_get_delta() * mouse_sens;
    p->yaw -= diff.x;
    p->pitch -= diff.y;
    p->pitch = clamp(p->pitch, -PI/3, PI/3);
    
    Quat yawq = quat_from_axis_angle(vec3_up, p->yaw);
    Vec3 local_sideways = rotate_vec3(yawq, {1, 0, 0});
    Quat pitchq = quat_from_axis_angle(local_sideways, p->pitch);
    p->camera.rot = pitchq * yawq;
}