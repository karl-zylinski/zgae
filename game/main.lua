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
    --state.ship = spawn_ship(nil, nil)
    state.avatar = spawn_avatar(Vec3(0, 0, -5), nil)
    local box_geo = renderer.load_geometry_obj("box.wobj")
    state.box1 = spawn_entity(Vec3(3, 0, 0), nil, box_geo)
    state.box1:set_collider(physics.create_mesh_collider("box.wobj"))
    state.box2 = spawn_entity(Vec3(1.7, 2.1, 0), Quat.from_axis_angle(Vec3(0, 1, 0), 0.000001), box_geo)
    state.box2:set_collider(physics.create_mesh_collider("box.wobj"))
end

function update()
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


    if keyboard.is_held(Key.G) then
        box2_move.z = time.dt()
    end

    if keyboard.is_held(Key.B) then
        box2_move.z = -time.dt()
    end

    local rot = Quat();
    if keyboard.is_held(Key.R) then
        rot = Quat.from_axis_angle(Vec3(0, 1, 0), time.dt())
    
    end

    if keyboard.is_held(Key.T) then
        rot = Quat.from_axis_angle(Vec3(0, 1, 0), time.dt())
    end
    

    state.box2:move(box2_move)
    state.box2:set_rotation((state.box2.rotation*rot):normalized());
    state.camera_pos = state.avatar.position
    state.camera_rot = state.avatar.look_dir
    --renderer.draw_world(world.render_world, state.camera_rot, state.camera_pos)
    local solved_dist = physics.intersect_and_solve(state.box2.collider, state.box1.collider)
    local solved_dist_vec3 = Vec3(solved_dist.x, solved_dist.y, solved_dist.z)
    state.box2:move(solved_dist_vec3)
    table.foreach(world.entities, function(id, e) e:update() end)
end

function draw()
    renderer.draw_world(world.render_world, state.camera_rot, state.camera_pos)
end

function shutdown()
    render_world.destroy(world.render_world)
end