#version 430

in vec2 tex_coord0;
out vec4 frag_color;

uniform sampler2D depth_tex;
uniform sampler2D thickness_tex;

uniform int width;
uniform int height;

int particle_num;
uniform float sphere_radius;

uniform mat4 projection_mat;
uniform mat4 inverse_proj;
uniform mat4 inverse_modelview;

uniform vec2 texel_size;

//get zw in ndc space
vec2 getZW(vec2 coords )
{
    float z = texture(depth_tex, coords).x;
    vec4 pos = vec4(0, 0, z, 1);
    pos = projection_mat * pos; //clip space
    z = pos.z / pos.w; //ndc space
    return vec2(z, pos.w);
}

vec3 uvToEye(vec2 uv, float depth, float w)
{
    vec4 N;
    N.x = uv.x * 2.0 - 1.0;
    N.y = uv.y * 2.0 - 1.0;
    N.z = depth; //in ndc space

    //clip space
    N = w * N;
    N.w = w;

    vec4 posEye = inverse_proj * N;
    return posEye.xyz;
}

vec3 computeNormal(vec2 uv)
{
    vec2 zw;
    zw = getZW(uv);
    vec3 posEye = uvToEye(uv, zw.x, zw.y);

    zw = getZW(uv + vec2(texel_size.x, 0));
    vec3 posEye_right = uvToEye(uv + vec2(texel_size.x, 0), zw.x, zw.y);

    zw = getZW(uv + vec2(-texel_size.x, 0));
    vec3 posEye_left = uvToEye(uv + vec2(-texel_size.x, 0), zw.x, zw.y);

    vec3 ddx = posEye_right - posEye;
    vec3 ddx2 = posEye - posEye_left;
    if(abs(ddx.z) > abs(ddx2.z))
        ddx = ddx2;

    zw = getZW(uv + vec2(0 ,texel_size.y));
    vec3 posEye_up = uvToEye(uv + vec2(0, texel_size.y), zw.x, zw.y);
    
    zw = getZW(uv + vec2(0 ,-texel_size.y));
    vec3 posEye_down = uvToEye(uv + vec2(0, -texel_size.y), zw.x, zw.y);

    vec3 ddy = posEye_up - posEye;
    vec3 ddy2 = posEye - posEye_down;
    if(abs(ddy.z) > abs(ddy2.z))
        ddy = ddy2;

    vec3 n = cross(ddx, ddy);
    return normalize(n);
}

void main()
{
        float z = texture(depth_tex, tex_coord0).r;
        if(z == 0)
            discard;

        vec2 zw;
        zw = getZW(tex_coord0);
        vec3 posEye = uvToEye(tex_coord0, zw.x, zw.y);
        vec4 posWorld = inverse_modelview * vec4(posEye, 1.0);

        float thickness = texture(thickness_tex, tex_coord0).r;
        float thickness_left = texture(thickness_tex, tex_coord0 + vec2(-texel_size.x, 0)).r;
        float thickness_right = texture(thickness_tex, tex_coord0 + vec2(texel_size.x, 0)).r;
        float thickness_down = texture(thickness_tex, tex_coord0 + vec2(0, -texel_size.y)).r;
        float thickness_up = texture(thickness_tex, tex_coord0 + vec2(0, texel_size.y)).r;
        //thickness *= sphere_radius;
        vec3 N = computeNormal(tex_coord0);

        const float k_r = 5.0f;
        const float k_g = 1.0f;
        const float k_b = 0.1;

        vec4 c_beer = vec4(exp(-k_r * thickness),
                          exp(-k_g * thickness),
                          exp(-k_b * thickness),
                          1 - exp(-3 * thickness));
        const vec3 L = vec3(0.577, 0.577, 0.577);
        vec3 E = normalize(-posEye);
        vec3 R = normalize(reflect(-L, N));
        vec4 specular = pow(max(0.0, dot(R, E)), 30.0f) * vec4(1.0, 1.0, 1.0, 1.0);
        vec4 diffuse = max(0, dot(N, L)) * c_beer;
        frag_color = max(0, dot(N, L)) * (posWorld + 2.5) / 5.0 + specular; //color with position
        frag_color = diffuse + specular;
        
        /* //fresnel reflection */

        float r_0 = 0.3f;
        float fres_refl = r_0 + (1 - r_0) * pow(1 - dot(N, E), 5.0f);
        fres_refl =  pow(1 - abs(dot(N, E)), 8.0f);

        float normal_reflectance = pow(clamp(dot(N, L), 0, 1), 6);
        float spec_coeff = clamp(normal_reflectance +  (1 - normal_reflectance) * fres_refl, 0, 1);
        frag_color = diffuse + 0.1 * thickness * spec_coeff * vec4(1.0);

 }

