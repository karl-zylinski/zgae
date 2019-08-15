constant_buffer = [
    {
        name = "mvp"
        type = "mat4"
        value = "mat_model_view_projection"
    }
]

input_layout = [
    {
        name = "position"
        type = "vec3"
        value = "vertex_position"
    }
    {
        name = "color"
        type = "vec4"
        value = "vertex_color"
    }
]

type = "vertex"
source = "shader_default_vertex.spv"