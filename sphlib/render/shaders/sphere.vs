#version 430

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 color;

uniform mat4 modelview_mat;
uniform mat4 projection_mat;
uniform float sphere_radius;
uniform int width;
uniform int height;

const float scale = height * projection_mat[1][1];

out vec4 v_position_in_view;
out vec3 v_position_in_world;
out vec4 v_color;

void main()
{
    v_position_in_view = modelview_mat * vec4(position.xyz, 1.0);
    v_position_in_world = position.xyz;
    v_color = color;
    float dist = length(v_position_in_view);
    gl_PointSize =  scale * sphere_radius / dist;
    gl_Position = projection_mat * modelview_mat * vec4(position.xyz, 1.0);
}

