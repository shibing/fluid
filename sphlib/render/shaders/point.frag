#version 430

in vec4 v_position;

smooth out vec4 color;
void main()
{
    vec2 pos = gl_PointCoord.xy * 2 - 1;
    if(length(pos) > 1.0)
        discard;
    color = (v_position + vec4(4, 2.5, 2, 1.0)) / vec4(8, 5, 4, 2);
}

