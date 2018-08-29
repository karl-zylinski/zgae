Entity = class(Entity)

local id_counter = 1

function Entity:init(rw, position, rotation, geometry, init_func, update_func)
    self.id = id_counter
    id_counter = id_counter + 1
    self.position = position or Vec3(0, 0, 0)
    self.rotation = rotation or Quat()
    self.update_func = update_func
    self.render_world = rw

    if geometry ~= nil and self.render_world ~= nil then
        self.render_object = render_object.create()
        render_object.set_geometry(self.render_object, geometry)
        render_object.set_position(self.render_object, self.position)
        render_object.set_rotation(self.render_object, self.rotation)
        render_world.add(self.render_world, self.render_object)
    end

    if init_func ~= nil then
        init_func(self)
    end
    world.entities[self.id] = self
end

function Entity:destroy()
    if self.render_object ~= nil then
        if self.render_world ~= nil then
            render_world.remove(self.render_world, self.render_object)
        end

        render_object.destroy(self.render_object)
        physics.destroy_collider(self.collider)
    end
    world.entities[self.id] = nil
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

    if self.collider ~= nil then
        physics.set_collider_position(self.collider, self.position)
    end
end

function Entity:move(delta)
    if not is_class(delta, Vec3) then
        error("Arg 1 nil (delta): Vec3 required")
    end

    self:set_position(self.position + delta)
end

function Entity:set_rotation(rotation)
    if rotation == nil then
        error("Arg 1 nil (rotation): Quat requried")
    end

    self.rotation = rotation

    if self.render_object ~= nil then
        render_object.set_rotation(self.render_object, self.rotation)
    end

    if self.collider ~= nil then
        physics.set_collider_rotation(self.collider, self.rotation)
    end
end

function Entity:set_collider(collider)
    if collider ~= self.collider and self.collider ~= nil then
        physics.destroy_collider(self.collider)
    end

    self.collider = collider

    if self.collider ~= nil then
        physics.set_collider_position(self.collider, self.position)
        physics.set_collider_rotation(self.collider, self.rotation)
    end
end

function Entity:intersects(other)
    if not is_class(other, Entity) then
        error("Entity:intersects, arg 1: Entity required")
    end

    if self.collider == nil or other.collider == nil then
        return false
    end

    return physics.intersect(self.collider, other.collider)
end

function Entity:update()
    if self.update_func == nil then
        return
    end

    self.update_func(self)
end
