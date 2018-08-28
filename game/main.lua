require 'key'
require 'entity'
require 'avatar'
require 'ship'

local state = {
    camera_rot = Quaternion(),
    camera_pos = Vector3(),
    ship = nil,
    props = {},
    avatar = nil
}

world = {
    render_world = nil,
    entities = {}
}

function spawn_entity(pos, rot, geometry, init_func, update_func)
    local e = Entity(world.render_world, pos, rot, geometry, init_func, update_func)
    table.insert(world.entities, e)
    return e
end

function start()
    world.render_world = render_world.create()
    local sph_geo = renderer.load_geometry_obj("sphere.wobj")
    local p1 = spawn_entity(Vector3(4, 1, 10), nil, sph_geo)
    local p2 = spawn_entity(Vector3(-7, 0, 21), nil, sph_geo)
    state.ship = spawn_ship(nil, nil)
    state.avatar = spawn_avatar(nil, nil)
end

function update()
    table.foreach(world.entities, function(i, e) e:update() end)
    state.camera_pos = state.avatar.position
    state.camera_rot = state.avatar.look_dir
end

function draw()
    renderer.draw_world(world.render_world, state.camera_rot, state.camera_pos)
end

function shutdown()
    render_world.destroy(world.render_world)
end