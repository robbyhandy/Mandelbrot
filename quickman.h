// -------------------------------------------------------------------------------------
// QuickMAN.h -- Header file for the QuickMAN SSE/SSE2-based Mandelbrot Set calculator
// Copyright (C) 2006-2008 Paul Gentieu (paul.gentieu@yahoo.com)
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
// Project Page: http://quickman.sourceforge.net
//
// Author: Paul Gentieu (main code, ASM iteration cores, palettes)
//
// -------------------------------------------------------------------------------------
//
// 11/12/08 PG: Initial for v1.10. Split off most definitions from the C files
// into this header file.

#define CFG_FILE "quickman.cfg"         // configuration file containing default settings

// Precision used in calculation
#define PRECISION_AUTO        0 // Automatically determined based on magnification
#define PRECISION_SINGLE      1 // 32-bit float
#define PRECISION_DOUBLE      2 // 64-bit double
#define PRECISION_EXTENDED    3 // 80-bit (x87) double or larger

// Available algorithms
#define ALG_FAST_ASM_AMD      0 // Use the "wave" algorithm to guess pixels
#define ALG_EXACT_ASM_AMD     1 // Calculate every pixel (no interpolation or guessing)
#define ALG_FAST_ASM_INTEL    2 // Intel versions
#define ALG_EXACT_ASM_INTEL   3 //
#define ALG_FAST_C            4 // Unoptimized C versions
#define ALG_EXACT_C           5 //
//#define ALG_ERROR           6 // Show error image: exact ^ fast

#define ALG_EXACT             1 // using Exact alg if this bit set (change with above)
#define ALG_INTEL             2 // using Intel alg if this bit set
#define ALG_C                 4 // using C alg if this bit set

// Rendering algorithms
#define RALG_STANDARD         0 // keep this 0
#define RALG_NORMALIZED       1

#define NUM_ELEM(a) (sizeof(a) / sizeof(a[0]))

#define MAX_THREADS_IND     5 // Set this to set maximum number of threads (== 2^this)
#define MAX_THREADS        (1 << MAX_THREADS_IND)

// Max threads that can be running at once, with save going on in the background
#define MAX_QUEUE_THREADS  (MAX_THREADS * 2 + 3)

//#define USE_PERFORMANCE_COUNTER   // See get_timer()

#ifdef USE_PERFORMANCE_COUNTER
typedef LARGE_INTEGER TIME_UNIT;
#else
typedef DWORD TIME_UNIT;
#endif

#define DEFAULT_PAL  2 // new loud palette

// Home image parameters
#define MAG_START       0.3
#define HOME_RE         -0.7
#define HOME_IM         0.001 // offset y a bit so x axis isn't black on the left side (looks a little better)
#define HOME_MAG        1.35
#define HOME_MAX_ITERS  256

// Navigation modes
#define MODE_ZOOM             1     // zoom in/out 2x or zoom on rectangle, using magnifier
#define MODE_RTZOOM           2     // realtime zoom in/out
#define MODE_PAN              3     // pan around the image

// Bits for do_rtzoom
#define RTZOOM_IN             1     // zoom in
#define RTZOOM_OUT            2     // zoom out
#define RTZOOM_WITH_BUTTON    4     // 1 if zooming with the zoom button, 0 if with the mouse

// Multiply or divide the magnification by this factor when the user
// zooms using a single click.
#define MAG_ZOOM_FACTOR       2.0
#define MAG_MIN               0.02  // minimum magnification

// Magnitude squared must exceed this for a point to be considered diverged.
// Values smaller than 16 usually give less aesthetically pleasing results. Maybe
// make this user-settable.

#define DIVERGED_THRESH       16.0 // 4.0
#define DIVERGED_THRESH_SQ    256  // DIVERGED_THRESH squared (integer)

// Integer portion of high word in double, corresponding to threshold above
#define DIV_EXP               0x40300000   // for doubles with 11 bit exponent
#define DIV_EXP_FLOAT         0x41800000   // for floats with 8 bit exponent
//#define DIV_EXP             0x40100000   // for 4.0

#define MIN_ITERS             2            // allow to go down to min possible, for overhead testing
#define MAX_ITERS             0x08000000   // keep upper 4 bits free in iter array, in case we need them for something

#define MIN_SIZE              4            // min image size dimension. Code should work down to 1 x 1

//#define WM_DUMMY   (WM_APP + 100)        // use this form if messages needed

// Status bits
#define STAT_NEED_RECALC         1  // 1 if image must be recalculated next time, for whatever reason
#define STAT_RECALC_FOR_PALETTE  2  // 1 if image must be recalculated before a new palette can be applied (true after panning)
#define STAT_FULLSCREEN          4  // 1 if in fullscreen mode
#define STAT_RECALC_IMMEDIATELY  8  // 1 if the image should be recalculated immediately (e.g., after window was resized)
#define STAT_DIALOG_HIDDEN       16 // 1 if the control dialog is currently hidden
#define STAT_PALETTE_LOCKED      32 // 1 if the palette is currently locked (ignore logfile palettes)
#define STAT_HELP_SHOWING        64 // 1 if the help window is showing
#define STAT_DOING_SAVE         128 // 1 if doing a save

// Quadrant-based panning structures

// Structure for rectangles used in panning
typedef struct
{
   int x[2];   // x coordinates; x[0] <= x[1]
   int y[2];   // y coordinates; y[0] <= y[1]
   int valid;  // nonzero if rect is valid, 0 if invalid (should be ignored)
}
rectangle;

// Structure for quadrants used in panning
typedef struct
{
   int status;          // see below
   HBITMAP handle;      // handle to the bitmap

   rectangle quad_rect; // rectangle coordinates of this quadrant

   // Raw bitmap data. Each 32-bit value is an RGB triplet (in bits 23-0) and one 0 byte
   // (in bits 31-24). Faster to access than a 24-bit bitmap.
   unsigned *bitmap_data;

   // Blitting parameters. All offsets are quadrant-relative (i.e., range from 0 to
   // xsize - 1 and 0 to ysize - 1 inclusive).

   int src_xoffs;       // Source offsets: blitting starts from this point in the bitmap
   int src_yoffs;
   int dest_xoffs;      // Dest offsets: blitting goes to this point on the screen
   int dest_yoffs;
   int blit_xsize;      // Size of area to blit
   int blit_ysize;
}
quadrant;

// Values for the quadrant status word
#define QSTAT_DO_BLIT      1  // if set, blit this quadrant's data to the screen

// Structures and variables used in iteration

// Structure to hold the state of 4 iterating points (or 8 for SSE). For SSE (single precision),
// 32-bit floats are packed into the fields, otherwise 64-bit doubles. This needs
// to be 16-byte aligned. Project/compiler options can't guarantee this alignment:
// must use syntax below.
//
// Only the first 4/8 values of each array are used (unless otherwise noted). But the
// additional values are still necessary to force each array to occupy its own 64-byte
// cache line (i.e, no sharing). With line sharing there can be conflicts that cost cycles.
//
// May even want to give each 128 bits (xmm reg) its own cache line- change x to x01, x23, etc.
// Initialization and divergence detection would be nastier
//
// Tried expanding x, y, and yy to 16 doubles so ..23 regs could have own cache line: no effect

typedef struct // sps
{
   double x[8];                  // 0   x, y, yy = the state of the iterating points
   double y[8];                  // 64
   double yy[8];                 // 128
   double a[8];                  // 192 Real coordinate of point
   double b[8];                  // 256 Imag coordinate of point
   double mag[8];                // 320 Magnitudes from current iteration
   double magprev[8];            // 384 Magnitudes from previous iteration
   double two_d[8];              // 448 Only 1st 2 used; must be set to 2.0. Used in SSE2 routine.
   float two_f[16];              // 512 Only 1st 4 used; must be set to 2.0. Used in SSE routine.
   double rad_d[8];              // 576 Radius^2 for divergence detection; only 1st 2 used; only used in Intel version
   float rad_f[16];              // 640 only 1st 4 used

   // Even though the following fields aren't used in the inner loop, there's a slight decrease in
   // performance if they aren't aligned to a 64-byte cache line.

   unsigned iters[16];           // 704 Current iteration counts; only 1st 4/8 used
   unsigned *iters_ptr[16];      // 768 Pointer into iteration count array; only 1st 4/8 used
   float *mag_ptr[16];           // 832 Pointer into iteration count array; only 1st 4/8 used
   unsigned long long iterctr;   // 896 Iterations counter, for benchmarking. M$ 64-bit aligns this, adding (crash-causing) extra padding if not already on a 64-bit boundary...
   double ab_in[2];              // 904 loop sets ab_in to the point to iterate on (ab_in[0] = re, ab_in[1] = im). Others unused. MS also 64-bit aligns this
   unsigned cur_max_iters;       // 920 Max iters to do this loop
   unsigned queue_status;        // Status of pointstruct queue (free/full slots)
   unsigned pad[8];              // Pad to make size a multiple of 64. Necessary, otherwise code will crash with 2 or more threads due to array misalignment.
}
man_pointstruct;

// Pointers to structure members for asm functions.
// There should be some function to calculate these automatically, but use constants for now.
// Changing structure order can slow things down anyway so it shouldn't be done without good reason.

// Aliases for 4 double-precision points. EBX points to the beginning of the structure.
#define PS4_X01            [ebx + 0]
#define PS4_X23            [ebx + 0 + 16]
#define PS4_Y01            [ebx + 64]
#define PS4_Y23            [ebx + 64 + 16]
#define PS4_YY01           [ebx + 128]
#define PS4_YY23           [ebx + 128 + 16]
#define PS4_A01            [ebx + 192]
#define PS4_A23            [ebx + 192 + 16]
#define PS4_B01            [ebx + 256]
#define PS4_B23            [ebx + 256 + 16]
#define PS4_MAG01          [ebx + 320]          // Magnitudes of points 0 and 1
#define PS4_MEXP0          [ebx + 320 + 4]      // Locations of exponent bits in magnitudes
#define PS4_MEXP1          [ebx + 320 + 12]
#define PS4_MAG23          [ebx + 320 + 16]     // Magnitudes of points 2 and 3
#define PS4_MEXP2          [ebx + 320 + 20]
#define PS4_MEXP3          [ebx + 320 + 28]
#define PS4_MAGPREV01      [ebx + 384]          // Magnitudes of points 0 and 1 after the previous iteration
#define PS4_MAGPREV23      [ebx + 384 + 16]     // ditto for points 2 and 3
#define PS4_TWO            [ebx + 448]
#define PS4_RAD            [ebx + 576]
#define PS4_ITERS0         [ebx + 704]
#define PS4_ITERS1         [ebx + 704 + 4]
#define PS4_ITERS2         [ebx + 704 + 8]
#define PS4_ITERS3         [ebx + 704 + 12]
#define PS4_ITERCTR_L      [ebx + 896]
#define PS4_ITERCTR_H      [ebx + 900]
#define PS4_CUR_MAX_ITERS  [ebx + 920]

// Aliases for 8 single precision points
#define PS8_X03            PS4_X01
#define PS8_X47            PS4_X23
#define PS8_Y03            PS4_Y01
#define PS8_Y47            PS4_Y23
#define PS8_YY03           PS4_YY01
#define PS8_YY47           PS4_YY23
#define PS8_A03            PS4_A01
#define PS8_A47            PS4_A23
#define PS8_B03            PS4_B01
#define PS8_B47            PS4_B23
#define PS8_MAG03          [ebx + 320]
#define PS8_MEXP0          [ebx + 320]
#define PS8_MEXP1          [ebx + 320 + 4]
#define PS8_MEXP2          [ebx + 320 + 8]
#define PS8_MEXP3          [ebx + 320 + 12]
#define PS8_MAG47          [ebx + 320 + 16]
#define PS8_MEXP4          [ebx + 320 + 16]
#define PS8_MEXP5          [ebx + 320 + 20]
#define PS8_MEXP6          [ebx + 320 + 24]
#define PS8_MEXP7          [ebx + 320 + 28]
#define PS8_MAGPREV03      PS4_MAGPREV01
#define PS8_MAGPREV47      PS4_MAGPREV23
#define PS8_TWO            [ebx + 512]
#define PS8_RAD            [ebx + 640]
#define PS8_ITERS0         [ebx + 704]       // Iteration counters for 8 points
#define PS8_ITERS1         [ebx + 704 + 4]
#define PS8_ITERS2         [ebx + 704 + 8]
#define PS8_ITERS3         [ebx + 704 + 12]
#define PS8_ITERS4         [ebx + 704 + 16]
#define PS8_ITERS5         [ebx + 704 + 20]
#define PS8_ITERS6         [ebx + 704 + 24]
#define PS8_ITERS7         [ebx + 704 + 28]
#define PS8_ITERCTR_L      PS4_ITERCTR_L
#define PS8_ITERCTR_H      PS4_ITERCTR_H
#define PS8_CUR_MAX_ITERS  PS4_CUR_MAX_ITERS

// Generic setting structure. Some flags are encoded by upper or lowercase
// letters in the name (see below). The name is not case sensitive in files.
typedef struct // sset
{
   char *name;       // Name of the setting
   int val;          // Value of the setting
   int default_val;  // Default value for the setting
   int min;          // Its min and max limits, to prevent bad data
   int max;          // in files from messing things up
}
setting;

#define LOWERCASE 0x20

// Autoreset flag is set by having the first letter of the name uppercase.
// If the autoreset flag is set, the setting resets to default_val before each new image.
#define SETTING_AUTORESET(s) (!((s)->name[0] & LOWERCASE))

// Global configuration and algorithm settings. All fields must be setting structs.
typedef struct
{
   setting pan_rate;        // value for the GUI pan rate slider
   setting pan_key;         // if nonzero, this image should be auto-panned (contains key code)
   setting zoom_rate;       // value for the GUI zoom rate slider
   setting zoom_in_out;     // 0 = no zoom, 1 = zoom in, 2 = zoom out (to be implemented)
   setting xsize;           // X image size. 0 = maximize window (to be implemented)
   setting ysize;           // Y image size  0 = restore window (to be implemented)
   setting max_iters_color; // the color for points with max iters- for making lakes and such. Cool...
   setting pal_xor;         // value to XOR with the palette to create the image palette (0xFFFFFF for invert, etc)
   setting options;         // options bitfield

   // Stripes per thread (bitfield; 4 bits per num_threads index). For example, bits 7-4 give
   // the number of stripes per thread for two threads, 11-8 are for four threads, etc.

   setting stripes_per_thread;
   setting blit_stripe_thickness;   // thickness of stripes used in striped_blit
   setting pfcmin;                  // pan filter constant min and max
   setting pfcmax;                  // 10000 times the real value for these
}
settings;

// Default for stripes per thread bitfield.
// 32 threads: 2; 16 threads: 3; 8 threads: 4; 4 threads: 4; 2 threads: 7; 1 thread: 1
#define SPT_DEFAULT  0x234471

// Bits in options bitfield
#define OPT_RECALC_ON_RESIZE        1     // 1 to recalculate immediately whenever the window is resized, 0 = not
#define OPT_DIALOG_IN_FULLSCREEN    2     // 1 if the control dialog should initially be visible
                                          // after entering fullscreen mode, 0 if not
                                          // can always toggle on/off with C key
#define OPT_NORMALIZED              4     // if 1, start with the normalized rendering algorithm (otherwise standard)
#define OPT_EXACT_ALG               8     // if 1, start with an exact algorithm (default fast)

// Default for options bitfield
#define OPTIONS_DEFAULT (OPT_RECALC_ON_RESIZE)

// A log entry structure. It can have its own set of settings. The settings fields are all
// initialized to -1, then set to any values in the logfile that are found for this entry.
// Any setting that's >= 0 will be copied to the global config settings before the image
// is displayed.

typedef struct
{
   double re;
   double im;
   double mag;
   unsigned max_iters;
   unsigned palette;

   // Kinda wasteful to have the entire settings struct here when all we need are the
   // val fields, but it's easier this way
   settings log_settings;
}
log_entry;

// A stripe for the thread function to calculate. See man_calculate().
typedef struct
{
   int xstart;
   int xend;
   int ystart;
   int yend;
}
stripe;

// Maximum number of stripes per image to give each thread. See man_calculate().
#define MAX_STRIPES        8

// Thread state structure
typedef struct
{
   int thread_num;               // thread number
   man_pointstruct *ps_ptr;      // pointer to this thread's iterating point structure, from array in man_calc_struct
   stripe stripes[MAX_STRIPES];  // list of stripes for this thread to calculate
   int num_stripes;
   HANDLE done_event;            // event set by thread when calculation is finished
   void *calc_struct;            // pointer to parent man_calc_struct

   // Nonessential variables (for profiling, load balance testing, etc)
   unsigned long long total_iters;  // iters value that keeps accumulating until reset (before next zoom, etc)
   unsigned points_guessed;         // points guessed in fast algorithm
}
thread_state;

// Use this for overhead-sensitive functions (but nothing else, as it can
// hog registers). Seems to have a small effect.
#define FASTCALL __fastcall

// Define this to dump any loaded BMP palette to palarray.c as a compilable array
// #define DUMP_USER_PAL

typedef struct
{
   // Arrays of dwords: msb is 0, lower 3 bytes are RGB triplets.
   // Can have variable-size palettes, given by size field.
   unsigned *rgb;
   int size;
} palette;

// Structure defining the work for each palette mapping thread to do

// Some of these values don't need to be in the structure (can be globals) but it
// really doesn't make much difference.

typedef struct
{
   void *calc_struct;
   unsigned *dest;
   unsigned *src;
   unsigned xsize;
   unsigned ysize;
   unsigned *pal;
   unsigned pal_size;
   unsigned max_iters_color;
   int thread_num;
}
pal_work;

// Maximum max_iters for which palette lookup array is used
#define PAL_LOOKUP_MAX        32768

// Structure for holding all the parameters, sub-structures, and memory pointers used
// for a mandelbrot calculation. Separate structures are used for the main calculation and
// for saving images, so both can run in parallel.

typedef struct // smcs
{
   // This MUST be the first structure member, due to alignment requirements
   man_pointstruct pointstruct_array[MAX_THREADS];

   // These function pointers get set based on the algorithm in use

   // Point queueing function (C/SSE/SSE2/x87)
   void (FASTCALL *queue_point)(void *calc_struct, man_pointstruct *ps_ptr, unsigned *iters_ptr);

   // Iteration function (C/SSE/SSE2/x87, AMD/Intel). Returns the number of iterations done per point.
   unsigned (*mandel_iterate)(man_pointstruct *ps_ptr);

   // State structures and events for each thread used in the calculation
   thread_state thread_states[MAX_THREADS];
   HANDLE thread_done_events[MAX_THREADS];

   // Image size and offset parameters
   int xsize;
   int ysize;
   int min_dimension;
   int image_size;

   // To reduce the effects of precision loss at high magnifications, panning is now tracked as an
   // offset from the re/im point. The former method was to update re/im on every pan.

   long long pan_xoffs;
   long long pan_yoffs;

   // Calculation parameters
   double re;           // real part of image center coordinate
   double im;           // imaginary part of image center coordinate
   double mag;          // magnification
   unsigned max_iters;  // maximum iterations to do per point
   unsigned max_iters_last; // last max_iters used in a calculation

   // GUI parameters, latched before the calculation starts
   int alg;             // algorithm
   int cur_alg;         // current algorithm (can switch during panning)
   int precision;       // user-desired precision

   // Dynamically allocated arrays
   double *img_re;      // arrays for holding the RE, IM coordinates
   double *img_im;      // of each pixel in the image

   unsigned *iter_data_start; // for dummy line creation: see alloc_man_mem
   unsigned *iter_data;       // iteration counts for each pixel in the image. Converted to a bitmap by applying the palette.
   int iter_data_line_size;   // size of one line of iteration data (needs 2 dummy pixels at the end of each line)

   float *mag_data;     // magnitude (squared) for each point
   int mag_data_offs;   // byte offset of mag_data from iter_data. Could be negative; must be int

   unsigned char *png_buffer; // buffer for data to write to PNG file

   // Palette and rendering related items
   unsigned palette;                       // current palette to use
   unsigned prev_pal;                      // previous palette used
   unsigned pal_xor;                       // for inversion (use 0xFFFFFF)
   unsigned max_iters_color;               // color of max iters points, from logfile/cfgfile
   int rendering_alg;                      // rendering algorithm: standard or normalized iteration count

   pal_work pal_work_array[MAX_THREADS];   // work for palette mapping threads
   HANDLE pal_events[MAX_THREADS];
   unsigned pal_lookup[PAL_LOOKUP_MAX + 1]; // palette lookup table; need one extra entry for max_iters

   // Misc items
   unsigned flags;   // See below
}
man_calc_struct;

// Values for man_calc_struct flags field
#define FLAG_IS_SAVE          1 // 1 if this is a saving structure, 0 for normal calculation
#define FLAG_CALC_RE_ARRAY    2 // set to 0 on first row when saving, otherwise 1- reduces overhead

// Get the magnitude (squared) corresponding to the iteration count at iter_ptr. Points
// to an entry in the mag_data array of a man_calc_struct.
#define MAG(m, iter_ptr) *((float *) ((char *) iter_ptr + m->mag_data_offs))

// Prototypes
void do_man_calculate(int recalc_all);

// From palettes.c and imagesave.c
int png_save_start(char *file, int width, int height);
int png_save_write_row(unsigned char *row);
int png_save_end(void);
int init_palettes(double diverged_thresh);
int load_palette(FILE *fp);
int load_palette_from_bmp(FILE *fp);
int get_palette_rgb_val(int ind, char *str, int length, unsigned *rgb);
void apply_palette(man_calc_struct *m, unsigned *dest, unsigned *src, unsigned xsize, unsigned ysize);
