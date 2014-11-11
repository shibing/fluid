#version 430

layout (location = 0) in vec4 position;
uniform mat4 matrix;

void main()
{
    gl_PointSize = 5.0;
    gl_Position = matrix * vec4(position.xyz, 1.0);
}
