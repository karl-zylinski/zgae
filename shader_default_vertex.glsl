#version 400
#extension GL_ARB_separate_shader_objects: enable
#extension GL_ARB_shading_language_420pack:  enable

layout(push_constant) uniform Matrices
{
    mat4 mvp;
    mat4 model;
};

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec4 in_color;
layout (location = 3) in vec2 in_texcoord;

layout (location = 0) out vec4 out_pos;
layout (location = 1) out vec4 out_world_pos;
layout (location = 2) out vec3 out_normal;
layout (location = 3) out vec2 out_texcoord;
layout (location = 4) out vec4 out_color;

void main() {
    out_pos = mvp * vec4(in_pos, 1);
    out_world_pos = vec4(in_pos * mat3(model), 1);
    out_normal = normalize(mat3(model) * in_normal);
    out_texcoord = in_texcoord;
    out_color = in_color;

    gl_Position = out_pos;
}
