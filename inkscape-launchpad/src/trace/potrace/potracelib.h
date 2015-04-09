/* Copyright (C) 2001-2015 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */

#ifndef POTRACELIB_H
#define POTRACELIB_H

/* this file defines the API for the core Potrace library. For a more
   detailed description of the API, see potracelib.pdf */

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------- */
/* tracing parameters */

/* turn policies */
#define POTRACE_TURNPOLICY_BLACK 0
#define POTRACE_TURNPOLICY_WHITE 1
#define POTRACE_TURNPOLICY_LEFT 2
#define POTRACE_TURNPOLICY_RIGHT 3
#define POTRACE_TURNPOLICY_MINORITY 4
#define POTRACE_TURNPOLICY_MAJORITY 5
#define POTRACE_TURNPOLICY_RANDOM 6

/* structure to hold progress bar callback data */
struct potrace_progress_s {
  void (*callback)(double progress, void *privdata); /* callback fn */
  void *data;          /* callback function's private data */
  double min, max;     /* desired range of progress, e.g. 0.0 to 1.0 */
  double epsilon;      /* granularity: can skip smaller increments */
};
typedef struct potrace_progress_s potrace_progress_t;

/* structure to hold tracing parameters */
struct potrace_param_s {
  int turdsize;        /* area of largest path to be ignored */
  int turnpolicy;      /* resolves ambiguous turns in path decomposition */
  double alphamax;     /* corner threshold */
  int opticurve;       /* use curve optimization? */
  double opttolerance; /* curve optimization tolerance */
  potrace_progress_t progress; /* progress callback function */
};
typedef struct potrace_param_s potrace_param_t;

/* ---------------------------------------------------------------------- */
/* bitmaps */

/* native word size */
typedef unsigned long potrace_word;

/* Internal bitmap format. The n-th scanline starts at scanline(n) =
   (map + n*dy). Raster data is stored as a sequence of potrace_words
   (NOT bytes). The leftmost bit of scanline n is the most significant
   bit of scanline(n)[0]. */
struct potrace_bitmap_s {
  int w, h;              /* width and height, in pixels */
  int dy;                /* words per scanline (not bytes) */
  potrace_word *map;     /* raw data, dy*h words */
};
typedef struct potrace_bitmap_s potrace_bitmap_t;

/* ---------------------------------------------------------------------- */
/* curves */

/* point */
struct potrace_dpoint_s {
  double x, y;
};
typedef struct potrace_dpoint_s potrace_dpoint_t;

/* segment tags */
#define POTRACE_CURVETO 1
#define POTRACE_CORNER 2

/* closed curve segment */
struct potrace_curve_s {
  int n;                    /* number of segments */
  int *tag;                 /* tag[n]: POTRACE_CURVETO or POTRACE_CORNER */
  potrace_dpoint_t (*c)[3]; /* c[n][3]: control points. 
			       c[n][0] is unused for tag[n]=POTRACE_CORNER */
};
typedef struct potrace_curve_s potrace_curve_t;

/* Linked list of signed curve segments. Also carries a tree structure. */
struct potrace_path_s {
  int area;                         /* area of the bitmap path */
  int sign;                         /* '+' or '-', depending on orientation */
  potrace_curve_t curve;            /* this path's vector data */

  struct potrace_path_s *next;      /* linked list structure */

  struct potrace_path_s *childlist; /* tree structure */
  struct potrace_path_s *sibling;   /* tree structure */

  struct potrace_privpath_s *priv;  /* private state */
};
typedef struct potrace_path_s potrace_path_t;  

/* ---------------------------------------------------------------------- */
/* Potrace state */

#define POTRACE_STATUS_OK         0
#define POTRACE_STATUS_INCOMPLETE 1

struct potrace_state_s {
  int status;                       
  potrace_path_t *plist;            /* vector data */

  struct potrace_privstate_s *priv; /* private state */
};
typedef struct potrace_state_s potrace_state_t;

/* ---------------------------------------------------------------------- */
/* API functions */

/* get default parameters */
potrace_param_t *potrace_param_default(void);

/* free parameter set */
void potrace_param_free(potrace_param_t *p);

/* trace a bitmap*/
potrace_state_t *potrace_trace(const potrace_param_t *param, 
			       const potrace_bitmap_t *bm);

/* free a Potrace state */
void potrace_state_free(potrace_state_t *st);

/* return a static plain text version string identifying this version
   of potracelib */
char *potrace_version(void);

#ifdef  __cplusplus
} /* end of extern "C" */
#endif

#endif /* POTRACELIB_H */
