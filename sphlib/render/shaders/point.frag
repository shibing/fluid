#version 430

in vec4 v_position;
in vec4 v_color;

smooth out vec4 color;
void main()
{
    vec2 pos = gl_PointCoord.xy * 2 - 1;
    if(length(pos) > 1.0)
        discard;
    color = v_color;
}

