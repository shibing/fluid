#version 430

uniform mat4 modelview_mat;
uniform mat4 projection_mat;
uniform float sphere_radius;

in vec4 v_position_in_view;
in vec4 v_color;

out vec4 color;

void main()
{
    if(v_color.w > 0.5)
        discard;
    float far = gl_DepthRange.far;
    float near = gl_DepthRange.near;

    vec3 N;
    N.xy = gl_PointCoord.xy * vec2(-2.0, 2.0) + vec2(1, -1);
    float mag = dot(N.xy, N.xy);
    if(mag > 1.0) {
        discard;
        return;
    }
    N.z = sqrt(1.0 - mag);

    vec4 sphere_position_in_view = vec4(v_position_in_view.xyz + N * sphere_radius, 1.0);
    vec4 sphere_position_in_proj = projection_mat * sphere_position_in_view;

    float ndc_depth = sphere_position_in_proj.z / sphere_position_in_proj.w;

    gl_FragDepth = ((far - near) * ndc_depth + far + near)/2.0;

    //color.x = (ndc_depth + 1.0) * 0.5 ;
    color.x = sphere_position_in_view.z; //write view space z to texture
}
