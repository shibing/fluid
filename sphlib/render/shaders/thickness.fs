#version 430

const float sigma = 3.0f;
const float E = 0.0f;

in vec4 v_color;

out vec4 color;
void main()
{
    if(v_color.w > 0.5)
       discard; 
    vec3 N;
    N.xy = gl_PointCoord.xy * vec2(2.0, -2.0) + vec2(-1.0, 1.0);
    float mag = dot(N.xy, N.xy);
    if(mag > 1) {
        discard;
        return;
    }
    float dist = length(gl_PointCoord.xy - 0.5);
    float g_dist = 0.02f * exp(-(dist - E) * (dist - E) / (2 * sigma));
    color.x = g_dist; //thickness

}
