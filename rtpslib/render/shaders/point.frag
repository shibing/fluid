#version 430

out vec4 color;
void main()
{
    float len = dot(gl_PointCoord - 0.5, gl_PointCoord - 0.5);
    color = mix(vec4(1.0), vec4(0.0), smoothstep(0.23, 0.27, len));
}

