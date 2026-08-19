/*  Copyright (C) 2012-2026 P.D. Buchan (pdbuchan@yahoo.com)

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
//                      - Uses an external palette file for colors.

// Compile: gcc -Wall basic.c -o basic
// Usage: ./basic

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <errno.h>

#define MAX_STRINGLEN 256  // Maximum length of a character string

// RGB color
typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} RGB;

// Function prototypes
int inputtext (char *);
void write_u16_le (FILE *, uint16_t);
void write_u32_le (FILE *, uint32_t);
void write_s32_le (FILE *, int32_t);

int
main (void) {

  size_t index, ncolor, iterations, maxit, width, height, row_size, image_size, x, y;
  long double rc, ic, span, u, v, stepu, stepv, r1, r2, i1, i2;
  uint8_t padding[3] = {0, 0, 0};  // Padding to make each row 4 bytes aligned
  char temp[MAX_STRINGLEN], *endptr;
  size_t palette_capacity;
  RGB *palette, *tmp_palette;
  FILE *fp, *fo;

  // Open color palette.
  fp = fopen ("palette", "r");
  if (fp == NULL) {
    printf ("Can\'t open palette file.\n");
    exit (EXIT_FAILURE);
  }

  // Read the color palette, growing its memory allocation as necessary.
  ncolor = 0;
  palette_capacity = 16;
  palette = malloc (palette_capacity * sizeof (RGB));
  if (palette == NULL) {
    fprintf (stderr, "Cannot allocate memory for color palette.\n");
    fclose (fp);
    exit (EXIT_FAILURE);
  }

  while (1) {
    RGB color;
    int nread;

    nread = fscanf (fp, "%hhu %hhu %hhu", &color.r, &color.g, &color.b);
    if (nread == EOF) {
      break;
    }
    if (nread != 3) {
      fprintf (stderr, "Invalid color entry in palette file.\n");
      free (palette);
      fclose (fp);
      exit (EXIT_FAILURE);
    }

    if (ncolor == palette_capacity) {
      if (palette_capacity > (SIZE_MAX / 2 / sizeof (RGB))) {
        fprintf (stderr, "Color palette is too large to process.\n");
        free (palette);
        fclose (fp);
        exit (EXIT_FAILURE);
      }

      palette_capacity *= 2;
      tmp_palette = realloc (palette, palette_capacity * sizeof (RGB));
      if (tmp_palette == NULL) {
        fprintf (stderr, "Cannot increase memory allocation for color palette.\n");
        free (palette);
        fclose (fp);
        exit (EXIT_FAILURE);
      }
      palette = tmp_palette;
    }

    palette[ncolor] = color;
    ncolor++;
  }
  fclose (fp);

  if (ncolor == 0) {
    fprintf (stderr, "Palette file contains no colors.\n");
    free (palette);
    exit (EXIT_FAILURE);
  }

  // Reduce the allocation to the number of colors actually read.
  tmp_palette = realloc (palette, ncolor * sizeof (RGB));
  if (tmp_palette != NULL) {
    palette = tmp_palette;
  }

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

  // Iterate on each point z in the (u, v) plane.
  // Pixels are in the (x, y) plane and are sampled at their centers.
  // Because BMP rows are written from bottom to top for a positive height,
  // y = 0 corresponds to the lowest imaginary coordinate in the image.
  for (y = 0; y < height; y++) {
    v = ic + ((long double) y + 0.5L - (long double) height / 2.0L) * stepv;
    for (x = 0; x < width; x++) {
      u = rc + ((long double) x + 0.5L - (long double) width / 2.0L) * stepu;

      // Iterate until either maxit is reached or |z| > 2.
      iterations = 0;
      r1 = 0.0L;
      i1 = 0.0L;
      while ((((r1 * r1) + (i1 * i1)) <= 4.0L) && (iterations < maxit)) {
        r2 = (r1 * r1) - (i1 * i1) + u;
        i2 = (2.0L * i1 * r1) + v;
        iterations++;
        r1 = r2;
        i1 = i2;
      }

      // If the escape criterion is reached, the point is outside the set;
      // use its integer iteration count to select a palette color.
      if (((r1 * r1) + (i1 * i1)) > 4.0L) {
        index = iterations % ncolor;
        fputc (palette[index].b, fo);
        fputc (palette[index].g, fo);
        fputc (palette[index].r, fo);

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

  // Free allocated memory.
  free (palette);

  return (EXIT_SUCCESS);
}

// Obtain a text string from standard input.
int
inputtext (char *text) {

  // Request new text from standard input.
  fgets (text, MAX_STRINGLEN, stdin);

  // Remove trailing newline, if there.
  if ((strnlen (text, MAX_STRINGLEN) > 0) && (text[strnlen (text, MAX_STRINGLEN) - 1] == '\n')) {
    text[strnlen (text, MAX_STRINGLEN) - 1] = '\0';  // Replace newline with string termination.
  }

  return (EXIT_SUCCESS);
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
