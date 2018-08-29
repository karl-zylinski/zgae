local klass_id_counter = 1

function class(klass, super)
    if not klass then
        klass = {}
        klass.class_id = klass_id_counter
        klass_id_counter = klass_id_counter + 1

        local meta = {}
        meta.__call = function(self, ...)
            local object = {}
            setmetatable(object, klass)
            if object.init then object:init(...) end
            return object
        end
        setmetatable(klass, meta)
    end
    
    if super then
        for k,v in pairs(super) do
            klass[k] = v
        end
    end
    klass.__index = klass
    
    return klass
end

function is_class(obj, klass)
    return obj ~= nil and klass ~= nil and type(obj) == "table" and type(klass) == "table" and obj.class_id ~= nil and obj.class_id == klass.class_id
end

function is_type(obj, t)
    if type(t) == "string" then
        return type(obj) == t
    end

    return is_class(obj, t)
end