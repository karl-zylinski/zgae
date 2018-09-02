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
    --state.ship = spawn_ship(Vec3(0, 10, 0), nil)
    state.avatar = spawn_avatar(Vec3(0, -3, 0), nil)
    local box_geo = renderer.load_geometry_obj("tet.wobj")
    state.box1 = spawn_entity(Vec3(3, 0, 0), nil, box_geo)
    state.box1:set_collider(physics.create_mesh_collider("tet.wobj"))
    local box_geo = renderer.load_geometry_obj("tet.wobj")
    state.box2 = spawn_entity(Vec3(3,1.5, 0), Quat.from_axis_angle(Vec3(0, 0, 1), PI), box_geo)
    state.box2:set_collider(physics.create_mesh_collider("tet.wobj"))
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
        box2_move.z = time.dt()
    end

    if keyboard.is_held(Key.Down) then
        box2_move.z = -time.dt()
    end

    if keyboard.is_held(Key.G) then
        box2_move.y = time.dt()
    end

    if keyboard.is_held(Key.B) then
        box2_move.y = -time.dt()
    end

    local rot = Quat();
    if keyboard.is_held(Key.R) then
        rot = Quat.from_axis_angle(Vec3(0, 0, 1), -time.dt())
    
    end

    if keyboard.is_held(Key.T) then
        rot = Quat.from_axis_angle(Vec3(0, 0, 1), time.dt())
    end
    

    state.box2:move(box2_move)
    state.box2:set_rotation((state.box2.rotation*rot):normalized());
    state.camera_pos = state.avatar.position
    state.camera_rot = state.avatar.look_dir
    local solved_dist = physics.intersect_and_solve(state.box2.collider, state.box1.collider)
    local solved_dist_vec3 = Vec3(solved_dist.x, solved_dist.y, solved_dist.z)
    state.box2:move(solved_dist_vec3)
    table.foreach(world.entities, function(id, e) e:update() end)
    renderer.draw_world(world.render_world, state.camera_rot, state.camera_pos)
end

function shutdown()
    render_world.destroy(world.render_world)
end