#version 400
#extension GL_ARB_separate_shader_objects: enable
#extension GL_ARB_shading_language_420pack:  enable

layout(push_constant) uniform VertexPushConstant
{
    mat4 view_projection;
    vec4 color;
};

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec4 in_color;
layout (location = 0) out vec4 out_pos;
layout (location = 1) out vec4 out_color;

void main()
{
    out_pos = view_projection * vec4(in_pos, 1);
    out_color = in_color;
    gl_Position = out_pos;
}
