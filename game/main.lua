local render_world_handle

function start()
    local box_geo = renderer.load_geometry_obj("box.wobj")
    render_world_handle = render_world.create()
    render_world.add(render_world_handle, box_geo)
end

function update()
end

function draw()
    renderer.draw_world(render_world_handle)
end

function shutdown()
    render_world.destroy(render_world_handle)
end