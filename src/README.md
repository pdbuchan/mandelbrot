# Mandelbrot Set C Language Examples

Each file here is an independent C language program that will produce a bitmap image rendering of the user-specified area of the Mandelbrot set, with the horizontal and vertical pixel dimensions and maximum number of iterations per point also specified by the user. Many of these programs have constants defined/set near the top of the program, and the user is encouraged to adjust values to see how the image is affected.

Each program demonstrates a different palette implementation in which different methods are used to assign colors to points assessed to be outside the set. In each case, points assessed as being within the Mandelbrot set are set to black.

## 

| File | Description |
|---|---|
| `basic.c` | Exterior colors are assigned based upon integer iteration count indexing an external palette file. |
| `palette` | Palette file to be used with `basic.c`. |
| `cosine.c` | Exterior colors are assigned via a cosine method using the smooth iteration count. |
| `linear.c` | Exterior colors are derived from the smooth iteration count by linearly interpolating between a base set of RGB colors. |
| `smoothstep.c` | Exterior colors are derived from the smooth iteration count using cubic smoothstep interpolation between a cyclic sequence of RGB colors. |
| `bezier.c` | Exterior colors are derived from the smooth iteration count by cyclically traversing a Bezier palette. |
| `warm_power.c` | Exterior colors are derived from the smooth iteration count using a warm-colored RGB power-law palette. |
| `fire.c` | Exterior colors are derived from the smooth iteration count using a cyclic "Fire" palette. |
| `hsv2rgb.c` | Exterior colors are derived by mapping the smooth iteration count cyclically around the HSV hue weheel at full saturation and value, then converting to RGB. |
| `gaussian.c` | Exterior colors are derived from the smooth iteration count using three phase-shifted Gaussian functions, one for each RGB channel. |
| `cubehelix.c` | Exterior colors are derived from the smooth iteration count using D.A. Green's Cubehelix color scheme. |
| `spectrum.c` | Exterior colors are derived from the smooth iteration count using an approximate visible-light spectrum from violet through red. |

