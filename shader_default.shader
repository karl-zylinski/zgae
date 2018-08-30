constant_buffer = [
    {
        name = "model_view_projection"
        type = "mat4"
        value = "mat_mvp"
    }
    {
        name = "model"
        type = "mat4"
        value = "mat_model"
    }
    {
        name = "projection"
        type = "mat4"
        value = "mat_projection"
    }
]

input_layout = [
    {
        name = "position"
        type = "vec3"
        value = "vertex_position"
    }
    {
        name = "normal"
        type = "vec3"
        value = "vertex_normal"
    }
    {
        name = "uv"
        type = "vec2"
        value = "vertex_texcoord"
    }
    {
        name = "color"
        type = "vec4"
        value = "vertex_color"
    }
]

source_hlsl = "shader_default.hlsl"