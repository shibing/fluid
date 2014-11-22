#version 430

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 color;
uniform mat4 matrix;

out vec4 v_position;
out vec4 v_color;

void main()
{
    gl_PointSize = 5.0;
    v_position = position;
    v_color = color;
    gl_Position = matrix * vec4(position.xyz, 1.0);
}
