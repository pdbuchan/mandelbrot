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
//                      - Colors exterior points from their smooth iteration
//                        count using D.A. Green's Cubehelix color scheme.
//                      - Traverses Cubehelix from black to white and back so
//                        successive palette cycles join continuously.

// Compile: gcc -Wall cubehelix.c -o cubehelix -lm
// Usage: ./cubehelix

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <errno.h>

static const long double CYCLE_RATE = 0.01L;  // Palette cycles per smooth iteration (e.g., CYCLE_RATE = 0.01 gives period = 100 iterations)
static const long double PI = 3.141592653589793238462643383279502884L;
static const long double CUBEHELIX_START = 0.5L;      // Starting color: 0.5 corresponds to purple
static const long double CUBEHELIX_ROTATIONS = -1.5L; // Number and direction of color rotations
static const long double CUBEHELIX_HUE = 1.2L;        // Amplitude of color deviation from greyscale
static const long double CUBEHELIX_GAMMA = 1.0L;      // Gamma correction applied to intensity
#define MAX_STRINGLEN 256  // Maximum length of a character string

// RGB color
typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} RGB;

// Function prototypes
int inputtext (char *);
RGB cubehelix_palette (long double);
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
        color = cubehelix_palette (smooth * CYCLE_RATE);
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

  int ch;
  size_t len;

  if (fgets (text, MAX_STRINGLEN, stdin) == NULL) {
    fprintf (stderr, "Unable to read text from standard input.\n");
    exit (EXIT_FAILURE);
  }

  len = strlen (text);

  // Remove trailing newline, and a preceding carriage return if present.
  if ((len > 0) && (text[len - 1] == '\n')) {
    text[--len] = '\0';
    if ((len > 0) && (text[len - 1] == '\r')) {
      text[--len] = '\0';
    }
    return (EXIT_SUCCESS);
  }

  // If the buffer is full, determine whether the input was exactly
  // MAX_STRINGLEN - 1 characters or was genuinely too long.
  if (len == MAX_STRINGLEN - 1) {

    ch = getchar ();

    // Exactly MAX_STRINGLEN - 1 characters followed by newline or EOF.
    if ((ch == '\n') || (ch == EOF)) {
      return (EXIT_SUCCESS);
    }

    // Handle CRLF after an exactly full input line.
    if (ch == '\r') {
      ch = getchar ();
      if ((ch == '\n') || (ch == EOF)) {
        return (EXIT_SUCCESS);
      }
    }

    // Discard the remainder of an overlong input line.
    while ((ch != '\n') && (ch != EOF)) {
      ch = getchar ();
    }

    fprintf (stderr, "Input text is too long; maximum is %d characters.\n",
             MAX_STRINGLEN - 1);
    exit (EXIT_FAILURE);
  }

  return (EXIT_SUCCESS);
}

// Cubehelix palette.
//
// This implements the color construction described by D.A. Green (2011),
// "A colour scheme for the display of astronomical intensity images"
// (arXiv:1108.5083). Cubehelix follows a tapered helix around the black-to-
// white diagonal of the RGB color cube while accounting approximately for
// the unequal perceived brightness of red, green, and blue.
//
// Green's original parameter lambda runs monotonically from black (0) to
// white (1). For a repeating Mandelbrot palette, the wrapped palette
// coordinate is mapped smoothly 0 -> 1 -> 0, so each cycle goes from black
// to white and back to black without a discontinuity.
RGB
cubehelix_palette (long double t) {

  long double lambda, lambda_gamma, amplitude, phi, r, g, b;
  RGB color;

  // Wrap into one cycle, then traverse Cubehelix forward and backward.
  t -= floorl (t);
  lambda = 0.5L * (1.0L - cosl (2.0L * PI * t));

  lambda_gamma = powl (lambda, CUBEHELIX_GAMMA);
  phi = 2.0L * PI * (CUBEHELIX_START / 3.0L + CUBEHELIX_ROTATIONS * lambda);
  amplitude = CUBEHELIX_HUE * lambda_gamma * (1.0L - lambda_gamma) / 2.0L;

  // Equation (2) of Green (2011).
  r = lambda_gamma + amplitude * (-0.14861L * cosl (phi) + 1.78277L * sinl (phi));
  g = lambda_gamma + amplitude * (-0.29227L * cosl (phi) - 0.90649L * sinl (phi));
  b = lambda_gamma + amplitude * ( 1.97294L * cosl (phi));

  // Clip any small excursions outside the RGB cube.
  r = fminl (1.0L, fmaxl (0.0L, r));
  g = fminl (1.0L, fmaxl (0.0L, g));
  b = fminl (1.0L, fmaxl (0.0L, b));

  color.r = (uint8_t) lroundl (255.0L * r);
  color.g = (uint8_t) lroundl (255.0L * g);
  color.b = (uint8_t) lroundl (255.0L * b);

  return (color);
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
