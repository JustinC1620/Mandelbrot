function rough_approx = rough_luma_chroma(num_steps)
%  rough_luma_chroma Approximates rough max luma and chroma
%  num_steps: Number of steps across hue space
    
    % Constants
    HUES = 360;
    LUMAS = 100;
    CHROMAS = 145;
    BLOCK = 64;
    
    % Generate values for hue, luma, and chroma
    h = cheby_space(0, HUES, num_steps);
    l = 0 : (LUMAS) / (num_steps - 1) : LUMAS;
    c = 0 : (CHROMAS) / (num_steps - 1) : CHROMAS;

    % Allocate memory for result
    num_blocks = ceil(num_steps / BLOCK);
    rough_approx = zeros(BLOCK, 3, num_blocks);
    
    % Calcuate rough approximation 
    parfor i = 1 : num_blocks
        % Hue slice for block
        start = (i - 1) * BLOCK + 1;
        stop = start + BLOCK - 1;
        if (stop > num_steps)
            stop = num_steps;
        end
        
        % Mesh of luma, chroma, and hue values
        [L, C, H] = meshgrid(l, c, h(start : stop));
        
        % Check what values are in rgb gamut
        [r, g, b] = lch_to_rgb(L, C, H);
        I = in_gamut(r, g, b);
        
        % Mask mesh
        C_masked = C;
        C_masked(~I) = -Inf;
        
        % Get max chroma values
        [~, ~, n] = size(C_masked);
        [c_i, linear_indices] = max(reshape(C_masked, [], n), [], 1);
        c_i = c_i';
        
        % Get max luma based on chroma
        L_reshaped = reshape(L, [], n);
        l_i = zeros(n, 1);
        for j = 1 : n
            l_i(j) = L_reshaped(linear_indices(j), j);
        end
        
        % Create lch matrix
        h_i = H(1, 1, :);
        h_i = h_i(:);
        lch = [l_i, c_i, h_i];
        
        % Pad lch matrix and store
        zeros(BLOCK - size(lch, 1), 3);
        rough_approx(:, :, i) = [lch; zeros(BLOCK - size(lch, 1), 3)];
    end
    
    % Resahpe rough_approx to 2D-matrix
    rough_approx = reshape(permute(rough_approx, [1 3 2]),...
        [], size(rough_approx, 2));
    rough_approx = rough_approx(1 : num_steps, :);
end