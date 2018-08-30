constant_buffer = [
    {
        name = "view_projection"
        type = "mat4"
        value = "mat_view_projection"
    }
    {
        name = "color"
        type = "vec4"
    }
]

input_layout = [
    {
        name = "position"
        type = "vec3"
        value = "vertex_position"
    }
]

source_hlsl = "debug_draw.hlsl"