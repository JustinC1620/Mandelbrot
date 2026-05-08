#version 460 core
out vec4 frag_color;

in vec2 coord;

uniform sampler2D mu_tex;
uniform float time;
uniform int render_mode;
uniform bool ok_lch;

vec3 polar_to_cartesian(in const vec3 polar) 
{
    float H = radians(polar.z); 
    return vec3(polar.x, polar.y * cos(H), polar.y * sin(H));
}

vec3 lab_to_xyz(in const vec3 lab) 
{    
    const float delta = 6.0 / 29.0;
    const float kappa = 16.0 / 116.0;
    const float gamma = 3.0 * delta * delta;
    const vec3 white_point = vec3(95.047, 100.000, 108.883);

    vec3 xyz = vec3(0.0);
    xyz.y = (lab.x + 16.0) / 116.0;
    xyz.x = lab.y / 500.0 + xyz.y;
    xyz.z = xyz.y - lab.z / 200.0;
    
    xyz.x = (xyz.x > delta) ? xyz.x * xyz.x * xyz.x : (xyz.x - kappa) * gamma;
    xyz.y = (xyz.y > delta) ? xyz.y * xyz.y * xyz.y : (xyz.y - kappa) * gamma;
    xyz.z = (xyz.z > delta) ? xyz.z * xyz.z * xyz.z : (xyz.z - kappa) * gamma;
    
    return xyz * white_point;
}

vec3 xyz_to_srgb(in const vec3 xyz) 
{
    const mat3 M = mat3(
         3.240812398895283, -0.9692430170086407,  0.055638398436112804,
        -1.5373084456298136, 1.8759663029085742, -0.20400746093241362,
        -0.4985865229069666, 0.04155503085668564, 1.0571295702861434
    );
    return M * (xyz / 100.0);
}

vec3 srgb_to_rgb(in const vec3 srgb) 
{
    const float gamma = 1.0 / 2.4;
    const float a = 0.00313066844250060782371;
    const float b = 1.055;
    const float c = 0.055;
    const float d = 12.92;
    vec3 rgb;
    rgb.r = (srgb.r > a) ? b * pow(srgb.r, gamma) - c : d * srgb.r;
    rgb.g = (srgb.g > a) ? b * pow(srgb.g, gamma) - c : d * srgb.g;
    rgb.b = (srgb.b > a) ? b * pow(srgb.b, gamma) - c : d * srgb.b;
    
    return clamp(rgb, 0, 1);
}

vec3 lch_to_rgb(in vec3 lch) {
    return srgb_to_rgb(xyz_to_srgb(lab_to_xyz(polar_to_cartesian(lch))));
}

vec3 oklab_to_lms(in const vec3 oklab) 
{
    const mat3 M = mat3(
        1.0,           1.0,           1.0,
        0.3963377774, -0.1055613458, -0.0894841775,
        0.2158037573, -0.0638541728, -1.2914855480
    );
    vec3 lms = M * oklab;
    return lms * lms * lms;
}

vec3 lms_to_srgb(in const vec3 lms) 
{
    const mat3 M = mat3(
         4.0767416621, -1.2684380046,  -0.0041960863,
        -3.3077115913,  2.6097574011,  -0.7034186147,
         0.2309699292, -0.3413193965,   1.7076147010
    );
    return M * lms;
}

vec3 oklch_to_rgb(in const vec3 lch) {
    return srgb_to_rgb(lms_to_srgb(oklab_to_lms(polar_to_cartesian(lch))));
}

float modulate_value(in const float val, in const float center, 
                     in const float amp, in const float lambda, in const float phi)
{
    const float PI = 3.1415926535897932;
    return center + amp * (1.0 - 2.0 * pow(cos(lambda * PI * val + phi), 2));
}

vec4 powers(in const float w, in const float base)
{
    float z = w * base;
    float y = z * base;
    float x = y * base;
    return vec4(x, y, z, w);
}

float sum3x4(in const mat3x4 M)
{
    return dot(M[0] + M[1] + M[2], vec4(1.0));
}

vec2 iter_to_lch(in const mat3x4 power_mat) 
{
    const mat3x4 luma_mat = mat3x4
    (
          82.12107433759,     16.9122036737341, -486.744702088594,
         -79.0167021749514, 1050.34228856749,    104.445458315961,
        -981.709485001709,    -9.81483056041056, 370.282292765689,
         -32.2094857382239,  -73.1688646628623,   -8.71180561278684
    );
    const float luma_const = 90.2545370226951;

    const mat3x4 chroma_mat = mat3x4
    (
        -171.486303479868,   -39.0068704557659, 1131.72212229658,
         220.079633858195, -2800.0065916923,    -407.219160990875,
        3168.67495726893,    201.618281161229, -1602.04720347011,
         140.65740570673,    324.099505708911,  -103.355194919133
    );
    const float chroma_const = 54.8060915809228;

    const mat3x4 luma_result = matrixCompMult(luma_mat, power_mat);
    const mat3x4 chroma_result = matrixCompMult(chroma_mat, power_mat);
    
    const float L = sum3x4(luma_result) + luma_const;
    const float C = sum3x4(chroma_result) + chroma_const;

    return vec2(L, C);
}

vec2 iter_to_oklch(in const mat3x4 power_mat) 
{
    const mat3x4 luma_mat = mat3x4
    (
        -0.197017475142613, -0.127483090100062,   1.43505731158663,
         1.05437844091479,  -3.94209394296273,   -3.1866542026319,
         4.92905899817345,   4.17598597749999,   -2.50049398585562,
        -2.01987194244925,   0.0492765078472333,  0.038296478840831
    );
    const float luma_const = 0.880778576417983;

    const mat3x4 chroma_mat = mat3x4
    (
         0.0946118362065885, 0.0750058583527888, -0.486144951052041,
        -0.574645520421571,  0.83342911985362,    1.72120707643475,
        -0.475923251587507, -2.47991767746912,   -0.0752208619026352,
         1.62661930713243,   0.18336334769115,   -0.309276857156298
    );
    const float chroma_const = 0.173239700678255;

    const mat3x4 luma_result = matrixCompMult(luma_mat, power_mat);
    const mat3x4 chroma_result = matrixCompMult(chroma_mat, power_mat);
    
    const float L = sum3x4(luma_result) + luma_const;
    const float C = sum3x4(chroma_result) + chroma_const;

    return vec2(L, C);
}

float read_mu(ivec2 p)
{
    ivec2 size = textureSize(mu_tex, 0);
    p = clamp(p, ivec2(0), size - 1);
    return texelFetch(mu_tex, p, 0).r;
}

vec3 shade(in const ivec2 p) {
    const float mu = read_mu(p);
    if (mu == 0) {return vec3(0.0);}

    const float shift = 15.0;

    const float l_shift = ok_lch ? shift * 0.01             : shift;
    const float c_shift = ok_lch ? shift * 0.00255172413793 : shift;
    const float eased_iter = sqrt(mu);

    //const float H = mod(mod(360 * eased_iter, 360) + 10 * time, 360);  // Time Shifting Color
    const float H = mod(360 * 2 * eased_iter, 360);
    const float h_norm = (H - 180) / 127.456120241149;
    mat3x4 power_mat = mat3x4(0.0);
    power_mat[2] = powers(h_norm, h_norm);
    power_mat[1] = powers(power_mat[2].x * h_norm, h_norm);
    power_mat[0] = powers(power_mat[1].x * h_norm, h_norm);

    vec2 LC = ok_lch ? iter_to_oklch(power_mat) : iter_to_lch(power_mat);

    LC.x = modulate_value(mu, LC.x - l_shift, l_shift, shift, 0.0);
    LC.y = modulate_value(mu, LC.y - c_shift, c_shift, shift, 0.0);

    if (ok_lch) {
        return oklch_to_rgb(vec3(LC, H));
    }
    else {
        return lch_to_rgb(vec3(LC, H));
    }
}

float laplace_mu(in const ivec2 p)
{
    float d = 0.0;

    d += -1.0 * read_mu(p + ivec2( 0,  2));
    d += -2.0 * read_mu(p + ivec2(-1,  1));
    d += -4.0 * read_mu(p + ivec2( 0,  1));
    d += -2.0 * read_mu(p + ivec2( 1,  1));
    d += -1.0 * read_mu(p + ivec2(-2,  0));
    d += -4.0 * read_mu(p + ivec2(-1,  0));
    d += 20.0 * read_mu(p + ivec2( 0,  0));
    d += -4.0 * read_mu(p + ivec2( 1,  0));
    d += -1.0 * read_mu(p + ivec2( 2,  0));
    d += -2.0 * read_mu(p + ivec2(-1, -1));
    d += -4.0 * read_mu(p + ivec2( 0, -1));
    d += -2.0 * read_mu(p + ivec2( 1, -1));
    d += -1.0 * read_mu(p + ivec2( 0, -2));

    return abs(d);
}

float sobel_mu(in const ivec2 p)
{
    float gx = 0.0;
    float gy = 0.0;

    gx += -1.0 * read_mu(p + ivec2(-1,  1));
    gx += -2.0 * read_mu(p + ivec2(-1,  0));
    gx += -1.0 * read_mu(p + ivec2(-1, -1));
    gx +=  1.0 * read_mu(p + ivec2( 1,  1));
    gx +=  2.0 * read_mu(p + ivec2( 1,  0));
    gx +=  1.0 * read_mu(p + ivec2( 1, -1));

    gy += -1.0 * read_mu(p + ivec2(-1,  1));
    gy += -2.0 * read_mu(p + ivec2( 0,  1));
    gy += -1.0 * read_mu(p + ivec2( 1,  1));
    gy +=  1.0 * read_mu(p + ivec2(-1, -1));
    gy +=  2.0 * read_mu(p + ivec2( 0, -1));
    gy +=  1.0 * read_mu(p + ivec2( 1, -1));

    return length(vec2(gx, gy));
}

void main()
{
    const ivec2 p = ivec2(gl_FragCoord.xy);
    const float k = 2.0;
    vec3 color;

    if (render_mode == 1)
        color = vec3(sobel_mu(p));
    else if (render_mode == 2)
        color = vec3(1 - exp(-pow(laplace_mu(p) / k, 2)));
    else
        color = shade(p);

    frag_color = vec4(color, 1.0);
}



