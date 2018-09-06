Entity = class(Entity)

function Entity:init(position, rotation, init_func, update_func, geometry)
    self.handle = entity_create(position or Vec3(), rotation or Quat(), geometry)
    self.update_func = update_func

    if init_func ~= nil then
        init_func(self)
    end

    if geometry ~= nil then
        self:set_geometry(geometry)
    end
    table.insert(world.entities, self)
end

function Entity:update()
    if self.update_func == nil then
        return
    end

    self.update_func(self)
end

function Entity:set_geometry(geometry)
    if geometry == nil then
        render_world.remove(world.render_world, self.handle)
    end

    render_world.add(world.render_world, self.handle)
    entity_set_geometry(self.handle, geometry);
end

function Entity:set_position(position)
    entity_set_position(self.handle, position)
end

function Entity:get_position()
    return entity_get_position(self.handle)
end

function Entity:set_rotation(rotation)
    entity_set_rotation(self.handle, rotation)
end

function Entity:get_rotation()
    return entity_get_rotation(self.handle)
end

function Entity:move(delta)
    entity_set_position(self.handle, entity_get_position(self.handle) + delta)
end

function Entity:set_collider(collider)
    entity_set_collider(self.handle, collider)
end

function Entity:get_collider(collider)
    return entity_get_collider(self.handle)
end

function Entity:create_rigid_body()
    physics.create_rigid_body(self.handle)
end

function Entity:destroy_rigid_body()
    physics.destroy_rigid_body(self.handle)
end
