#version 430

layout(location = 0) in vec3 position;
out vec3 tex_coords;

uniform mat4 mat;
void main()
{
    gl_Position = mat * (vec4((20 * position), 1.0));
    gl_Position.z = gl_Position.w;
    tex_coords = position;
}

