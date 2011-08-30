/* Copyright (C) 2001-2011 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */

#ifndef BITMAP_H
#define BITMAP_H

#include <string.h>
#include <stdlib.h>

/* The bitmap type is defined in potracelib.h */
#include "potracelib.h"

/* The present file defines some convenient macros and static inline
   functions for accessing bitmaps. Since they only produce inline
   code, they can be conveniently shared by the library and frontends,
   if desired */

/* ---------------------------------------------------------------------- */
/* some measurements */

#define BM_WORDSIZE ((int)sizeof(potrace_word))
#define BM_WORDBITS (8*BM_WORDSIZE)
#define BM_HIBIT (((potrace_word)1)<<(BM_WORDBITS-1))
#define BM_ALLBITS (~(potrace_word)0)

/* macros for accessing pixel at index (x,y). U* macros omit the
   bounds check. */

#define bm_scanline(bm, y) ((bm)->map + (y)*(bm)->dy)
#define bm_index(bm, x, y) (&bm_scanline(bm, y)[(x)/BM_WORDBITS])
#define bm_mask(x) (BM_HIBIT >> ((x) & (BM_WORDBITS-1)))
#define bm_range(x, a) ((int)(x) >= 0 && (int)(x) < (a))
#define bm_safe(bm, x, y) (bm_range(x, (bm)->w) && bm_range(y, (bm)->h))
#define BM_UGET(bm, x, y) ((*bm_index(bm, x, y) & bm_mask(x)) != 0)
#define BM_USET(bm, x, y) (*bm_index(bm, x, y) |= bm_mask(x))
#define BM_UCLR(bm, x, y) (*bm_index(bm, x, y) &= ~bm_mask(x))
#define BM_UINV(bm, x, y) (*bm_index(bm, x, y) ^= bm_mask(x))
#define BM_UPUT(bm, x, y, b) ((b) ? BM_USET(bm, x, y) : BM_UCLR(bm, x, y))
#define BM_GET(bm, x, y) (bm_safe(bm, x, y) ? BM_UGET(bm, x, y) : 0)
#define BM_SET(bm, x, y) (bm_safe(bm, x, y) ? BM_USET(bm, x, y) : 0)
#define BM_CLR(bm, x, y) (bm_safe(bm, x, y) ? BM_UCLR(bm, x, y) : 0)
#define BM_INV(bm, x, y) (bm_safe(bm, x, y) ? BM_UINV(bm, x, y) : 0)
#define BM_PUT(bm, x, y, b) (bm_safe(bm, x, y) ? BM_UPUT(bm, x, y, b) : 0)

/* free the given bitmap. Leaves errno untouched. */
static inline void bm_free(potrace_bitmap_t *bm) {
  if (bm) {
    free(bm->map);
  }
  free(bm);
}

/* return new un-initialized bitmap. NULL with errno on error */
static inline potrace_bitmap_t *bm_new(int w, int h) {
  potrace_bitmap_t *bm;
  int dy = (w + BM_WORDBITS - 1) / BM_WORDBITS;

  bm = (potrace_bitmap_t *) malloc(sizeof(potrace_bitmap_t));
  if (!bm) {
    return NULL;
  }
  bm->w = w;
  bm->h = h;
  bm->dy = dy;
  bm->map = (potrace_word *) malloc(dy * h * BM_WORDSIZE);
  if (!bm->map) {
    free(bm);
    return NULL;
  }
  return bm;
}

/* clear the given bitmap. Set all bits to c. */
static inline void bm_clear(potrace_bitmap_t *bm, int c) {
  memset(bm->map, c ? -1 : 0, bm->dy * bm->h * BM_WORDSIZE);
}

/* duplicate the given bitmap. Return NULL on error with errno set. */
static inline potrace_bitmap_t *bm_dup(const potrace_bitmap_t *bm) {
  potrace_bitmap_t *bm1 = bm_new(bm->w, bm->h);
  if (!bm1) {
    return NULL;
  }
  memcpy(bm1->map, bm->map, bm->dy * bm->h * BM_WORDSIZE);
  return bm1;
}

/* invert the given bitmap. */
static inline void bm_invert(potrace_bitmap_t *bm) {
  int i;
  for (i = 0; i < bm->dy * bm->h; i++) {
    bm->map[i] ^= BM_ALLBITS;
  }
}

#endif /* BITMAP_H */
