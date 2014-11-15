
#version 430

in vec2 tex_coord0;
out vec4 frag_color;


uniform sampler2D sampler;
uniform mat4 projection_mat;

//restore depth

void main()
{
   float z  = vec4(texture(sampler, tex_coord0).r); 
   vec4 pos = vec4(0, 0, z, 1);
   pos = projection_mat * pos;
   z = pos.z / pos.w;
   frag_color = vec4((z + 1) * 0.5);
}   
