require 'key'
require 'entity'
require 'avatar'
require 'ship'

local state = {
    camera_rot = Quaternion(),
    camera_pos = Vector3(),
    ship = nil,
    avatar = nil
}

world = {
    render_world = nil,
    entities = {}
}

function spawn_entity(pos, rot, geometry, init_func, update_func)
    return Entity(world.render_world, pos, rot, geometry, init_func, update_func)
end

function start()
    world.render_world = render_world.create()
    --local sph_geo = renderer.load_geometry_obj("sphere.wobj")
    --local p1 = spawn_entity(Vector3(4, 1, 10), nil, sph_geo)
    --local p2 = spawn_entity(Vector3(-7, 0, 21), nil, sph_geo)
    --state.ship = spawn_ship(nil, nil)
    state.avatar = spawn_avatar(nil, nil)
    local box_geo = renderer.load_geometry_obj("box.wobj")
    state.box1 = spawn_entity(Vector3(0, 0, 10), nil, box_geo)
    state.ps1 = physics.add_mesh("box.wobj", Vector3(0, 0, 10), Quaternion())
    state.ps2_pos = Vector3(-1, 0.5, 11)
    state.box2 = spawn_entity(state.ps2_pos, Quaternion(), box_geo)
    state.ps2 = physics.add_mesh("box.wobj", state.ps2_pos, Quaternion())
end

function update()
    local rot = Quaternion.from_axis_angle(Vector3(0, 1, 0), time.dt())
    state.box2:set_rotation((state.box2.rotation*rot):normalized());

    if keyboard.is_held(Key.Left) then
        state.ps2_pos.x = state.ps2_pos.x-time.dt()
    end

    if keyboard.is_held(Key.Right) then
        state.ps2_pos.x = state.ps2_pos.x+time.dt()
    end

    if keyboard.is_held(Key.Up) then
        state.ps2_pos.y = state.ps2_pos.y+time.dt()
    end

    if keyboard.is_held(Key.Down) then
        state.ps2_pos.y = state.ps2_pos.y-time.dt()
    end

    state.box2:set_position(state.ps2_pos)
    physics.set_shape_position(state.ps2, state.ps2_pos)
    physics.set_shape_rotation(state.ps2, state.box2.rotation)

    local lax = physics.intersect(state.ps1, state.ps2)
    print(lax == true)
    table.foreach(world.entities, function(id, e) e:update() end)
    state.camera_pos = state.avatar.position
    state.camera_rot = state.avatar.look_dir
end

function draw()
    renderer.draw_world(world.render_world, state.camera_rot, state.camera_pos)
end

function shutdown()
    render_world.destroy(world.render_world)
end