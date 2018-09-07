require 'key'
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

function start()
    world.render_world = render_world_create()
    state.avatar = spawn_avatar(Vec3(0, -3, 0), Quat())
    local floor_geo = renderer_load_geometry_obj("floor.wobj")
    state.floor = Entity(Vec3(0, 0, -4), Quat(), nil, nil, floor_geo)
    state.floor:set_collider(physics_create_mesh_collider("floor.wobj"))
    local box_geo = renderer_load_geometry_obj("box.wobj")
    state.box1 = Entity(Vec3(0.2, 4.1, 4), Quat(), nil, nil, box_geo)
    state.box1:set_collider(physics_create_mesh_collider("box.wobj"))
    state.box1:create_rigid_body()
    local box_geo = renderer_load_geometry_obj("box.wobj")
    state.box2 = Entity(Vec3(0, 4, 0), Quat.from_axis_angle(Vec3(0, 0, 1), PI), nil, nil, box_geo)
    state.box2:set_collider(physics_create_mesh_collider("box.wobj"))
    state.box2:create_rigid_body(state.box2.handle)
end

function update()
    local box2_move = Vec3()
    if keyboard_is_held(Key.Left) then
        box2_move.x = -time_dt()
    end

    if keyboard_is_held(Key.Right) then
        box2_move.x = time_dt()
    end

    if keyboard_is_held(Key.Up) then
        box2_move.z = time_dt()
    end

    if keyboard_is_held(Key.Down) then
        box2_move.z = -time_dt()
    end

    if keyboard_is_held(Key.G) then
        box2_move.y = time_dt()
    end

    if keyboard_is_held(Key.B) then
        box2_move.y = -time_dt()
    end

    state.box2:move(box2_move)
    state.camera_pos = state.avatar:get_position()
    state.camera_rot = state.avatar.look_dir
    table.foreach(world.entities, function(i, e) e:update() end)
    renderer_draw_world(world.render_world, state.camera_rot, state.camera_pos)
end

function shutdown()
    render_world_destroy(world.render_world)
end