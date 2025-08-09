# Mandelbrot
Arbitray precision Mandelbrot set viewer made with OpenGL.

Uses the escape algorithm with periodicity checking to determin if a point is within the set.

Color is generated in LCH space using a renormalized fractional escape iteration to detemin the hue. The luminance and chroma a determined by a polynomial approximation of the maximal values of each while remain in the RGB gamut.

## Controls
Use the arrow keys to move the camera. 

Shift and control zoom in and out respectively.

## Settings
The default resolution and anti aliasing setting can be adjusted in utils.h

## Examples
![Example 1](https://github.com/JustinC1620/Mandelbrot/blob/master/Examples/example_1.png)
![Example 2](https://github.com/JustinC1620/Mandelbrot/blob/master/Examples/example_2.png)
![Example 3](https://github.com/JustinC1620/Mandelbrot/blob/master/Examples/example_3.png)
![Example 4](https://github.com/JustinC1620/Mandelbrot/blob/master/Examples/example_4.png)
![Example 5](https://github.com/JustinC1620/Mandelbrot/blob/master/Examples/example_5.png)
![Example 6](https://github.com/JustinC1620/Mandelbrot/blob/master/Examples/example_6.png)
