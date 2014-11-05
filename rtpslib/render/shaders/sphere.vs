#version 430

layout (location = 0) in vec4 position;

uniform mat4 modelview_mat;
uniform mat4 projection_mat;
uniform float sphere_radius;

out vec4 v_position_in_view;

void main()
{
    v_position_in_view = modelview_mat * vec4(position.xyz, 1.0);
    float dist = length(v_position_in_view);
    gl_PointSize = 15;
    gl_Position = projection_mat * modelview_mat * vec4(position.xyz, 1.0);
}

