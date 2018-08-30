require 'key'
require 'entity'
require 'avatar'
require 'ship'

local state = {
    camera_rot = Quat(),
    camera_pos = Vec3(),
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
    --local p1 = spawn_entity(Vec3(4, 1, 10), nil, sph_geo)
    --local p2 = spawn_entity(Vec3(-7, 0, 21), nil, sph_geo)
    ---state.ship = spawn_ship(nil, nil)
    state.avatar = spawn_avatar(Vec3(0, 0, -10), nil)
    --[[local box_geo = renderer.load_geometry_obj("box.wobj")
    state.box1 = spawn_entity(Vec3(0, 0, 10), nil, box_geo)
    state.box1:set_collider(physics.create_mesh_collider("box.wobj"))
    state.box2 = spawn_entity(Vec3(-1, 0.5, 11), nil, box_geo)
    state.box2:set_collider(physics.create_mesh_collider("box.wobj"))]]
end

function update()
--[[    local rot = Quat.from_axis_angle(Vec3(0, 1, 0), time.dt())
    state.box2:set_rotation((state.box2.rotation*rot):normalized());

    local box2_move = Vec3()
    if keyboard.is_held(Key.Left) then
        box2_move.x = -time.dt()
    end

    if keyboard.is_held(Key.Right) then
        box2_move.x = time.dt()
    end

    if keyboard.is_held(Key.Up) then
        box2_move.y = time.dt()
    end

    if keyboard.is_held(Key.Down) then
        box2_move.y = -time.dt()
    end

    state.box2:move(box2_move)]]
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