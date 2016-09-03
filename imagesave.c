// -------------------------------------------------------------------------------------
// imagesave.c -- Code for saving QuickMAN images.
// Copyright (C) 2008 Paul Gentieu (paul.gentieu@yahoo.com)
//
// This file is part of QuickMAN.
//
// QuickMAN is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
//
// -------------------------------------------------------------------------------------
//
// 11/09/08 PG: New file for v1.10.

#define STRICT
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x501    // Windows XP

#include <windows.h>
#include <stdio.h>
#include <setjmp.h>

// #defines and prototypes for accessing the bare-bones write-only PNGLIB.
// Derived from png.h

#define PNG_LIBPNG_VER_STRING "1.2.33" // Copy this from png.h

#define PNG_COLOR_TYPE_RGB            2
#define PNG_INTERLACE_NONE            0
#define PNG_COMPRESSION_TYPE_DEFAULT  0
#define PNG_FILTER_TYPE_DEFAULT       0

// Library won't care if we use void for all pointer types

extern void *png_create_write_struct(void *ver, void *err_ptr,
            void *err_fn, void *warn_fn);
extern void *png_create_info_struct(void *png_ptr);
extern void png_destroy_write_struct(void **png_ptr_ptr, void **png_info_ptr_ptr);
extern void png_init_io(void *png_ptr, FILE *fp);
extern void png_set_compression_level(void *png_ptr, int level);
extern void png_set_IHDR(void *png_ptr, void *info_ptr, unsigned width, unsigned height,
            int bit_depth, int color_type, int interlace_method,
            int compression_method, int filter_method);
extern void png_write_info(void *png_ptr, void *info_ptr);
extern void png_write_row(void *png_ptr, unsigned char *row);
extern void png_write_end(void *png_ptr, void *info_ptr);
extern void png_set_bgr(void *png_ptr);

#define png_jmpbuf(png_ptr)(png_ptr) // assumes jmp_buf is 1st png_struct member...

// -------------- PNG functions -----------------

static void *png;
static void *pnginfo;
static FILE *png_fp;

int png_error(void)
{
   MessageBox(NULL, "The PNG library returned an error. Possible causes:\n\n"
                    "1. The image you are trying to save may be too large.\n"
                    "2. The filename for the image is invalid.",
                    NULL, MB_OK | MB_ICONSTOP | MB_TASKMODAL);
   return 0;
}

// Start the PNG save for an image of dimensions WIDTH x HEIGHT.

int png_save_start(char *file, int width, int height)
{
   if ((png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) == NULL)
      return png_error();

   if ((pnginfo = png_create_info_struct(png)) == NULL || fopen_s(&png_fp, file, "wb"))
   {
      png_destroy_write_struct(&png, NULL);
      return png_error();
   }

   if (setjmp(png_jmpbuf(png)))
   {
      // Any pnglib errors jump here
      png_destroy_write_struct(&png, &pnginfo);
      fclose(png_fp);
      return png_error();
   }

   png_init_io(png, png_fp);
   png_set_bgr(png);  // Set B,G,R color order: necessary for correct palette mapping

   // Set Zlib compression levels; 0 = none; 9 = best. 3-6 are supposed to be almost as
   // good as 9 for images. Test: 6- 43s 17519kb; 9- 45s 18573kb; 9 sometimes gives worse compression
   png_set_compression_level(png, 6);

   png_set_IHDR(png, pnginfo, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

   png_write_info(png, pnginfo);
   return 1;
}

// Write one row of the image. Row must have 3 * width bytes (RGB), where width
// was the parameter passed to png_save_start.
int png_save_write_row(unsigned char *row)
{
   if (setjmp(png_jmpbuf(png)))
   {
      // any pnglib errors jump here
      png_destroy_write_struct(&png, &pnginfo);
      fclose(png_fp);
      return png_error();
   }
   png_write_row(png, row);
   return 1;
}

// Finish the PNG save. Call after all rows are written.
int png_save_end(void)
{
   if (setjmp(png_jmpbuf(png)))
   {
      // any pnglib errors jump here
      png_destroy_write_struct(&png, &pnginfo);
      fclose(png_fp);
      return png_error();
   }

   png_write_end(png, NULL);
   png_destroy_write_struct(&png, &pnginfo);
   fclose(png_fp);
   return 1;
}

// Later add functions for saving raw RGB data, magnitudes, and iteration counts
