#pragma once
#include "entity_types.h"

Entity entity_create(
    World* w,
    const Vec3& pos,
    const Quat& rot);
