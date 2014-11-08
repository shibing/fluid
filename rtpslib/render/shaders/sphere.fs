#version 430

uniform mat4 modelview_mat;
uniform mat4 projection_mat;
uniform float sphere_radius;

in vec4 v_position_in_view;
in vec3 v_position_in_world;

out vec4 color;

vec4 diffuse_color = vec4(1.0);

vec3 L = vec3(1.0, 1.0, 1.0);

void main()
{
    float far = gl_DepthRange.far;
    float near = gl_DepthRange.near;

    vec3 N;
    N.xy = gl_PointCoord.xy * vec2(2.0, -2.0) + vec2(-1, 1);
    float mag = dot(N.xy, N.xy);
    if(mag > 1.0) discard;
    N.z = sqrt(1.0 - mag);

    vec4 sphere_position_in_view = vec4(v_position_in_view.xyz + N * sphere_radius, 1.0);
    vec4 sphere_position_in_proj = projection_mat * sphere_position_in_view;

    sphere_position_in_proj.z  /= sphere_position_in_proj.w;

    gl_FragDepth = ((far - near) * sphere_position_in_proj.z + far + near)/2.0;
    color = max(0, dot(N, L)) * vec4((v_position_in_world + 2.5) / 5.0, 1.0);

}
