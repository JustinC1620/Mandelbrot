function rough_approx = rough_luma_chroma(num_steps, space)
%   rough_luma_chroma  Approximates rough max luma and chroma per hue
%   num_steps: Number of steps across hue space
%   space:     'lch' or 'oklch'
%
%   Strategy: for every (hue, L) pair, binary-search the maximum in-gamut C.
%   This correctly traces the 2D gamut boundary in (L,C) space for each hue,
%   then picks the (L,C) pair that maximises the equal-weighted objective.

    if nargin < 2
        space = 'lch';
    end

    switch lower(space)
        case 'lch'
            L_MAX = 100;
            C_MAX = 145;
        case 'oklch'
            L_MAX = 1;
            C_MAX = 0.4;
        otherwise
            error("rough_luma_chroma: unknown space '%s'.", space);
    end

    BISECT_ITERS = 24;   % 2^-24 relative resolution on C — plenty for a rough seed

    h_vec = cheby_space(0, 360, num_steps);   % (1 x num_steps)
    l_vec = linspace(0, L_MAX, num_steps);    % (1 x num_steps)

    % For each hue, find max in-gamut C at every L via bisection,
    % then pick the best (L, C) pair.
    % We vectorise over hues (columns) and L values (rows) simultaneously.
    %
    % Matrices are (num_steps_L  x  num_steps_H)

    L_mat = repmat(l_vec(:),    1, num_steps);   % L varies down rows
    H_mat = repmat(h_vec(:)',   num_steps, 1);   % H varies across cols

    % Binary-search for max in-gamut C at each (L, H) pair
    lo = zeros(num_steps, num_steps);
    hi = repmat(C_MAX, num_steps, num_steps);

    for k = 1 : BISECT_ITERS
        mid = (lo + hi) / 2;
        [r, g, b] = color_to_rgb(L_mat, mid, H_mat, space);
        in = in_gamut(r, g, b);
        lo(in)  = mid(in);    % feasible  → try higher
        hi(~in) = mid(~in);   % infeasible → try lower
    end

    C_boundary = lo;   % max in-gamut C for every (L, H) pair

    % Mark (L,H) pairs where even C=0 is out of gamut (e.g. L too bright)
    [r0, g0, b0] = color_to_rgb(L_mat, zeros(num_steps, num_steps), H_mat, space);
    valid = in_gamut(r0, g0, b0);
    C_boundary(~valid) = -Inf;

    % Objective: equal-weight normalised (L + C) — maximise over L rows
    obj = L_mat / L_MAX + C_boundary / C_MAX;
    obj(~valid) = -Inf;

    [~, best_l_idx] = max(obj, [], 1);   % best L index for each hue column

    rough_approx = zeros(num_steps, 3);
    for j = 1 : num_steps
        li = best_l_idx(j);
        rough_approx(j, :) = [l_vec(li), C_boundary(li, j), h_vec(j)];
    end
end