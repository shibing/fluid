#version 430

in vec4 v_position;
in vec4 v_color;

smooth out vec4 color;
void main()
{
    if(v_color.w > 0.5)
        discard;
    vec2 pos = gl_PointCoord.xy * 2 - 1;
    if(length(pos) > 1.0)
        discard;
    /* color = (v_position + vec4(4, 2.5, 2, 1.0)) / vec4(8, 5, 4, 2); */
    color = v_color;

}

