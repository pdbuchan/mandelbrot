# Mandelbrot Set C Language Examples

Each file here is an independent C language program that will produce a bitmap image rendering of a user-specified area of the Mandelbrot set, with the horizontal and vertical pixel dimensions and maximum number of iterations per point also specified by the user. Many of these programs have constants defined/set near the top of the program, and the user is encouraged to adjust values to see how the results are affected.

Each program demonstrates a different palette implementation in which different methods are used to assign colors to points assessed to be outside the set. In each case, points assessed as being within the Mandelbrot set are set to black.

## List of Programs

| File | Description |
|---|---|
| `basic.c` | Exterior colors are assigned based upon integer iteration count indexing an external palette file. |
| `palette` | Palette text file to be used with `basic.c`. |
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

## Coordinates of Some Interesting Areas of the Mandelbrot Set

| Real Coordinate of Center | Imaginary Coordinate of Center | Horizontal Span on Real Axis | Description |
|---|---|---|---|
| `-0.7` | `0.0` | `3.0769`  | Whole Mandelbrot set |
| `-0.87591` | `0.20464` | 0.53184 | Seahorse valley |
| `-0.759856` | `0.125547` | `0.051579` | Spirals and seahorses |
| `-0.74303` | `0.126433` | `0.01611` | Upside-down seahorse |
| `-0.7435669` | `0.1314023` | `0.0022878` | Seahorse tail |
| `-0.7436499` | `0.13188204` | `0.00073801` | Part of a tail |
| `-0.74364085` | `0.13182733` | `0.00012068` | Satellite |
| `-0.743643135` | `0.131825963` | `0.000014628` | Crowns and tails |
| `-0.743644786` | `0.1318252536` | `0.0000029336` | Antenna |
