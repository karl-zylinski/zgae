constant_buffer = {
    binding = 0
    fields = [
        {
            name = "mvp"
            type = "mat4"
            value = "mat_model_view_projection"
        },
        {
            name = "model"
            type = "mat4"
            value = "mat_model"
        }
    ]
}

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
        name = "color"
        type = "vec4"
        value = "vertex_color"
    }
    {
        name = "texcoord"
        type = "vec2"
        value = "vertex_texcoord"
    }
]

type = "vertex"
source = "shader_default_vertex.spv"