#version 430

in vec2 tex_coord0;
out vec4 frag_color;

uniform sampler2D depth_tex;
uniform sampler2D thickness_tex;

uniform int width;
uniform int height;

uniform mat4 projection_mat;
uniform mat4 inverse_proj;
uniform mat4 inverse_modelview;

uniform vec2 texel_size;

//get zw in ndc space
vec2 getZW(vec2 coords )
{
    float z = texture(depth_tex, coords).x;
    vec4 pos = vec4(0, 0, z, 1);
    pos = projection_mat * pos;
    z = pos.z / pos.w;
    return vec2(z, pos.w);
}

vec3 uvToEye(vec2 uv, float depth, float w)
{
    vec4 N;
    N.x = uv.x * 2.0 - 1.0;
    N.y = uv.y * 2.0- 1.0;
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
    vec3 posEye_left = uvToEye(uv - vec2(texel_size.x, 0), zw.x, zw.y);

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
        vec3 N = computeNormal(tex_coord0);

        vec4 c_beer = vec4(exp(-0.6 * thickness),
                          exp(-0.2 * thickness),
                          exp(-0.05 * thickness),
                          1 - exp(-3 * thickness));
        const vec3 L = vec3(0.577, 0.577, 0.577);
        frag_color = max(0, dot(N, L)) * (posWorld + 2.5) / 5.0;
 }

