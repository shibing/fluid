
out vec4 FragColor;
in vec2 gTexCoord;

uniform sampler2D Sampler;
uniform vec3 text_color;

void main()
{
    float A = texture(Sampler, gTexCoord).r;
    FragColor = vec4(text_color, A);
}
