/*  Copyright (C) 2026 P.D. Buchan (pdbuchan@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Mandelbrot Generator - Produce a bitmap file of user-specified area of
//                        the Mandelbrot set.
//                      - Determines color via linear interpolation.

// Compile: gcc -Wall linear.c -o linear -lm
// Usage: ./linear

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <errno.h>

static const long double CYCLE_RATE = 0.01L;  // Palette cycles per smooth iteration (e.g., CYCLE_RATE = 0.01 gives period = 100 iterations)
#define MAX_STRINGLEN 256  // Maximum length of a character string

// RGB color
typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} RGB;

// Function prototypes
int inputtext (char *);
RGB linear_interp (RGB, RGB, long double);
RGB linear_gradient_palette (long double);
void write_u16_le (FILE *, uint16_t);
void write_u32_le (FILE *, uint32_t);
void write_s32_le (FILE *, int32_t);

int
main (void) {

  size_t iterations, maxit, width, height, row_size, image_size, x, y;
  long double rc, ic, span, u, v, stepu, stepv, r1, r2, i1, i2, log_zn, smooth;
  uint8_t padding[3] = {0, 0, 0};  // Padding to make each row 4 bytes aligned
  char temp[MAX_STRINGLEN], *endptr;
  RGB color;
  FILE *fo;

  // Ask for real coordinate of center of plot.
  fprintf (stdout, "\nWhat is the real coordinate of the center? ");
  memset (temp, 0, MAX_STRINGLEN * sizeof (char));
  inputtext (temp);
  errno = 0; 
  rc = (long double) strtold (temp, &endptr);
  if ((errno == ERANGE) || (endptr == temp) || (*endptr != '\0')) {
    fprintf (stderr, "Cannot make type long double of: %s\n", temp);
    exit (EXIT_FAILURE);
  }

  // Ask for imaginary coordinate of center of plot.
  fprintf (stdout, "\nWhat is the imaginary coordinate of the center? ");
  memset (temp, 0, MAX_STRINGLEN * sizeof (char));
  inputtext (temp);
  errno = 0;
  ic = (long double) strtold (temp, &endptr);
  if ((errno == ERANGE) || (endptr == temp) || (*endptr != '\0')) {
    fprintf (stderr, "Cannot make type long double of: %s\n", temp);
    exit (EXIT_FAILURE);
  }

  // Ask for horizontal span on real axis.
  fprintf (stdout, "\nWhat is the horizontal span (on real axis)? ");
  memset (temp, 0, MAX_STRINGLEN * sizeof (char));
  inputtext (temp);
  errno = 0;
  span = strtold (temp, &endptr);
  if ((errno == ERANGE) || (endptr == temp) || (span <= 0.0L) || (*endptr != '\0')) {
    fprintf (stderr, "Cannot make a positive type long double of: %s\n", temp);
    exit (EXIT_FAILURE);
  }

  // Ask for image width (px).
  fprintf (stdout, "\nWhat is the width (px)? ");
  memset (temp, 0, MAX_STRINGLEN * sizeof (char));
  inputtext (temp);
  errno = 0;
  width = (size_t) strtoull (temp, &endptr, 10);
  if ((errno == ERANGE) || (endptr == temp) || (width == 0) || (*endptr != '\0')) {
    fprintf (stderr, "Cannot make a positive size_t of: %s\n", temp);
    exit (EXIT_FAILURE);
  }

  if (width > INT32_MAX) {
    fprintf (stderr, "Image horizontal dimension is too large for BMP format.\n");
    exit (EXIT_FAILURE);
  }

  // Ask for image height (px).
  fprintf (stdout, "\nWhat is the height (px)? ");
  memset (temp, 0, MAX_STRINGLEN * sizeof (char));
  inputtext (temp);
  errno = 0;
  height = (size_t) strtoull (temp, &endptr, 10);
  if ((errno == ERANGE) || (endptr == temp) || (height == 0) || (*endptr != '\0')) {
    fprintf (stderr, "Cannot make a positive size_t of: %s\n", temp);
    exit (EXIT_FAILURE);
  }

  if (height > INT32_MAX) {
    fprintf (stderr, "Image vertical dimension is too large for BMP format.\n");
    exit (EXIT_FAILURE);
  }

  // Ask for maximum allowable number of iterations.
  fprintf (stdout, "\nWhat is the maximum allowable number of iterations? ");
  memset (temp, 0, MAX_STRINGLEN * sizeof (char));
  inputtext (temp);
  errno = 0;
  maxit = (size_t) strtoull (temp, &endptr, 10);
  if ((errno == ERANGE) || (endptr == temp) || (maxit == 0) || (*endptr != '\0')) {
    fprintf (stderr, "Cannot make a positive size_t of: %s\n", temp);
    exit (EXIT_FAILURE);
  }

  // Determine the coordinate spacing between adjacent pixel centers.
  // Use the same spacing on both axes so the complex plane is not distorted.
  stepu = span / (long double) width;
  stepv = stepu;

  // Check to see if we have reached the limit of precision.
  if ((rc + stepu == rc) || (ic + stepv == ic)) {
    fprintf (stderr, "Pixel spacing is too small for long double precision.\n");
    exit (EXIT_FAILURE);
  }

  // Calculate the padding required for each row.
  // BMP rows must contain a multiple of four bytes.
  if (width > (SIZE_MAX - 3) / 3) {
    fprintf (stderr, "Image horizontal dimension is too large for BMP format.\n");
    exit (EXIT_FAILURE);
  }
  row_size = (width * 3 + 3) & ~(size_t) 3;

  // The BMP file-size and image-size fields are unsigned 32-bit values.
  if (height > ((UINT32_MAX - 54) / row_size)) {
    fprintf (stderr, "Size of image is too large for BMP format.\n");
    exit (EXIT_FAILURE);
  }
  image_size = row_size * height;

  // Open output file.
  fo = fopen ("output.bmp", "r");
  if (fo != NULL) {
    printf ("Output file output.bmp already exists.\n");
    fclose (fo);
    exit (EXIT_FAILURE);
  }
  fo = fopen ("output.bmp", "wb");
  if (fo == NULL) {
    printf ("Can't open new bitmap file.\n");
    exit (EXIT_FAILURE);
  }

  // BMP file header
  write_u16_le (fo, 0x4d42);           // File type, should be "BM"
  write_u32_le (fo, 54 + (uint32_t) image_size);  // Size of the file (bytes)
  write_u16_le (fo, 0);                // Reserved (set to 0)
  write_u16_le (fo, 0);                // Reserved (set to 0)
  write_u32_le (fo, 54);               // Offset (bytes) to the start of the pixel data

  // BMP information header
  write_u32_le (fo, 40);               // Size of this header (40 bytes)
  write_s32_le (fo, (int32_t) width);   // Width of the image (px)
  write_s32_le (fo, (int32_t) height);  // Height of the image (px)
  write_u16_le (fo, 1);                // Number of color planes (always 1)
  write_u16_le (fo, 24);               // Bits per pixel (24 for RGB)
  write_u32_le (fo, 0);                // Compression method (0 for none)
  write_u32_le (fo, (uint32_t) image_size);  // Size of the image data (bytes)
  write_s32_le (fo, 7874);             // Horizontal resolution (in pixels per meter) (200 DPI)
  write_s32_le (fo, 7874);             // Vertical resolution (in pixels per meter) (200 DPI)
  write_u32_le (fo, 0);                // Number of colors used (0 for 2^24)
  write_u32_le (fo, 0);                // Important colors (0 for all)

  // Iterate on each point in the (u, v) plane.
  // Pixels are in the (x, y) plane and are sampled at their centers.
  // Because BMP rows are written from bottom to top for a positive height,
  // y = 0 corresponds to the lowest imaginary coordinate in the image.
  for (y = 0; y < height; y++) {
    v = ic + ((long double) y + 0.5L - (long double) height / 2.0L) * stepv;
    for (x = 0; x < width; x++) {
      u = rc + ((long double) x + 0.5L - (long double) width / 2.0L) * stepu;

      // Iterate until either maxit is reached or |z| > 2.
      r1 = 0.0L;
      i1 = 0.0L;
      iterations = 0;
      while ((((r1 * r1) + (i1 * i1)) <= 4.0L) && (iterations < maxit)) {
        r2 = (r1 * r1) - (i1 * i1) + u;
        i2 = (2.0L * i1 * r1) + v;
        r1 = r2;
        i1 = i2;
        iterations++;
      }

      // Calculate a continuous (normalized) iteration count.
      if (((r1 * r1) + (i1 * i1)) > 4.0L) {
        log_zn = logl ((r1 * r1) + (i1 * i1)) / 2.0L;
        smooth = (long double) iterations + 1.0L - logl (log_zn) / logl (2.0L);

        // If the escape criterion is reached, the point is outside the set;
        // calculate its smooth iteration count and assign a color.
        color = linear_gradient_palette (smooth * CYCLE_RATE);
        fputc (color.b, fo);
        fputc (color.g, fo);
        fputc (color.r, fo);

      // Otherwise, the point has not escaped within maxit iterations;
      // treat it as belonging to the set and plot it as black.
      } else {
        fputc (0, fo);
        fputc (0, fo);
        fputc (0, fo);

      }  // End if escape criterion is reached.

    }

    // Write padding, if necessary.
    fwrite (padding, 1, row_size - width * 3, fo);
  }

  // Close file descriptor.
  fclose (fo);

  return (EXIT_SUCCESS);
}

// Obtain a text string from standard input. It can include spaces.
int
inputtext (char *text) {

  size_t len;

  if (fgets (text, MAX_STRINGLEN, stdin) == NULL) {
    fprintf (stderr, "Unable to read text from standard input.\n");
    exit (EXIT_FAILURE);
  }

  len = strlen (text);
  if (len > 0 && text[len - 1] == '\n') {
    text[len - 1] = '\0';
  } else if (len == MAX_STRINGLEN - 1) {
    int ch;
    while ((ch = getchar ()) != '\n' && ch != EOF) {
      // Discard the remainder of an overlong input line.
    }
    fprintf (stderr, "Input text is too long; maximum is %d characters.\n", MAX_STRINGLEN - 2);
    exit (EXIT_FAILURE);
  }

  return (EXIT_SUCCESS);
}

// Linear interpolation palette.
RGB
linear_gradient_palette (long double t) {

  size_t i;
  long double scaled, frac;
  static const RGB colors[] = {
    {0, 7, 100},      // Dark blue
    {32, 107, 203},   // Blue
    {237, 255, 255},  // White
    {255, 170, 0},    // Orange
    {0, 2, 0}         // Black
  };

  const size_t n = sizeof (colors) / sizeof (colors[0]);

  t = fmodl (t, 1.0L); // Wrap around.

  scaled = t * (n - 1);
  i = (size_t) scaled;
  frac = scaled - i;

  return (linear_interp (colors[i], colors[i + 1], frac));
}

// Linearly interpolate between colors a and b.
// t = 0.0 returns color a, t = 1.0 returns color b,
// and intermediate values blend each RGB component independently.
RGB
linear_interp (RGB a, RGB b, long double t) {

  RGB c;

  // Interpolate each color channel and round to the nearest 8-bit value.
  c.r = (uint8_t) lroundl (a.r + (b.r - a.r) * t);
  c.g = (uint8_t) lroundl (a.g + (b.g - a.g) * t);
  c.b = (uint8_t) lroundl (a.b + (b.b - a.b) * t);

  return (c);
}

// Write a unsigned little-endian 16-bit value to file.
void
write_u16_le (FILE *fo, uint16_t val) {

  fputc (val & 0xff, fo);
  fputc ((val >> 8) & 0xff, fo);

}

// Write a unsigned little-endian 32-bit value to file.
void
write_u32_le (FILE *fo, uint32_t val) {

  fputc (val & 0xff, fo);
  fputc ((val >> 8) & 0xff, fo);
  fputc ((val >> 16) & 0xff, fo);
  fputc ((val >> 24) & 0xff, fo);

}

// Write a signed little-endian 32-bit value to file.
void
write_s32_le (FILE *fo, int32_t val) {

  write_u32_le (fo, (uint32_t) val);  // Cast as unsigned in order to preserve bit pattern.

}
