#version 430

layout(location = 0) in int character;
out int vcharacter;
out int vposition;

void main()
{
    vcharacter = character;
    vposition = gl_VertexID;
    gl_Position = vec4(0, 0, 0, 1);
}
