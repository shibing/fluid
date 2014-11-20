#version 430

in vec2 tex_coord0;
smooth out vec4 frag_color;

uniform sampler2D depth_tex;
uniform sampler2D thickness_tex;
uniform sampler2D background_tex;
uniform samplerCube cube_map_tex;
uniform sampler2D color_tex;

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
    if(z == 0)
        z = 0.00001;
    vec4 pos = vec4(0, 0, z, 1);
    pos = projection_mat * pos; //clip space
    //z = pos.z / pos.w; //ndc space
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
    vec2 zw_self;
    zw_self = getZW(uv);
    vec3 posEye = uvToEye(uv, zw_self.x / zw_self.y, zw_self.y);

    vec2 zw;
    zw = getZW(uv + vec2(texel_size.x, 0));
    if(zw.x == 0)
        zw.x = zw_self.x;
    vec3 posEye_right = uvToEye(uv + vec2(texel_size.x, 0), zw.x / zw.y, zw.y);

    zw = getZW(uv + vec2(-texel_size.x, 0));
    if(zw.x == 0)
        zw.x = zw_self.x;
    vec3 posEye_left = uvToEye(uv + vec2(-texel_size.x, 0), zw.x / zw.y, zw.y);

    vec3 ddx = posEye_right - posEye;
    vec3 ddx2 = posEye - posEye_left;
    if(abs(ddx.z) > abs(ddx2.z))
        ddx = ddx2;

    zw = getZW(uv + vec2(0 ,texel_size.y));
    if(zw.x == 0)
        zw.x = zw_self.x;
    vec3 posEye_up = uvToEye(uv + vec2(0, texel_size.y), zw.x / zw.y,  zw.y);
    
    zw = getZW(uv + vec2(0 ,-texel_size.y));
    if(zw.x == 0)
        zw.x = zw_self.x;
    vec3 posEye_down = uvToEye(uv + vec2(0, -texel_size.y), zw.x / zw.y, zw.y);

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
        vec4 background = texture(background_tex, tex_coord0);

        if(z == 0) {
            frag_color = background;
            return;
        }

        vec2 zw;
        zw = getZW(tex_coord0);
        vec3 posEye = uvToEye(tex_coord0, zw.x, zw.y);
        vec4 posWorld = inverse_modelview * vec4(posEye, 1.0);

        float thickness = texture(thickness_tex, tex_coord0).r;

        int radius = 10;
        float sum = 0;
        float wsum = 0;
        float sigma = 5;

        for(int r = -radius; r <= radius; r += 1) {
            float samp = texture(thickness_tex, tex_coord0 + r * vec2(texel_size.x, 0)).x;
            float v = r / sigma;
            float w = exp(-v * v/2.0);
            sum += samp * w;
            wsum += w;
        }

        for(int r = -radius; r <= radius; r += 1) {
            float samp = texture(thickness_tex, tex_coord0 + r * vec2(0, texel_size.y)).x;
            float v = r / sigma;
            float w = exp(-v * v/2.0);
            sum += samp * w;
            wsum += w;
        }
        if(wsum > 0)
            sum /= wsum;
       thickness = sum;

        vec3 N = computeNormal(tex_coord0);

        const float k_r = 5.0;
        const float k_g = 1.0;
        const float k_b = 0.1;

        vec4 c_beer = vec4(exp(-k_r * thickness),
                          exp(-k_g * thickness),
                          exp(-k_b * thickness),
                          1 - exp(-3 * thickness));

        const vec3 L = vec3(0.577, 0.577, 0.577);
        vec3 E = normalize(-posEye);
        vec3 R = normalize(reflect(-L, N));
        vec3 H = normalize(E + L);
        float diffuse = max(0, dot(N, L)) ;
        float specular = pow(max(0.0, dot(R, E)), 30.0f);

        vec4 refrac_color = texture(background_tex, tex_coord0 + N.xy * thickness); //refraction
        vec4 self_color = mix(c_beer * diffuse, refrac_color, exp(-thickness)); //the color of fluid self

        /* //fresnel reflection */
        float r_0 = 0.1;
        float fres_refl = r_0 + (1 - r_0) * pow(1 - max(0.0, dot(H, E)), 5.0f);

        //Cube Map reflection
        vec3 viewer_reflect = normalize(reflect(posEye, N));
        vec4 refl_color = texture(cube_map_tex, viewer_reflect);

        frag_color = (1 - fres_refl) * self_color +  fres_refl * refl_color + specular * vec4(1.0);
        frag_color = texture(color_tex, tex_coord0) * diffuse;
 }

