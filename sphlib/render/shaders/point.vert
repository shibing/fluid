#version 430

layout (location = 0) in vec4 position;
uniform mat4 matrix;

out vec4 v_position;

void main()
{
    gl_PointSize = 5.0;
    v_position = position;
    gl_Position = matrix * vec4(position.xyz, 1.0);
}
