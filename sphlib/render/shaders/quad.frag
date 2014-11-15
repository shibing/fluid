#version 430

in vec2 tex_coord0;
out vec4 frag_color;


uniform sampler2D sampler;

void main()
{
   frag_color = vec4(texture(sampler, tex_coord0).r); 
}
