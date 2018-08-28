Matrix4x4 = class(Matrix4x4)

function Matrix4x4:init()
    self.x = Vector4(1,0,0,0)
    self.y = Vector4(0,1,0,0)
    self.z = Vector4(0,0,1,0)
    self.w = Vector4(0,0,0,1)
end

function Matrix4x4.from_rotation_and_translation(q, t)
    local x = q.x
    local y = q.y
    local z = q.z
    local w = q.w
    local x2 = x + x
    local y2 = y + y
    local z2 = z + z
    local xx = x * x2
    local xy = x * y2
    local xz = x * z2
    local yy = y * y2
    local yz = y * z2
    local zz = z * z2
    local wx = w * x2
    local wy = w * y2
    local wz = w * z2

    local out = Matrix4x4()
    out.x.x = 1 - (yy + zz)
    out.x.y = xy + wz
    out.x.z = xz - wy
    out.x.w = 0
    out.y.x = xy - wz
    out.y.y = 1 - (xx + zz)
    out.y.z = yz + wx
    out.y.w = 0
    out.z.x = xz + wy
    out.z.y = yz - wx
    out.z.z = 1 - (xx + yy)
    out.z.w = 0
    out.w.x = t.x
    out.w.y = t.y
    out.w.z = t.z
    out.w.w = 1
    return out
end

Quaternion = class(Quaternion)

function Quaternion:init(x, y, z, w)
    if x ~= nil and y ~= nil and z ~= nil and w ~= nil then
        self.x = x
        self.y = y
        self.z = z
        self.w = w
    else
        self.x = 0
        self.y = 0
        self.z = 0
        self.w = 1
    end
end

function Quaternion:rotate_x(rads)
    local adjusted_rads = rads * 0.5;
    local bx = math.sin(adjusted_rads)
    local bw = math.cos(adjusted_rads)
    local old_x = self.x
    local old_y = self.y
    local old_z = self.z
    local old_w = self.w
    self.x = old_x * bw + old_w * bx
    self.y = old_y * bw + old_z * bx
    self.z = old_z * bw - old_y * bx
    self.w = old_w * bw - old_x * bx
end

function Quaternion:rotate_y(rads)
    local adjusted_rads = rads * 0.5;
    local by = math.sin(adjusted_rads)
    local bw = math.cos(adjusted_rads)
    local old_x = self.x
    local old_y = self.y
    local old_z = self.z
    local old_w = self.w
    self.x = old_x * bw - old_z * by
    self.y = old_y * bw + old_w * by
    self.z = old_z * bw + old_x * by
    self.w = old_w * bw - old_y * by
end

function Quaternion.from_axis_angle(axis, angle)
    if axis.x == 0 and axis.y == 0 and axis.z == 0 then
        return Quaternion()
    end

    local an = axis:normalized()
    local half_angle = angle * 0.5
    local s = math.sin(half_angle)
    return Quaternion(
        an.x * s,
        an.y * s,
        an.z * s,
        math.cos(half_angle)
    ):normalized()
end

function Quaternion:normalized()
    local len = math.sqrt(self.x * self.x + self.y * self.y + self.z * self.z + self.w * self.w);
    return Quaternion(
        self.x / len,
        self.y / len,
        self.z / len,
        self.w / len)
end

function Quaternion.from_euler(v)
    local cy = math.cos(v.y * 0.5);
    local sy = math.sin(v.y * 0.5);
    local cr = math.cos(v.z * 0.5);
    local sr = math.sin(v.z * 0.5);
    local cp = math.cos(v.x * 0.5);
    local sp = math.sin(v.x * 0.5);

    return Quaternion(
        cy * cr * cp + sy * sr * sp,
        cy * sr * cp - sy * cr * sp,
        cy * cr * sp + sy * sr * cp,
        sy * cr * cp - cy * sr * sp)
end


function Quaternion:transform_vec3(v)
    local qv = Vector3(self.x, self.y, self.z)
    local uv = qv:cross(v)
    local uuv = qv:cross(uv)
    return v + ((uv * self.w) + uuv) * 2.0;
end

function Quaternion:conjugate()
    return Quaternion(
        -self.x,
        -self.y,
        -self.z,
        self.w)
end

function Quaternion:__mul(o)
    return Quaternion(
        self.w * o.x + self.x * o.w + self.y * o.z - self.z * o.y,
        self.w * o.y - self.x * o.z + self.y * o.w + self.z * o.x,
        self.w * o.z + self.x * o.y - self.y * o.x + self.z * o.w,
        self.w * o.w - self.x * o.x - self.y * o.y - self.z * o.z)
end


Vector2 = class(Vector2)

function Vector2:init(x, y)
    self.x = x or 0
    self.y = y or 0
end

function Vector2:__add(o)
    return Vector2(self.x + o.x, self.y + o.y)
end

Vector3 = class(Vector3)

function Vector3:init(x, y, z)
    self.x = x or 0
    self.y = y or 0
    self.z = z or 0
end

function Vector3:__add(o)
    return Vector3(self.x + o.x, self.y + o.y, self.z + o.z)
end

function Vector3:__mul(o)
    return Vector3(self.x * o, self.y * o, self.z * o)
end

function Vector3:cross(o)
    return Vector3(
        self.y * o.z - self.z * o.y,
        self.z * o.x - self.x * o.z,
        self.x * o.y - self.y * o.x
    )
end

function Vector3:normalized()
    local len = math.sqrt(self.x*self.x + self.y*self.y + self.z*self.z)
    return Vector3(
        self.x / len,
        self.y / len,
        self.z / len
    )
end

Vector4 = class(Vector4)

function Vector4:init(x, y, z, w)
    self.x = x or 0
    self.y = y or 0
    self.z = z or 0
    self.w = w or 0
end