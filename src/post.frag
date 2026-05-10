#version 460 core
out vec4 frag_color;

in vec2 coord;

uniform sampler2D color_tex;
uniform int render_mode;
uniform float zoom;

vec4 read_color_dist(ivec2 p)
{
    ivec2 size = textureSize(color_tex, 0);
    p = clamp(p, ivec2(0), size - 1);
    return texelFetch(color_tex, p, 0);
}

vec3 read_rgb(ivec2 p)
{
    return read_color_dist(p).rgb;
}

float dist_to_height(float dist)
{
    float k = 0.1;
    if (dist == 0.0) return 1.0;
    float normalized = dist * textureSize(color_tex, 0).x / zoom;
    return clamp(1 / (1 + k * normalized), 0.0, 1.0);
}

float laplace_rgb(ivec2 p)
{
    vec3 d = vec3(0.0);

    d += -1.0 * read_rgb(p + ivec2( 0,  2));
    d += -2.0 * read_rgb(p + ivec2(-1,  1));
    d += -4.0 * read_rgb(p + ivec2( 0,  1));
    d += -2.0 * read_rgb(p + ivec2( 1,  1));
    d += -1.0 * read_rgb(p + ivec2(-2,  0));
    d += -4.0 * read_rgb(p + ivec2(-1,  0));
    d += 20.0 * read_rgb(p + ivec2( 0,  0));
    d += -4.0 * read_rgb(p + ivec2( 1,  0));
    d += -1.0 * read_rgb(p + ivec2( 2,  0));
    d += -2.0 * read_rgb(p + ivec2(-1, -1));
    d += -4.0 * read_rgb(p + ivec2( 0, -1));
    d += -2.0 * read_rgb(p + ivec2( 1, -1));
    d += -1.0 * read_rgb(p + ivec2( 0, -2));

    return length(d);
}

float sobel_rgb(ivec2 p)
{
    vec3 gx = vec3(0.0);
    vec3 gy = vec3(0.0);

    gx += -1.0 * read_rgb(p + ivec2(-1,  1));
    gx += -2.0 * read_rgb(p + ivec2(-1,  0));
    gx += -1.0 * read_rgb(p + ivec2(-1, -1));
    gx +=  1.0 * read_rgb(p + ivec2( 1,  1));
    gx +=  2.0 * read_rgb(p + ivec2( 1,  0));
    gx +=  1.0 * read_rgb(p + ivec2( 1, -1));

    gy += -1.0 * read_rgb(p + ivec2(-1,  1));
    gy += -2.0 * read_rgb(p + ivec2( 0,  1));
    gy += -1.0 * read_rgb(p + ivec2( 1,  1));
    gy +=  1.0 * read_rgb(p + ivec2(-1, -1));
    gy +=  2.0 * read_rgb(p + ivec2( 0, -1));
    gy +=  1.0 * read_rgb(p + ivec2( 1, -1));

    return length(vec2(length(gx), length(gy)));
}

void main()
{
    const ivec2 p = ivec2(gl_FragCoord.xy);
    const float k = 15.0;
    vec3 color;

    if (render_mode == 0){
        const vec2 light_dir = normalize(vec2(0.5, 0.5));
        const float light_angle = 30.0;
        const float normal_strength = 5.0;
        const float ambient = 0.1;
        const float h_r = dist_to_height(read_color_dist(p + ivec2( 1,  0)).a);
        const float h_l = dist_to_height(read_color_dist(p + ivec2(-1,  0)).a);
        const float h_u = dist_to_height(read_color_dist(p + ivec2( 0,  1)).a);
        const float h_d = dist_to_height(read_color_dist(p + ivec2( 0, -1)).a);

        const float scale = textureSize(color_tex, 0).x / zoom;

        const vec3 normal = normalize(vec3(
            (h_l - h_r) * normal_strength * scale,
            (h_d - h_u) * normal_strength * scale,
            1.0
        ));

        const float elev = radians(light_angle);
        const vec3 light = normalize(vec3(light_dir * cos(elev), sin(elev)));
        const float diffuse = max(dot(normal, light), 0.0);
        float lighting = ambient + (1.0 - ambient) * diffuse;
        lighting = lighting / (ambient + (1.0 - ambient) * 0.5);

        color = read_rgb(p) * lighting;
    }
    else if (render_mode == 3)
        color = vec3(sobel_rgb(p));
    else if (render_mode == 4)
        color = vec3(1 - exp(-pow(laplace_rgb(p) / k, 2)));
    else
        color = read_rgb(p);

    frag_color = vec4(color, 1.0);
}
