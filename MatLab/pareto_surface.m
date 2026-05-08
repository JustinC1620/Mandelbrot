% pareto_surface.m
% Computes the Pareto frontier of (L, C) for a grid of hue angles and
% luma/chroma weight combinations, then plots a 3D surface per space.
%
% Axes:  X = Hue (0-360°)
%        Y = Weight on Luma  (0 = all chroma, 1 = all luma)
%        Z = Normalized L or C at the optimum

SPACE     = 'oklch';   % 'lch' or 'oklch'
N_HUES    = 360;       % hue resolution (more = slower but smoother)
N_WEIGHTS = 21;        % number of weight steps from 0 to 1 (inclusive)
N_BISECT  = 32;        % bisection iterations for gamut boundary (~1e-10 resolution)
N_LUMAS   = 360;       % L resolution for boundary tracing per hue

% -------------------------------------------------------------------------
% Bounds
% -------------------------------------------------------------------------
switch lower(SPACE)
    case 'lch'
        L_MAX = 100;
        C_MAX = 145;
    case 'oklch'
        L_MAX = 1;
        C_MAX = 0.4;
    otherwise
        error("Unknown space '%s'.", SPACE);
end

% -------------------------------------------------------------------------
% Grids
% -------------------------------------------------------------------------
hue_vec    = linspace(0, 360, N_HUES);
weight_vec = linspace(0, 1,   N_WEIGHTS);   % w_L; w_C = 1 - w_L
l_vec      = linspace(0, L_MAX, N_LUMAS);

fprintf("=== Pareto surface: %s ===\n", upper(SPACE));
fprintf("Tracing gamut boundary for %d hues x %d L levels...\n", N_HUES, N_LUMAS);

% -------------------------------------------------------------------------
% Step 1: trace the gamut boundary C(L, H) for every (L, H) pair.
%         Vectorised over all hues simultaneously, bisecting C.
% -------------------------------------------------------------------------
tic;

L_mat = repmat(l_vec(:),   1, N_HUES);   % (N_LUMAS x N_HUES)
H_mat = repmat(hue_vec(:)', N_LUMAS, 1); % (N_LUMAS x N_HUES)

lo = zeros(N_LUMAS, N_HUES);
hi = repmat(C_MAX,  N_LUMAS, N_HUES);

for k = 1 : N_BISECT
    mid = (lo + hi) / 2;
    [r, g, b] = color_to_rgb(L_mat, mid, H_mat, SPACE);
    in = in_gamut(r, g, b);
    lo( in) = mid( in);
    hi(~in) = mid(~in);
end

% Mark rows where even C=0 is out of gamut (over-bright L values)
[r0, g0, b0] = color_to_rgb(L_mat, zeros(N_LUMAS, N_HUES), H_mat, SPACE);
valid = in_gamut(r0, g0, b0);
C_boundary = lo;
C_boundary(~valid) = NaN;

fprintf("Boundary traced in %.2f s\n", toc);

% -------------------------------------------------------------------------
% Step 2: for each (hue, weight) pair find the Pareto-optimal (L*, C*)
%         by maximising  w_L*(L/L_MAX) + w_C*(C/C_MAX)  over the boundary.
% -------------------------------------------------------------------------
fprintf("Computing Pareto optima for %d weights...\n", N_WEIGHTS);
tic;

% Preallocate result grids  (N_WEIGHTS x N_HUES)
L_opt = zeros(N_WEIGHTS, N_HUES);
C_opt = zeros(N_WEIGHTS, N_HUES);

L_norm_mat = L_mat / L_MAX;   % (N_LUMAS x N_HUES)
C_norm_mat = C_boundary / C_MAX;

for wi = 1 : N_WEIGHTS
    w_L = weight_vec(wi);
    w_C = 1 - w_L;

    obj = w_L .* L_norm_mat + w_C .* C_norm_mat;   % NaN where invalid
    obj(isnan(C_boundary)) = -Inf;

    [~, best_idx] = max(obj, [], 1);   % best L index per hue

    for hi_idx = 1 : N_HUES
        bi = best_idx(hi_idx);
        L_opt(wi, hi_idx) = l_vec(bi);
        C_opt(wi, hi_idx) = C_boundary(bi, hi_idx);
    end
end

% Normalize for plotting so both surfaces share a [0,1] colour scale
L_opt_norm = L_opt / L_MAX;
C_opt_norm = C_opt / C_MAX;

fprintf("Pareto optima computed in %.2f s\n", toc);

% -------------------------------------------------------------------------
% Step 3: plot
%         Surface 1 — optimal L (normalized)
%         Surface 2 — optimal C (normalized)
%         Both over (Hue x w_L) grid, coloured by value
% -------------------------------------------------------------------------
[H_grid, W_grid] = meshgrid(hue_vec, weight_vec);

fig = figure('Name', sprintf('%s Pareto Surface', upper(SPACE)), ...
             'Position', [100 100 1300 560]);

% --- Luma surface ---
ax1 = subplot(1, 2, 1);
surf(H_grid, W_grid, L_opt_norm, ...
     'EdgeColor', 'none', 'FaceAlpha', 0.92);
colormap(ax1, turbo(256));
clim([0 1]);
cb1 = colorbar;
cb1.Label.String = 'Normalized L  (L / L_{max})';
xlabel('Hue (°)');
ylabel('w_L  (luma weight)');
zlabel('Normalized L*');
title(sprintf('%s — Optimal L', upper(SPACE)));
xlim([0 360]);
ylim([0 1]);
zlim([0 1]);
view(-40, 30);
grid on;
lighting gouraud;
camlight('headlight');

% --- Chroma surface ---
ax2 = subplot(1, 2, 2);
surf(H_grid, W_grid, C_opt_norm, ...
     'EdgeColor', 'none', 'FaceAlpha', 0.92);
colormap(ax2, turbo(256));
clim([0 1]);
cb2 = colorbar;
cb2.Label.String = 'Normalized C  (C / C_{max})';
xlabel('Hue (°)');
ylabel('w_L  (luma weight)');
zlabel('Normalized C*');
title(sprintf('%s — Optimal C', upper(SPACE)));
xlim([0 360]);
ylim([0 1]);
zlim([0 1]);
view(-40, 30);
grid on;
lighting gouraud;
camlight('headlight');

sgtitle(sprintf('%s Pareto Frontier Surface — L vs C weight trade-off', upper(SPACE)), ...
        'FontSize', 13);

% -------------------------------------------------------------------------
% Step 4: bonus — slice plot at the cusp hue (~263° for OKLCH)
% -------------------------------------------------------------------------
[~, cusp_hi] = min(abs(hue_vec - 263));
cusp_hue     = hue_vec(cusp_hi);

fig2 = figure('Name', sprintf('%s Pareto Slice at H=%.0f°', upper(SPACE), cusp_hue), ...
              'Position', [100 700 700 420]);
ax3 = axes(fig2);
hold(ax3, 'on');

cmap = turbo(N_WEIGHTS);
for wi = 1 : N_WEIGHTS
    % Draw one point per weight on the (C, L) boundary for this hue
    scatter(ax3, C_opt(wi, cusp_hi) / C_MAX, ...
                 L_opt(wi, cusp_hi) / L_MAX, ...
                 60, cmap(wi,:), 'filled');
end

% Overlay the full gamut boundary at this hue for context
c_bnd = C_boundary(:, cusp_hi);
l_bnd = l_vec(:);
valid_bnd = ~isnan(c_bnd);
plot(ax3, c_bnd(valid_bnd) / C_MAX, l_bnd(valid_bnd) / L_MAX, ...
     'k-', 'LineWidth', 1.2, 'DisplayName', 'Gamut boundary');

cb3 = colorbar(ax3);
colormap(ax3, turbo(256));
clim(ax3, [0 1]);
cb3.Label.String = 'w_L  (0 = all chroma,  1 = all luma)';
xlabel(ax3, 'Normalized C  (C / C_{max})');
ylabel(ax3, 'Normalized L  (L / L_{max})');
title(ax3, sprintf('%s Pareto frontier at H = %.0f°  (cusp region)', ...
      upper(SPACE), cusp_hue));
grid(ax3, 'on');
hold(ax3, 'off');