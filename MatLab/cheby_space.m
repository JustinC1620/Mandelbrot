function y = cheby_space(a,b,n)
%   Creates an arrary of Chebyshev nodes
%   a: Starting value
%   b: Ending value
%   n: Number of nodes
    k = n - 1 : - 1 : 0;
    y = (a + b) ./ 2 + (( b - a) ./ 2) * cos(pi .* k ./ (n - 1));
end