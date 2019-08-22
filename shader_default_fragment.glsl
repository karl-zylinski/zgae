#version 400
#extension GL_ARB_separate_shader_objects: enable
#extension GL_ARB_shading_language_420pack: enable

layout (location = 0) in vec4 in_pos;
layout (location = 1) in vec4 in_world_pos;
layout (location = 2) in vec3 in_normal;
layout (location = 3) in vec2 in_texcoord;
layout (location = 4) in vec4 in_color;

layout (location = 0) out vec4 out_color;

void main()
{
    vec3 sun = vec3(29, -10, 35);
    vec3 pos_to_sun = sun - in_world_pos.xyz;
    float l = max(dot(normalize(pos_to_sun), in_normal), 0.1f);
    out_color = vec4(in_color.rgb * l, 1);
}