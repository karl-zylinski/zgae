#pragma once

fwd_struct(Camera);
fwd_struct(Vec3);
fwd_struct(Vec4);
fwd_enum(PrimitiveTopology);

void debug_set_camera(const Camera& c);
const Camera& debug_get_camera();
void debug_draw(const Vec3* vertices, u32 vertices_num, const Vec4* colors, PrimitiveTopology topology);
void print_vec3(const Vec3& v);