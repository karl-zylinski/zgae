Entity = class(Entity)

function Entity:init(rw, position, rotation, geometry, init_func, update_func)
    self.position = position or Vector3(0, 0, 0)
    self.rotation = rotation or Quaternion()
    self.update_func = update_func
    self.render_world = rw

    if geometry ~= nil and rw ~= nil then
        self.render_object = render_object.create()
        render_object.set_geometry(self.render_object, geometry)
        render_object.set_position(self.render_object, self.position)
        render_world.add(self.render_world, self.render_object)
    end

    if init_func ~= nil then
        init_func(self)
    end
end

function Entity:set_geometry(geometry)
    if geometry == nil then
        render_object.destroy(self.render_object)
        return
    end

    if self.render_object == nil and self.render_world ~= nil then
        self.render_object = render_object.create()
        render_world.add(self.render_world, self.render_object)
    end

    render_object.set_geometry(self.render_object, geometry)
    render_object.set_position(self.render_object, self.position)
end

function Entity:set_position(position)
    if position == nil then
        error("Trying to set position to nil")
    end

    self.position = position

    if self.render_object ~= nil then
        render_object.set_position(self.render_object, self.position)
    end
end

function Entity:update()
    if self.update_func == nil then
        return
    end

    self.update_func(self)
end
