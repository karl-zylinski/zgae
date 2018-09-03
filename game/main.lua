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

function spawn_entity(pos, rot, geometry, init_func, update_func)
    local entity_handle = entity.create(pos, rot, geometry)
    local e = {
        handle = entity_handle,
        update_func = update_func
    }

    if init_func ~= nil then
        init_func(e, entity_handle)
    end

    world.entities[entity_handle] = e
    render_world.add(world.render_world, entity_handle)
    return e
end

function start()
    world.render_world = render_world.create()
    --state.ship = spawn_ship(Vec3(0, 10, 0), nil)
    state.avatar = spawn_avatar(Vec3(0, -3, 0), Quat())
    local floor_geo = renderer.load_geometry_obj("floor.wobj")
    state.floor = spawn_entity(Vec3(0, 0, -4), Quat(), floor_geo)
    entity.set_collider(state.floor.handle, physics.create_mesh_collider("floor.wobj"))
    local box_geo = renderer.load_geometry_obj("box.wobj")
    state.box1 = spawn_entity(Vec3(0.2, 4.1, 4), Quat(), box_geo)
    entity.set_collider(state.box1.handle, physics.create_mesh_collider("box.wobj"))
    physics.create_rigid_body(state.box1.handle)
    local box_geo = renderer.load_geometry_obj("box.wobj")
    state.box2 = spawn_entity(Vec3(0, 4, 0), Quat.from_axis_angle(Vec3(0, 0, 1), PI), box_geo)
    entity.set_collider(state.box2.handle, physics.create_mesh_collider("box.wobj"))
    physics.create_rigid_body(state.box2.handle)
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

    entity.move(state.box2.handle, box2_move)
    state.camera_pos = entity.get_position(state.avatar.handle)
    state.camera_rot = state.avatar.look_dir
    table.foreach(world.entities, function(h, e) 
        if e.update_func ~= nil then
            e.update_func(e, h)
        end
    end)
    renderer.draw_world(world.render_world, state.camera_rot, state.camera_pos)
end

function shutdown()
    render_world.destroy(world.render_world)
end