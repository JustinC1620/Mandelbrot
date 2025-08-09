#version 460 core
out vec4 frag_color;
	
in vec2 coord;
	
uniform sampler2D tex;

vec2 sobel(sampler2D smpl, vec2 texel)
{
    const ivec2 SIZE = textureSize(smpl, 0);
    const vec2 SCALE = vec2(1.0 / SIZE.x, 1.0 / SIZE.y);
    const vec3 GRAY = vec3(0.299, 0.597, 0.114);
    vec2 g = vec2(0.0);

    g.x += 1.0 * dot(texture(smpl, texel + vec2(-1, 1) * SCALE).rgb, GRAY);
    g.x += 2.0 * dot(texture(smpl, texel + vec2(-1, 0) * SCALE).rgb, GRAY);
    g.x += 1.0 * dot(texture(smpl, texel + vec2(-1, -1) * SCALE).rgb, GRAY);

    g.x += -1.0 * dot(texture(smpl, texel + vec2(1, 1) * SCALE).rgb, GRAY);
    g.x += -2.0 * dot(texture(smpl, texel + vec2(1, 0) * SCALE).rgb, GRAY);
    g.x += -1.0 * dot(texture(smpl, texel + vec2(1, -1) * SCALE).rgb, GRAY);

    g.y += 1.0 * dot(texture(smpl, texel + vec2(-1, 1) * SCALE).rgb, GRAY);
    g.y += 2.0 * dot(texture(smpl, texel + vec2(0, 1) * SCALE).rgb, GRAY);
    g.y += 1.0 * dot(texture(smpl, texel + vec2(1, 1) * SCALE).rgb, GRAY);

    g.y += -1.0 * dot(texture(smpl, texel + vec2(-1, -1) * SCALE).rgb, GRAY);
    g.y += -2.0 * dot(texture(smpl, texel + vec2(0, -1) * SCALE).rgb, GRAY);
    g.y += -1.0 * dot(texture(smpl, texel + vec2(1, -1) * SCALE).rgb, GRAY);
    
    return g;
}

float laplace(sampler2D smpl, vec2 texel)
{
    const ivec2 SIZE = textureSize(smpl, 0);
    const vec2 SCALE = vec2(1.0 / SIZE.x, 1.0 / SIZE.y);
    const vec3 GRAY = vec3(0.299, 0.597, 0.114);

    float d = 0.0;

    d += 0.25 * dot(texture(smpl, texel + vec2(-1, 1) * SCALE).rgb, GRAY);
    d += 0.5 * dot(texture(smpl, texel + vec2(-1, 0) * SCALE).rgb, GRAY);
    d += 0.25 * dot(texture(smpl, texel + vec2(-1, -1) * SCALE).rgb, GRAY);

    d += 0.5 * dot(texture(smpl, texel + vec2(0, 1) * SCALE).rgb, GRAY);
    d += -3.0 * dot(texture(smpl, texel + vec2(0, 0) * SCALE).rgb, GRAY);
    d += 0.5 * dot(texture(smpl, texel + vec2(0, -1) * SCALE).rgb, GRAY);

    d += 0.25 * dot(texture(smpl, texel + vec2(1, 1) * SCALE).rgb, GRAY);
    d += 0.5 * dot(texture(smpl, texel + vec2(1, 0) * SCALE).rgb, GRAY);
    d += 0.25 * dot(texture(smpl, texel + vec2(1, -1) * SCALE).rgb, GRAY);

    return d;
}

mat2x3 sobelColor(sampler2D smpl, vec2 texel)
{
    const ivec2 SIZE = textureSize(smpl, 0);
    const vec2 SCALE = vec2(1.0 / SIZE.x, 1.0 / SIZE.y);
    const vec3 GRAY = vec3(0.299, 0.597, 0.114);
    mat2x3 g = mat2x3(0.0);

    g[0] += texture(smpl, texel + vec2(-1, 1) * SCALE).rgb;
    g[0] += 2 * texture(smpl, texel + vec2(-1, 0) * SCALE).rgb;
    g[0] += texture(smpl, texel + vec2(-1, 1) * SCALE).rgb;

    g[0] += -texture(smpl, texel + vec2(1, 1) * SCALE).rgb;
    g[0] += -2.0 * texture(smpl, texel + vec2(1, 0) * SCALE).rgb;
    g[0] += -texture(smpl, texel + vec2(1, 1) * SCALE).rgb;

    g[1] += texture(smpl, texel + vec2(-1, 1) * SCALE).rgb;
    g[1] += 2.0 * texture(smpl, texel + vec2(0, 1) * SCALE).rgb;
    g[1] += texture(smpl, texel + vec2(1, 1) * SCALE).rgb;

    g[1] += -texture(smpl, texel + vec2(-1, -1) * SCALE).rgb;
    g[1] += -2.0 * texture(smpl, texel + vec2(0, -1) * SCALE).rgb;
    g[1] += -texture(smpl, texel + vec2(1, -1) * SCALE).rgb;
    
    return g;
}
void main()
{   
    const int k = 2;
    float c = exp(-pow(abs(laplace(tex, coord)) / k, 2));
    //frag_color = vec4(vec3(length(sobel(tex, coord))), 0.0);
    frag_color = texture(tex, coord);
}