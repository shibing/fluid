#version 430
out vec4 FragColor;
in vec2 gTexCoord;

uniform sampler2D sampler;
uniform vec3 text_color;

void main()
{
    float A = texture(sampler, gTexCoord).r;
    FragColor = vec4(text_color, A);
    //FragColor = vec4(gTexCoord, 0, 1);
}
