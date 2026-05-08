function bool = in_gamut(r, g, b)
%   Checks if a normalized rbg tuple is in gamut
%   r: Red value
%   g: Green value
%   b: Blue value
    bool = r >= 0 & r <= 1 ...
           & g >= 0 & g <= 1 ...
           & b >= 0 & b <= 1;
end