local function avatar_init(e)
    e.look_dir = Quat()
end

local function avatar_update(e)
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
    e.rotation = (e.rotation*yawq):normalized()
    local pitchq = Quat.from_axis_angle(Vec3(-pitch, 0, 0), dt*20)
    e.look_dir = (yawq*e.look_dir*pitchq):normalized()
    e.position = e.position + e.rotation:transform_vec3(movement)
end

function spawn_avatar(pos, rot)
    return spawn_entity(pos, rot, nil, avatar_init, avatar_update)
end