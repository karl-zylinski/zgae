#include "player.h"
#include "camera_first_person.h"
#include "entity.h"
#include "keyboard.h"
#include "keyboard_types.h"
#include "time.h"
#include "math.h"

void player_update(Player* p)
{
    Vec3 d = {};
    float s = 4.0f;

    if (key_held(KC_A))
        d.x -= time_dt() * s;
    if (key_held(KC_D))
        d.x += time_dt() * s;
    if (key_held(KC_W))
        d.y += time_dt() * s;
    if (key_held(KC_S))
        d.y -= time_dt() * s;
    if (key_held(KC_R))
        d.z += time_dt() * s;
    if (key_held(KC_F))
        d.z -= time_dt() * s;

    Vec3 rotated_d = quat_transform_vec3(p->camera.rot, d);
    entity_move(&p->entity, rotated_d);
    p->camera.pos = entity_get_position(p->entity);
    camera_first_person_update(&p->camera);
}