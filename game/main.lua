Quaternion = class(Quaternion)

function Quaternion:init()
    self.x = 0
    self.y = 0
    self.z = 0
    self.w = 1
end

Vector3 = class(Vector3)

function Vector3:init(x, y, z)
    self.x = x or 0
    self.y = y or 0
    self.z = z or 0
end

function Vector3:__add(vec3)
    return Vector3(self.x + vec3.x, self.y + vec3.y, self.z + vec3.z)
end

function str_split(str, del)
    res = {}
    cur = ""
    for i = 1, #str do
        local c = str:sub(i,i)
        if c == del then
            table.insert(res, cur)
            cur = ""
        else
            cur = cur .. c
        end
    end

    if cur ~= "" then
        table.insert(res, cur)
    end

    return res
end

function enum(e, str_vals, offset)
    e = {}
    vals = str_split(str_vals, " ")
    for i,v in ipairs(vals) do
        e[v] = i + (offset or 0)
    end
    return e
end

Key = enum(Key, "Unknown A B C D E F G H I J K L M N O P Q R S T U V W X Y Z", -1)

game_state = {
    camera_rot = Quaternion(),
    camera_pos = Vector3()
}

local render_world_handle

function start()
    local box_geo = renderer.load_geometry_obj("ship.wobj")
    render_world_handle = render_world.create()
    render_world.add(render_world_handle, box_geo)
end

function update()
    local movement = Vector3()

    if keyboard.is_held(Key.W) then
        movement.z = movement.z+0.001
    end

    if keyboard.is_held(Key.A) then
        movement.x = movement.x-0.001
    end

    if keyboard.is_held(Key.S) then
        movement.z = movement.z-0.001
    end

    if keyboard.is_held(Key.D) then
        movement.x = movement.x+0.001
    end

    game_state.camera_pos = game_state.camera_pos + movement
end

function draw()
    renderer.draw_world(render_world_handle, game_state.camera_rot, game_state.camera_pos)
end

function shutdown()
    render_world.destroy(render_world_handle)
end