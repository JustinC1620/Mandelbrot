function [r, g, b] = lch_to_rgb(l, c, h)
%   Converts a color from lch to rgb
%   l: Luma value
%   c: Chroma value
%   h: Hue value
    [l, a, b] = lch_to_lab(l, c, h);
    [x, y, z] = lab_to_xyz(l, a, b);
    [r, g, b] = xyz_to_rgb(x, y, z);
end

function [l, a, b] = lch_to_lab(l, c, h)
%   Converts a color from lch to lab
%   l: Luma value
%   c: Chroma value
%   h: Hue value
    H = deg2rad(h);
    a = c .* cos(H);
    b = c .* sin(H);
end

function [x, y, z] = lab_to_xyz(l, a, b)
%   Converts a color from lab to xyz
%   l: Luma value
%   a: CIE a value
%   b: CIE b value
    d = 6 / 29;
    k = 16 / 116;
    g = 3 * d * d;
    wp = [95.047; 100; 108.883];

    y = (l + 16) ./ 116;
    x = a ./ 500 + y;
    z = y - b ./ 200;

    I = x > d;
    x(I) = x(I) .* x(I) .* x(I);
    x(~I) = (x(~I) - k) .* g;
    x = x .* wp(1);
    I = y > d;
    y(I) = y(I) .* y(I) .* y(I);
    y(~I) = (y(~I) - k) .* g;
    y = y .* wp(2);
    I = z > d;
    z(I) = z(I) .* z(I) .* z(I);
    z(~I) = (z(~I) - k) .* g;
    z = z .* wp(3);
end

function [r, g, b] = xyz_to_rgb(x, y, z)
%   Converts a color from xyz to rgb
%   x: CIE x value
%   y: CIE y value
%   z: CIE z value
    M = [3.2406, -1.5372, -0.4986;...
        -0.9689,  1.8758,  0.0415;...
         0.0557, -0.2040,  1.0570];
    
    coords = cat(4, x, y, z);
    coords = permute(coords, [4, 1, 2, 3]);
    result = pagemtimes(M, coords) ./ 100;
    r = squeeze(result(1,:,:,:));
    g = squeeze(result(2,:,:,:));
    b = squeeze(result(3,:,:,:));
    I = r > 0.0031308;
    r(I) = power(r(I), 1 / 2.4) .* 1.055 - 0.055;
    r(~I) = r(~I) .* 12.92;
    I = g > 0.0031308;
    g(I) = power(g(I), 1 / 2.4) .* 1.055 - 0.055;
    g(~I) = g(~I) .* 12.92;
    I = b > 0.0031308;
    b(I) = power(b(I), 1 / 2.4) .* 1.055 - 0.055;
    b(~I) = b(~I) .* 12.92;
end