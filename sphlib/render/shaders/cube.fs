#version 430

in vec3 tex_coords;
out vec4 color;

uniform samplerCube sampler;

void main()
{
    color = texture(sampler, tex_coords);
}
