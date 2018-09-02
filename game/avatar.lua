local function avatar_init(state, handle)
    state.look_dir = Quat()
end

local function avatar_update(state, handle)
    local movement = Vec3()
    local dt = time.dt()

    if keyboard.is_held(Key.W) then
        movement.y = movement.y+dt*10
    end

    if keyboard.is_held(Key.S) then
        movement.y = movement.y-dt*10
    end

    if keyboard.is_held(Key.A) then
        movement.x = movement.x-dt*10
    end

    if keyboard.is_held(Key.D) then
        movement.x = movement.x+dt*10
    end

    local md = mouse.get_delta()
    local yaw = md.x
    local pitch = md.y
    local yawq = Quat.from_axis_angle(Vec3(0, 0, -yaw), dt*20)
    local r = entity.get_rotation(handle)
    local old_rot = Quat(r.x, r.y, r.z, r.w)
    local new_rot = (old_rot*yawq):normalized()
    entity.set_rotation(handle, new_rot)
    local pitchq = Quat.from_axis_angle(Vec3(-pitch, 0, 0), dt*20)
    state.look_dir = (yawq*state.look_dir*pitchq):normalized()
    local p = entity.get_position(handle)
    entity.set_position(handle, Vec3(p.x, p.y, p.z) + new_rot:transform_vec3(movement))
end

function spawn_avatar(pos, rot)
    return spawn_entity(pos, rot, nil, avatar_init, avatar_update)
end