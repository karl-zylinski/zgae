local function ship_init(e)
    e.camera_anchor = Vector3(0, 3, -9)
end

local function ship_update(e)
    local movement = Vector3()
    local dt = time.dt()

    if keyboard.is_held(Key.Up) then
        movement.z = movement.z+dt
    end

    if keyboard.is_held(Key.Down) then
        movement.z = movement.z-dt
    end

    if keyboard.is_held(Key.Left) then
        movement.x = movement.x-dt
    end

    if keyboard.is_held(Key.Right) then
        movement.x = movement.x+dt
    end

    e:set_position(e.position + movement)
end

local ship_geo = nil
function spawn_ship(pos, rot)
    if ship_geo == nil then
        ship_geo = renderer.load_geometry_obj("ship.wobj")
    end

    return spawn_entity(pos, rot, ship_geo, ship_init, ship_update)
end
