% Constants
HUES = 360;
STEPS = 720;

% Generate rough approximation
disp("Started Rough Luma and Chroma Approximation.");
tic;
rough_approx = rough_luma_chroma(STEPS);
t =  toc; 
disp(['Finished Rough Luma and Chroma Approximation in: ',...
    num2str(t), ' seconds']);

% Optimize approximation
disp("Started Luma and Chroma Optimization.");
tic;
optimized_approx = zeros(HUES, 3);
parfor i = 1 : STEPS
    optimized_approx(i, :) = optimize_luma_chroma(rough_approx(i, :),...
        [0.5 0.5]);
end
t = toc; 
disp(['Finished Luma and Chroma Optimization in: ',...
    num2str(t), ' seconds']);

% Retrieve results
l = optimized_approx(:, 1);
c = optimized_approx(:, 2);
h = optimized_approx(:, 3);

% Fit polynomial and evaluate
H = 0 : 0.25 : HUES;
[c_poly, c_s, c_mu] = polyfit(h, c, 12);
[c_fit, ~] = polyval(c_poly, H, c_s, c_mu);
[l_poly, l_s, l_mu] = polyfit(h, l, 12);
[l_fit, ~] = polyval(l_poly, H, l_s, l_mu);

% Plot results
plot(optimized_approx(:, 3), optimized_approx(:, 1), '.');
hold on;
plot(optimized_approx(:, 3), optimized_approx(:, 2), '.');
plot(H, l_fit, 'LineWidth', 1.5);
plot(H, c_fit, 'LineWidth', 1.5);
legend('Luma Sample', 'Chroma Sample', 'Luma Poly', 'Chroma Poly');
xlabel('Hue');
ylabel('Luma and Chroma Values');
xlim([0 360]);
hold off;

% Record Values
dfile ='lch_poly_approx.dia';
if exist(dfile, 'file') ; delete(dfile); end
diary(dfile)
format compact;
format longG;
diary on;
disp("Luma Poly Coefficients:"), disp(l_poly');
disp("Chroma Poly Coefficients:"), disp(c_poly');
disp("Mu:"), disp(c_mu(1));
disp("Sigma:"), disp(c_mu(2));
diary off;