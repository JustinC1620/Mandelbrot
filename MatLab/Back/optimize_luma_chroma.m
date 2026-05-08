function optimized_result = optimize_luma_chroma(rough_approx, weights)
%   Optimizes maximum luma and chroma values per hue
%   rough_approx: Matrix of initial lch values
%   weight: Weights for luma and chroma values in optimization

    % Defualt weights
    if nargin < 2
        weights = [0.5, 0.5];
    end
    
    % Normalize weights
    weights = weights / sum(weights);
    w_l = weights(1);
    w_c = weights(2);
    
    % Get number of hue points
    n_hues = size(rough_approx, 1);
    
    % Create initial guess: [l1, c1, l2, c2, ..., ln, cn]
    x0 = reshape([rough_approx(:,1), rough_approx(:,2)]', [], 1);
    
    % Create bounds
    lb = repmat([0, 0], 1, n_hues);
    ub = repmat([100, 145], 1, n_hues);
    
    % Extract hue values
    hue_values = rough_approx(:, 3);
    
    % Optimization options
    options = optimoptions('fmincon', ...
        'Display', 'off', ...
        'Algorithm', 'interior-point', ...
        'MaxIterations', 200, ...
        'OptimalityTolerance', 1e-6, ...
        'StepTolerance', 1e-12, ...
        'ConstraintTolerance', 1e-12, ...
        'MaxFunctionEvaluations', 1e6);
    
    % Objective function handle
    objective = @(x) obj_func(x, w_l, w_c);
    
    % Constraint function handle
    constraint = @(x) con_func(x, hue_values);
    
    % Run Optimization
    [x_opt, ~, exitflag] = fmincon(objective, x0, [], [], [], [], ...
                                  lb, ub, constraint, options);
    
    % Reshape result
    if exitflag > 0
        lc_pairs = reshape(x_opt, 2, [])';
        optimized_result = [lc_pairs, hue_values];
        
        % Verify all solutions are in gamut (vectorized check)
        [r, g, b] = lch_to_rgb(optimized_result(:,1), optimized_result(:,2), optimized_result(:,3));
        valid_mask = in_gamut(r, g, b);
        
        % Replace invalid solutions with original values
        optimized_result(~valid_mask, :) = rough_approx(~valid_mask, :);
    else
        % Fall back to original if optimization failed
        optimized_result = rough_approx;
    end
end

% Objective function
function f = obj_func(x, w_l, w_c)
%   Objective function to be minimized

    % Reshape x to [n_hues x 2] matrix where each row is [L, C]
    lc_pairs = reshape(x, 2, [])';
    
    % Extract L and C vectors
    L_vec = lc_pairs(:, 1);
    C_vec = lc_pairs(:, 2);
    
    % Normalize L and C to [0,1]
    L_norm = L_vec / 100;   % L typically ranges 0-100
    C_norm = C_vec / 145;   % C typically ranges 0-145
    
    % Objective: maximize weighted sum
    objectives = -(w_l * L_norm + w_c * C_norm);
    f = sum(objectives);
end

% Constraint function
function [c, ceq] = con_func(x, hue_values)
%   Constraints of objective function

    n_hues = length(hue_values);
    
    % Reshape x to [n_hues x 2] matrix
    lc_pairs = reshape(x, 2, [])';
    
    % Extract L and C vectors
    L_vec = lc_pairs(:, 1);
    C_vec = lc_pairs(:, 2);
    H_vec = hue_values;
    
    % RGB conversion and gamut check
    [r, g, b] = lch_to_rgb(L_vec, C_vec, H_vec);
    valid_mask = in_gamut(r, g, b);
    
    % Create constraint vector
    c = zeros(n_hues, 1);
    c(valid_mask) = -1;
    c(~valid_mask) = 1;
    
    ceq = []; % No equality constraints
end