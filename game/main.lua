game_state = {
    camera_rot = Quaternion(),
    camera_pos = Vector3()
}

Key = enum(Key, "Unknown A B C D E F G H I J K L M N O P Q R S T U V W X Y Z Num0 Num1 Num2 Num3 Num4 Num5 Num6 Num7 Num8 Num9 Escape LControl LShift LAlt LSystem RControl RShift RAlt RSystem Menu LBracket RBracket SemiColon Comma Period Quote Slash BackSlash Tilde Equal Dash Space Return BackSpace Tab PageUp PageDown End Home Insert Delete Add Subtract Multiply Divide Left Right Up Down Numpad0 Numpad1 Numpad2 Numpad3 Numpad4 Numpad5 Numpad6 Numpad7 Numpad8 Numpad9 F1 F2 F3 F4 F5 F6 F7 F8 F9 F10 F11 F12 F13 F14 F15 Pause NumKeys", -1)
local render_world_handle

function start()
    local box_geo = renderer.load_geometry_obj("ship.wobj")
    render_world_handle = render_world.create()
    render_world.add(render_world_handle, box_geo)
end

function update()
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
        game_state.camera_rot:rotate_y(md.x * 0.001)
        game_state.camera_rot:rotate_x(md.y * 0.001)
    end

    game_state.camera_pos = game_state.camera_pos + game_state.camera_rot:transform_vec3(movement)
end

function draw()
    renderer.draw_world(render_world_handle, game_state.camera_rot, game_state.camera_pos)
end

function shutdown()
    render_world.destroy(render_world_handle)
end