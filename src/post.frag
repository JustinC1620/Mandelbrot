#version 460 core
out vec4 frag_color;

in vec2 coord;

uniform sampler2D color_tex;
uniform int render_mode;

vec3 read_rgb(ivec2 p)
{
    ivec2 size = textureSize(color_tex, 0);
    p = clamp(p, ivec2(0), size - 1);
    return texelFetch(color_tex, p, 0).rgb;
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
    ivec2 p = ivec2(gl_FragCoord.xy);
    const float k = 15.0;
    vec3 color;

    if (render_mode == 3)
        color = vec3(sobel_rgb(p));
    else if (render_mode == 4)
        color = vec3(1 - exp(-pow(laplace_rgb(p) / k, 2)));
    else
        color = read_rgb(p);
    
    frag_color = vec4(color, 1.0);
}