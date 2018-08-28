state = {
    camera_rot = Quaternion(),
    camera_pos = Vector3(),
    ship = nil,
    render_world = nil
}

Ship = class(Ship)

function Ship:init()
    self.position = Vector3()
    self.render_obj = render_object.create()
    local box_geo = renderer.load_geometry_obj("ship.wobj")
    render_object.set_geometry(self.render_obj, box_geo)
    render_world.add(state.render_world, self.render_obj)
end

function Ship:update()
    local movement = Vector3()

    if keyboard.is_held(Key.Up) then
        movement.z = movement.z+0.0005
    end

    if keyboard.is_held(Key.Down) then
        movement.z = movement.z-0.0005
    end

    if keyboard.is_held(Key.Left) then
        movement.x = movement.z-0.0005
    end

    if keyboard.is_held(Key.Right) then
        movement.x = movement.x+0.0005
    end

    self.position = self.position + movement
    render_object.set_position(self.render_obj, self.position)
end

Key = enum(Key, "Unknown A B C D E F G H I J K L M N O P Q R S T U V W X Y Z Num0 Num1 Num2 Num3 Num4 Num5 Num6 Num7 Num8 Num9 Escape LControl LShift LAlt LSystem RControl RShift RAlt RSystem Menu LBracket RBracket SemiColon Comma Period Quote Slash BackSlash Tilde Equal Dash Space Return BackSpace Tab PageUp PageDown End Home Insert Delete Add Subtract Multiply Divide Left Right Up Down Numpad0 Numpad1 Numpad2 Numpad3 Numpad4 Numpad5 Numpad6 Numpad7 Numpad8 Numpad9 F1 F2 F3 F4 F5 F6 F7 F8 F9 F10 F11 F12 F13 F14 F15 Pause NumKeys", -1)
local render_world_handle

function start()
    state.render_world = render_world.create()
    state.ship = Ship()
end

function update()
    state.ship:update()
    local movement = Vector3()

    if keyboard.is_held(Key.W) then
        movement.z = movement.z+0.0005
    end

    if keyboard.is_held(Key.A) then
        movement.x = movement.x-0.0005
    end

    if keyboard.is_held(Key.S) then
        movement.z = movement.z-0.0005
    end

    if keyboard.is_held(Key.D) then
        movement.x = movement.x+0.0005
    end

    local md = mouse.get_delta()

    if md.x ~= 0 or md.y ~= 0 then
        state.camera_rot:rotate_y(md.x * 0.001)
        state.camera_rot:rotate_x(md.y * 0.001)
    end

    state.camera_pos = state.camera_pos + state.camera_rot:transform_vec3(movement)
end

function draw()
    renderer.draw_world(state.render_world, state.camera_rot, state.camera_pos)
end

function shutdown()
    render_world.destroy(state.render_world)
end