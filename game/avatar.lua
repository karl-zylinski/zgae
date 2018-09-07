require 'entity'

local function avatar_init(e)
    e.look_dir = Quat()
end

local function avatar_update(e)
    local movement = Vec3()
    local dt = time_dt()

    if keyboard_is_held(Key.W) then
        movement.y = movement.y+dt*10
    end

    if keyboard_is_held(Key.S) then
        movement.y = movement.y-dt*10
    end

    if keyboard_is_held(Key.A) then
        movement.x = movement.x-dt*10
    end

    if keyboard_is_held(Key.D) then
        movement.x = movement.x+dt*10
    end

    local md = mouse_get_delta()
    local yaw = md.x
    local pitch = md.y
    local yawq = Quat.from_axis_angle(Vec3(0, 0, -yaw), dt*20)
    local r = e:get_rotation()
    local old_rot = Quat(r.x, r.y, r.z, r.w)
    local new_rot = (old_rot*yawq):normalized()
    e:set_rotation(new_rot)
    local pitchq = Quat.from_axis_angle(Vec3(-pitch, 0, 0), dt*20)
    e.look_dir = (yawq*e.look_dir*pitchq):normalized()
    local p = e:get_position()
    e:set_position(Vec3(p.x, p.y, p.z) + new_rot:transform_vec3(movement))
end

function spawn_avatar(pos, rot)
    return Entity(pos, rot, avatar_init, avatar_update, nil)
end