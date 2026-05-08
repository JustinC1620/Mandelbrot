function optimized_result = optimize_luma_chroma(rough_approx, space, weights)
%   optimize_luma_chroma  Optimizes maximum luma and chroma values per hue
%   rough_approx: Row vector [L, C, H] from rough_luma_chroma (single hue)
%   space:        'lch' or 'oklch'
%   weights:      [w_l, w_c] — relative weight of luma vs chroma (default [0.5 0.5])

    if nargin < 2 || isempty(space)
        space = 'lch';
    end
    if nargin < 3 || isempty(weights)
        weights = [0.5, 0.5];
    end

    % Normalize weights
    weights = weights / sum(weights);
    w_l = weights(1);
    w_c = weights(2);

    % Bounds per space
    switch lower(space)
        case 'lch'
            L_MAX = 100;
            C_MAX = 145;
            step_tol = 1e-10;
        case 'oklch'
            L_MAX = 1;
            C_MAX = 0.4;
            step_tol = 1e-7;   % coarser: avoids singular KKT at machine-epsilon scale
        otherwise
            error("optimize_luma_chroma: unknown space '%s'.", space);
    end

    hue = rough_approx(3);
    lb  = [0,     0    ];
    ub  = [L_MAX, C_MAX];

    % Pull the seed strictly inside the feasible region so interior-point
    % has a proper interior to start from.  A boundary seed (constraint == 0)
    % gives a rank-deficient KKT matrix on the very first iteration.
    x0 = pull_inside(rough_approx(1:2), hue, space, lb, ub);

    options = optimoptions('fmincon', ...
        'Display',                 'off', ...
        'Algorithm',               'interior-point', ...
        'MaxIterations',           400, ...
        'OptimalityTolerance',     1e-8, ...
        'StepTolerance',           step_tol, ...
        'ConstraintTolerance',     1e-8, ...
        'MaxFunctionEvaluations',  1e5);

    objective  = @(x) obj_func(x, w_l, w_c, L_MAX, C_MAX);
    constraint = @(x) con_func(x, hue, space);

    [x_opt, ~, exitflag] = fmincon(objective, x0, [], [], [], [], ...
                                   lb, ub, constraint, options);

    if exitflag > 0
        % Verify the result is genuinely in gamut before accepting
        [r, g, b] = color_to_rgb(x_opt(1), x_opt(2), hue, space);
        if in_gamut(r, g, b)
            optimized_result = [x_opt(1), x_opt(2), hue];
        else
            optimized_result = rough_approx;
        end
    else
        optimized_result = rough_approx;
    end
end

% -------------------------------------------------------------------------

function x0 = pull_inside(x_seed, hue, space, lb, ub)
%   Nudge the seed toward the feasible interior so that no nonlinear
%   constraint is active (== 0) at x0.  We binary-search along the line
%   from the seed toward the centre of the box until all constraints < 0.
    centre = (lb + ub) ./ 2 .* [0.5, 0.5];   % a point likely deep inside gamut
    x0 = x_seed;
    for attempt = 1 : 20
        [r, g, b] = color_to_rgb(x0(1), x0(2), hue, space);
        tol = 1e-6;
        if r > tol && r < 1-tol && ...
           g > tol && g < 1-tol && ...
           b > tol && b < 1-tol
            return;   % strictly inside — good seed
        end
        % Step 50 % of the remaining distance toward centre
        x0 = x0 * 0.5 + centre * 0.5;
    end
    % Last resort: use centre directly
    x0 = centre;
end

% -------------------------------------------------------------------------

function f = obj_func(x, w_l, w_c, L_MAX, C_MAX)
%   Maximize weighted sum of normalized L and C
    L_norm = x(1) / L_MAX;
    C_norm = x(2) / C_MAX;
    f = -(w_l * L_norm + w_c * C_norm);
end

function [c_con, ceq] = con_func(x, hue, space)
%   Smooth gamut constraint: each RGB channel must be in [0, 1].
%   Returns c_con <= 0 when satisfied so fmincon gets gradient info
%   from the actual distance to each channel boundary.
    [r, g, b] = color_to_rgb(x(1), x(2), hue, space);

    % Upper bounds: channel - 1 <= 0
    % Lower bounds: -channel  <= 0
    c_con = [r - 1; g - 1; b - 1; -r; -g; -b];
    ceq   = [];
end