function [r, g, b] = color_to_rgb(l, c, h, space)
%   Converts a color from LCH or OKLCH to RGB
%   l:     Luma value
%   c:     Chroma value
%   h:     Hue value (degrees)
%   space: 'lch' or 'oklch'
    switch lower(space)
        case 'lch'
            [r, g, b] = lch_to_rgb(l, c, h);
        case 'oklch'
            [r, g, b] = oklch_to_rgb(l, c, h);
        otherwise
            error("color_to_rgb: unknown space '%s'. Use 'lch' or 'oklch'.", space);
    end
end

% -------------------------------------------------------------------------
% LCH pipeline
% -------------------------------------------------------------------------

function [r, g, b] = lch_to_rgb(l, c, h)
    [l, a, b] = lch_to_lab(l, c, h);
    [x, y, z] = lab_to_xyz(l, a, b);
    [r, g, b] = xyz_to_srgb(x, y, z);
    [r, g, b] = srgb_gamma(r, g, b);
end

function [l, a, b] = lch_to_lab(l, c, h)
    H = deg2rad(h);
    a = c .* cos(H);
    b = c .* sin(H);
end

function [x, y, z] = lab_to_xyz(l, a, b)
    d  = 6 / 29;
    k  = 16 / 116;
    g  = 3 * d * d;
    wp = [95.047; 100.0; 108.883];

    y = (l + 16) ./ 116;
    x = a ./ 500 + y;
    z = y - b ./ 200;

    I = x > d;  x(I) = x(I).^3;          x(~I) = (x(~I) - k) .* g;  x = x .* wp(1);
    I = y > d;  y(I) = y(I).^3;          y(~I) = (y(~I) - k) .* g;  y = y .* wp(2);
    I = z > d;  z(I) = z(I).^3;          z(~I) = (z(~I) - k) .* g;  z = z .* wp(3);
end

function [r, g, b] = xyz_to_srgb(x, y, z)
    M = [ 3.2406, -1.5372, -0.4986; ...
         -0.9689,  1.8758,  0.0415; ...
          0.0557, -0.2040,  1.0570];

    sz      = size(x);
    coords  = cat(4, x, y, z);
    coords  = permute(coords, [4, 1, 2, 3]);
    result  = pagemtimes(M, coords) ./ 100;

    r = reshape(result(1,:,:,:), sz);
    g = reshape(result(2,:,:,:), sz);
    b = reshape(result(3,:,:,:), sz);
end

% -------------------------------------------------------------------------
% OKLCH pipeline
% -------------------------------------------------------------------------

function [r, g, b] = oklch_to_rgb(l, c, h)
    [l, a, b_] = oklch_to_oklab(l, c, h);
    [r, g, b_] = oklab_to_lms(l, a, b_);
    [r, g, b]  = lms_to_srgb(r, g, b_);
    [r, g, b]  = srgb_gamma(r, g, b);
end

function [l, a, b] = oklch_to_oklab(l, c, h)
    H = deg2rad(h);
    a = c .* cos(H);
    b = c .* sin(H);
end

function [r, g, b] = oklab_to_lms(l, a, b)
    M = [1.0,  0.3963377774,  0.2158037573; ...
         1.0, -0.1055613458, -0.0638541728; ...
         1.0, -0.0894841775, -1.2914855480];

    sz     = size(l);
    coords = cat(4, l, a, b);
    coords = permute(coords, [4, 1, 2, 3]);
    lms    = pagemtimes(M, coords);
    lms    = lms .^ 3;

    r = reshape(lms(1,:,:,:), sz);
    g = reshape(lms(2,:,:,:), sz);
    b = reshape(lms(3,:,:,:), sz);
end

function [r, g, b] = lms_to_srgb(r_lms, g_lms, b_lms)
    M = [ 4.0767416621, -3.3077115913,  0.2309699292; ...
         -1.2684380046,  2.6097574011, -0.3413193965; ...
         -0.0041960863, -0.7034186147,  1.7076147010];

    coords = cat(4, r_lms, g_lms, b_lms);
    coords = permute(coords, [4, 1, 2, 3]);
    result = pagemtimes(M, coords);
    r = squeeze(result(1,:,:,:));
    g = squeeze(result(2,:,:,:));
    b = squeeze(result(3,:,:,:));
end

% -------------------------------------------------------------------------
% Shared sRGB gamma encoding (same transfer function for both pipelines)
% -------------------------------------------------------------------------

function [r, g, b] = srgb_gamma(r, g, b)
    r = apply_gamma(r);
    g = apply_gamma(g);
    b = apply_gamma(b);
end

function v = apply_gamma(v)
    a     = 0.0031308;
    I     = v > a;
    v(I)  = 1.055 .* v(I).^(1/2.4) - 0.055;
    v(~I) = 12.92 .* v(~I);
end