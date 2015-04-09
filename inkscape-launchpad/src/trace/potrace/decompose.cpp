/* Copyright (C) 2001-2015 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "potracelib.h"
#include "curve.h"
#include "lists.h"
#include "bitmap.h"
#include "decompose.h"
#include "progress.h"

/* ---------------------------------------------------------------------- */
/* deterministically and efficiently hash (x,y) into a pseudo-random bit */

static inline int detrand(int x, int y) {
  unsigned int z;
  static const unsigned char t[256] = { 
    /* non-linear sequence: constant term of inverse in GF(8), 
       mod x^8+x^4+x^3+x+1 */
    0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 
    0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 
    0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 
    1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 
    0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 
    0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 
    0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 
    0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 
    1, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 
    0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 
    1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 
  };

  /* 0x04b3e375 and 0x05a8ef93 are chosen to contain every possible
     5-bit sequence */
  z = ((0x04b3e375 * x) ^ y) * 0x05a8ef93;
  z = t[z & 0xff] ^ t[(z>>8) & 0xff] ^ t[(z>>16) & 0xff] ^ t[(z>>24) & 0xff];
  return z;
}

/* ---------------------------------------------------------------------- */
/* auxiliary bitmap manipulations */

/* set the excess padding to 0 */
static void bm_clearexcess(potrace_bitmap_t *bm) {
  if (bm->w % BM_WORDBITS != 0) {
    potrace_word mask = BM_ALLBITS << (BM_WORDBITS - (bm->w % BM_WORDBITS));
    for (int y=0; y<bm->h; y++) {
      *bm_index(bm, bm->w, y) &= mask;
    }
  }
}

struct bbox_s {
  int x0, x1, y0, y1;    /* bounding box */
};
typedef struct bbox_s bbox_t;

/* clear the bm, assuming the bounding box is set correctly (faster
   than clearing the whole bitmap) */
static void clear_bm_with_bbox(potrace_bitmap_t *bm, bbox_t *bbox) {
  int imin = (bbox->x0 / BM_WORDBITS);
  int imax = ((bbox->x1 + BM_WORDBITS-1) / BM_WORDBITS);
  int i, y;

  for (y=bbox->y0; y<bbox->y1; y++) {
    for (i=imin; i<imax; i++) {
      bm_scanline(bm, y)[i] = 0;
    }
  }
}

/* ---------------------------------------------------------------------- */
/* auxiliary functions */

/* return the "majority" value of bitmap bm at intersection (x,y). We
   assume that the bitmap is balanced at "radius" 1.  */
static int majority(potrace_bitmap_t *bm, int x, int y) {
  int i, a, ct;

  for (i=2; i<5; i++) { /* check at "radius" i */
    ct = 0;
    for (a=-i+1; a<=i-1; a++) {
      ct += BM_GET(bm, x+a, y+i-1) ? 1 : -1;
      ct += BM_GET(bm, x+i-1, y+a-1) ? 1 : -1;
      ct += BM_GET(bm, x+a-1, y-i) ? 1 : -1;
      ct += BM_GET(bm, x-i, y+a) ? 1 : -1;
    }
    if (ct>0) {
      return 1;
    } else if (ct<0) {
      return 0;
    }
  }
  return 0;
}

/* ---------------------------------------------------------------------- */
/* decompose image into paths */

/* efficiently invert bits [x,infty) and [xa,infty) in line y. Here xa
   must be a multiple of BM_WORDBITS. */
static void xor_to_ref(potrace_bitmap_t *bm, int x, int y, int xa) {
  int xhi = x & -BM_WORDBITS;
  int xlo = x & (BM_WORDBITS-1);  /* = x % BM_WORDBITS */
  int i;
  
  if (xhi<xa) {
    for (i = xhi; i < xa; i+=BM_WORDBITS) {
      *bm_index(bm, i, y) ^= BM_ALLBITS;
    }
  } else {
    for (i = xa; i < xhi; i+=BM_WORDBITS) {
      *bm_index(bm, i, y) ^= BM_ALLBITS;
    }
  }
  /* note: the following "if" is needed because x86 treats a<<b as
     a<<(b&31). I spent hours looking for this bug. */
  if (xlo) {
    *bm_index(bm, xhi, y) ^= (BM_ALLBITS << (BM_WORDBITS - xlo));
  }
}

/* a path is represented as an array of points, which are thought to
   lie on the corners of pixels (not on their centers). The path point
   (x,y) is the lower left corner of the pixel (x,y). Paths are
   represented by the len/pt components of a path_t object (which
   also stores other information about the path) */

/* xor the given pixmap with the interior of the given path. Note: the
   path must be within the dimensions of the pixmap. */
static void xor_path(potrace_bitmap_t *bm, path_t *p) {
  int xa, x, y, k, y1;

  if (p->priv->len <= 0) {  /* a path of length 0 is silly, but legal */
    return;
  }

  y1 = p->priv->pt[p->priv->len-1].y;

  xa = p->priv->pt[0].x & -BM_WORDBITS;
  for (k=0; k<p->priv->len; k++) {
    x = p->priv->pt[k].x;
    y = p->priv->pt[k].y;

    if (y != y1) {
      /* efficiently invert the rectangle [x,xa] x [y,y1] */
      xor_to_ref(bm, x, min(y,y1), xa);
      y1 = y;
    }
  }
}

/* Find the bounding box of a given path. Path is assumed to be of
   non-zero length. */
static void setbbox_path(bbox_t *bbox, path_t *p) {
  int x, y;
  int k;

  bbox->y0 = INT_MAX;
  bbox->y1 = 0;
  bbox->x0 = INT_MAX;
  bbox->x1 = 0;

  for (k=0; k<p->priv->len; k++) {
    x = p->priv->pt[k].x;
    y = p->priv->pt[k].y;

    if (x < bbox->x0) {
      bbox->x0 = x;
    }
    if (x > bbox->x1) {
      bbox->x1 = x;
    }
    if (y < bbox->y0) {
      bbox->y0 = y;
    }
    if (y > bbox->y1) {
      bbox->y1 = y;
    }
  }
}

/* compute a path in the given pixmap, separating black from white.
   Start path at the point (x0,x1), which must be an upper left corner
   of the path. Also compute the area enclosed by the path. Return a
   new path_t object, or NULL on error (note that a legitimate path
   cannot have length 0). Sign is required for correct interpretation
   of turnpolicies. */
static path_t *findpath(potrace_bitmap_t *bm, int x0, int y0, int sign, int turnpolicy) {
  int x, y, dirx, diry, len, size, area;
  int c, d, tmp;
  point_t *pt, *pt1;
  path_t *p = NULL;

  x = x0;
  y = y0;
  dirx = 0;
  diry = -1;

  len = size = 0;
  pt = NULL;
  area = 0;
  
  while (1) {
    /* add point to path */
    if (len>=size) {
      size += 100;
      size = (int)(1.3 * size);
      pt1 = (point_t *)realloc(pt, size * sizeof(point_t));
      if (!pt1) {
	goto error;
      }
      pt = pt1;
    }
    pt[len].x = x;
    pt[len].y = y;
    len++;
    
    /* move to next point */
    x += dirx;
    y += diry;
    area += x*diry;
    
    /* path complete? */
    if (x==x0 && y==y0) {
      break;
    }
    
    /* determine next direction */
    c = BM_GET(bm, x + (dirx+diry-1)/2, y + (diry-dirx-1)/2);
    d = BM_GET(bm, x + (dirx-diry-1)/2, y + (diry+dirx-1)/2);
    
    if (c && !d) {               /* ambiguous turn */
      if (turnpolicy == POTRACE_TURNPOLICY_RIGHT
	  || (turnpolicy == POTRACE_TURNPOLICY_BLACK && sign == '+')
	  || (turnpolicy == POTRACE_TURNPOLICY_WHITE && sign == '-')
	  || (turnpolicy == POTRACE_TURNPOLICY_RANDOM && detrand(x,y))
	  || (turnpolicy == POTRACE_TURNPOLICY_MAJORITY && majority(bm, x, y))
	  || (turnpolicy == POTRACE_TURNPOLICY_MINORITY && !majority(bm, x, y))) {
	tmp = dirx;              /* right turn */
	dirx = diry;
	diry = -tmp;
      } else {
	tmp = dirx;              /* left turn */
	dirx = -diry;
	diry = tmp;
      }
    } else if (c) {              /* right turn */
      tmp = dirx;
      dirx = diry;
      diry = -tmp;
    } else if (!d) {             /* left turn */
      tmp = dirx;
      dirx = -diry;
      diry = tmp;
    }
  } /* while this path */

  /* allocate new path object */
  p = path_new();
  if (!p) {
    goto error;
  }

  p->priv->pt = pt;
  p->priv->len = len;
  p->area = area;
  p->sign = sign;

  return p;
 
 error:
   free(pt);
   return NULL; 
}

/* Give a tree structure to the given path list, based on "insideness"
   testing. I.e., path A is considered "below" path B if it is inside
   path B. The input pathlist is assumed to be ordered so that "outer"
   paths occur before "inner" paths. The tree structure is stored in
   the "childlist" and "sibling" components of the path_t
   structure. The linked list structure is also changed so that
   negative path components are listed immediately after their
   positive parent.  Note: some backends may ignore the tree
   structure, others may use it e.g. to group path components. We
   assume that in the input, point 0 of each path is an "upper left"
   corner of the path, as returned by bm_to_pathlist. This makes it
   easy to find an "interior" point. The bm argument should be a
   bitmap of the correct size (large enough to hold all the paths),
   and will be used as scratch space. Return 0 on success or -1 on
   error with errno set. */

static void pathlist_to_tree(path_t *plist, potrace_bitmap_t *bm) {
  path_t *p, *p1;
  path_t *heap, *heap1;
  path_t *cur;
  path_t *head;
  path_t **plist_hook;          /* for fast appending to linked list */
  path_t **hook_in, **hook_out; /* for fast appending to linked list */
  bbox_t bbox;
  
  bm_clear(bm, 0);

  /* save original "next" pointers */
  list_forall(p, plist) {
    p->sibling = p->next;
    p->childlist = NULL;
  }
  
  heap = plist;

  /* the heap holds a list of lists of paths. Use "childlist" field
     for outer list, "next" field for inner list. Each of the sublists
     is to be turned into a tree. This code is messy, but it is
     actually fast. Each path is rendered exactly once. We use the
     heap to get a tail recursive algorithm: the heap holds a list of
     pathlists which still need to be transformed. */

  while (heap) {
    /* unlink first sublist */
    cur = heap;
    heap = heap->childlist;
    cur->childlist = NULL;
  
    /* unlink first path */
    head = cur;
    cur = cur->next;
    head->next = NULL;

    /* render path */
    xor_path(bm, head);
    setbbox_path(&bbox, head);

    /* now do insideness test for each element of cur; append it to
       head->childlist if it's inside head, else append it to
       head->next. */
    hook_in=&head->childlist;
    hook_out=&head->next;
    list_forall_unlink(p, cur) {
      if (p->priv->pt[0].y <= bbox.y0) {
	list_insert_beforehook(p, hook_out);
	/* append the remainder of the list to hook_out */
	*hook_out = cur;
	break;
      }
      if (BM_GET(bm, p->priv->pt[0].x, p->priv->pt[0].y-1)) {
	list_insert_beforehook(p, hook_in);
      } else {
	list_insert_beforehook(p, hook_out);
      }
    }

    /* clear bm */
    clear_bm_with_bbox(bm, &bbox);

    /* now schedule head->childlist and head->next for further
       processing */
    if (head->next) {
      head->next->childlist = heap;
      heap = head->next;
    }
    if (head->childlist) {
      head->childlist->childlist = heap;
      heap = head->childlist;
    }
  }
  
  /* copy sibling structure from "next" to "sibling" component */
  p = plist;
  while (p) {
    p1 = p->sibling;
    p->sibling = p->next;
    p = p1;
  }

  /* reconstruct a new linked list ("next") structure from tree
     ("childlist", "sibling") structure. This code is slightly messy,
     because we use a heap to make it tail recursive: the heap
     contains a list of childlists which still need to be
     processed. */
  heap = plist;
  if (heap) {
    heap->next = NULL;  /* heap is a linked list of childlists */
  }
  plist = NULL;
  plist_hook = &plist;
  while (heap) {
    heap1 = heap->next;
    for (p=heap; p; p=p->sibling) {
      /* p is a positive path */
      /* append to linked list */
      list_insert_beforehook(p, plist_hook);
      
      /* go through its children */
      for (p1=p->childlist; p1; p1=p1->sibling) {
	/* append to linked list */
	list_insert_beforehook(p1, plist_hook);
	/* append its childlist to heap, if non-empty */
	if (p1->childlist) {
	  list_append(path_t, heap1, p1->childlist);
	}
      }
    }
    heap = heap1;
  }

  return;
}

/* find the next set pixel in a row <= y. Pixels are searched first
   left-to-right, then top-down. In other words, (x,y)<(x',y') if y>y'
   or y=y' and x<x'. If found, return 0 and store pixel in
   (*xp,*yp). Else return 1. Note that this function assumes that
   excess bytes have been cleared with bm_clearexcess. */
static int findnext(potrace_bitmap_t *bm, int *xp, int *yp) {
  int x;
  int y;
  int x0;

  x0 = (*xp) & ~(BM_WORDBITS-1);

  for (y=*yp; y>=0; y--) {
    for (x=x0; x<bm->w; x+=BM_WORDBITS) {
      if (*bm_index(bm, x, y)) {
	while (!BM_GET(bm, x, y)) {
	  x++;
	}
	/* found */
	*xp = x;
	*yp = y;
	return 0;
      }
    }
    x0 = 0;
  }
  /* not found */
  return 1;
}

/* Decompose the given bitmap into paths. Returns a linked list of
   path_t objects with the fields len, pt, area, sign filled
   in. Returns 0 on success with plistp set, or -1 on error with errno
   set. */

int bm_to_pathlist(const potrace_bitmap_t *bm, path_t **plistp, const potrace_param_t *param, progress_t *progress) {
  int x;
  int y;
  path_t *p;
  path_t *plist = NULL;  /* linked list of path objects */
  path_t **plist_hook = &plist;  /* used to speed up appending to linked list */
  potrace_bitmap_t *bm1 = NULL;
  int sign;

  bm1 = bm_dup(bm);
  if (!bm1) {
    goto error;
  }

  /* be sure the byte padding on the right is set to 0, as the fast
     pixel search below relies on it */
  bm_clearexcess(bm1);

  /* iterate through components */
  x = 0;
  y = bm1->h - 1;
  while (findnext(bm1, &x, &y) == 0) { 
    /* calculate the sign by looking at the original */
    sign = BM_GET(bm, x, y) ? '+' : '-';

    /* calculate the path */
    p = findpath(bm1, x, y+1, sign, param->turnpolicy);
    if (p==NULL) {
      goto error;
    }

    /* update buffered image */
    xor_path(bm1, p);

    /* if it's a turd, eliminate it, else append it to the list */
    if (p->area <= param->turdsize) {
      path_free(p);
    } else {
      list_insert_beforehook(p, plist_hook);
    }

    if (bm1->h > 0) { /* to be sure */
      progress_update(1-y/(double)bm1->h, progress);
    }
  }

  pathlist_to_tree(plist, bm1);
  bm_free(bm1);
  *plistp = plist;

  progress_update(1.0, progress);

  return 0;

 error:
  bm_free(bm1);
  list_forall_unlink(p, plist) {
    path_free(p);
  }
  return -1;
}
