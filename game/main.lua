require 'key'

state = {
    camera_rot = Quaternion(),
    camera_pos = Vector3(),
    ship = nil,
    props = {},
    render_world = nil,
    avatar = nil
}

StaticProp = class(StaticProp)

function StaticProp:init(geometry, position)
    self.render_obj = render_object.create()
    render_object.set_geometry(self.render_obj, geometry)
    render_object.set_position(self.render_obj, position)
    render_world.add(state.render_world, self.render_obj)
end

Ship = class(Ship)

function Ship:init()
    self.position = Vector3()
    self.render_obj = render_object.create()
    local box_geo = renderer.load_geometry_obj("ship.wobj")
    render_object.set_geometry(self.render_obj, box_geo)
    render_world.add(state.render_world, self.render_obj)
    self.camera_anchor = Vector3(0, 3, -9)
end

function Ship:update()
    local movement = Vector3()

    if keyboard.is_held(Key.W) then
        movement.z = movement.z+0.0005
    end

    if keyboard.is_held(Key.S) then
        movement.z = movement.z-0.0005
    end

    if keyboard.is_held(Key.A) then
        movement.x = movement.x-0.0005
    end

    if keyboard.is_held(Key.D) then
        movement.x = movement.x+0.0005
    end

    self.position = self.position + movement
    render_object.set_position(self.render_obj, self.position)
    state.camera_pos = self.position + self.camera_anchor
end

Avatar = class(Avatar)

function Avatar:init()
    self.position = Vector3()
    self.rotation = Quaternion()
    self.look_dir = Quaternion()
end

function Avatar:update()
    local movement = Vector3()

    if keyboard.is_held(Key.W) then
        movement.z = movement.z+0.0005
    end

    if keyboard.is_held(Key.S) then
        movement.z = movement.z-0.0005
    end

    if keyboard.is_held(Key.A) then
        movement.x = movement.x-0.0005
    end

    if keyboard.is_held(Key.D) then
        movement.x = movement.x+0.0005
    end

    local md = mouse.get_delta()
    local yaw = md.x
    local pitch = md.y
    local yawq = Quaternion.from_axis_angle(Vector3(0, yaw, 0), 0.005)
    self.rotation = (self.rotation*yawq):normalized()
    local pitchq = Quaternion.from_axis_angle(Vector3(pitch, 0, 0), 0.005)    
    self.look_dir = (yawq*self.look_dir*pitchq):normalized()
    state.camera_rot = self.look_dir
    self.position = self.position + self.look_dir:transform_vec3(movement)
    state.camera_pos = self.position
end

local render_world_handle

function start()
    state.render_world = render_world.create()
    local sph_geo = renderer.load_geometry_obj("sphere.wobj")
    local p1 = StaticProp(sph_geo, Vector3(4, 1, 10))
    local p2 = StaticProp(sph_geo, Vector3(-7, 0, 21))
    table.insert(state.props, p1)
    table.insert(state.props, p2)
    state.ship = Ship()
    state.avatar = Avatar()
end

function update()
    --state.ship:update()
    state.avatar:update()
end

function draw()
    renderer.draw_world(state.render_world, state.camera_rot, state.camera_pos)
end

function shutdown()
    render_world.destroy(state.render_world)
end