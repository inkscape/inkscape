/* Copyright (C) 2001-2015 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */

#ifndef BITMAP_H
#define BITMAP_H

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stddef.h>

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

#define bm_scanline(bm, y) ((bm)->map + (ptrdiff_t)(y)*(ptrdiff_t)(bm)->dy)
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

/* return new un-initialized bitmap. NULL with errno on error.
   Assumes w, h >= 0. */
static inline potrace_bitmap_t *bm_new(int w, int h) {
  potrace_bitmap_t *bm;
  int dy = w == 0 ? 0 : (w - 1) / BM_WORDBITS + 1;
  ptrdiff_t size = (ptrdiff_t)dy * (ptrdiff_t)h * (ptrdiff_t)BM_WORDSIZE;

  /* check for overflow error */
  if (size < 0 || (h != 0 && dy != 0 && size / h / dy != BM_WORDSIZE)) {
    errno = ENOMEM;
    return NULL;
  }

  bm = (potrace_bitmap_t *) malloc(sizeof(potrace_bitmap_t));
  if (!bm) {
    return NULL;
  }
  bm->w = w;
  bm->h = h;
  bm->dy = dy;
  bm->map = (potrace_word *) malloc(size);
  if (!bm->map) {
    free(bm);
    return NULL;
  }
  return bm;
}

/* clear the given bitmap. Set all bits to c. */
static inline void bm_clear(potrace_bitmap_t *bm, int c) {
  /* Note: if the bitmap was created with bm_new, then it is
     guaranteed that size will fit into the ptrdiff_t type. */
  ptrdiff_t size = (ptrdiff_t)bm->dy * (ptrdiff_t)bm->h * (ptrdiff_t)BM_WORDSIZE;
  memset(bm->map, c ? -1 : 0, size);
}

/* duplicate the given bitmap. Return NULL on error with errno set. */
static inline potrace_bitmap_t *bm_dup(const potrace_bitmap_t *bm) {
  potrace_bitmap_t *bm1 = bm_new(bm->w, bm->h);
  ptrdiff_t size = (ptrdiff_t)bm->dy * (ptrdiff_t)bm->h * (ptrdiff_t)BM_WORDSIZE;
  if (!bm1) {
    return NULL;
  }
  memcpy(bm1->map, bm->map, size);
  return bm1;
}

/* invert the given bitmap. */
static inline void bm_invert(potrace_bitmap_t *bm) {
  ptrdiff_t i;
  ptrdiff_t size = (ptrdiff_t)bm->dy * (ptrdiff_t)bm->h;

  for (i = 0; i < size; i++) {
    bm->map[i] ^= BM_ALLBITS;
  }
}

#endif /* BITMAP_H */
