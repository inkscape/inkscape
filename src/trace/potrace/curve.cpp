/* Copyright (C) 2001-2011 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */

/* private part of the path and curve data structures */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "potracelib.h"
#include "lists.h"
#include "curve.h"

#define SAFE_MALLOC(var, n, typ) \
  if ((var = (typ *)malloc((n)*sizeof(typ))) == NULL) goto malloc_error 

/* ---------------------------------------------------------------------- */
/* allocate and free path objects */

path_t *path_new(void) {
  path_t *p = NULL;
  privpath_t *priv = NULL;

  SAFE_MALLOC(p, 1, path_t);
  memset(p, 0, sizeof(path_t));
  SAFE_MALLOC(priv, 1, privpath_t);
  memset(priv, 0, sizeof(privpath_t));
  p->priv = priv;
  return p;

 malloc_error:
  free(p);
  free(priv);
  return NULL;
}

/* free the members of the given curve structure. Leave errno unchanged. */
static void privcurve_free_members(privcurve_t *curve) {
  free(curve->tag);
  free(curve->c);
  free(curve->vertex);
  free(curve->alpha);
  free(curve->alpha0);
  free(curve->beta);
}

/* free a path. Leave errno untouched. */
void path_free(path_t *p) {
  if (p) {
    if (p->priv) {
      free(p->priv->pt);
      free(p->priv->lon);
      free(p->priv->sums);
      free(p->priv->po);
      privcurve_free_members(&p->priv->curve);
      privcurve_free_members(&p->priv->ocurve);
    }
    free(p->priv);
    /* do not free p->fcurve ! */
  }
  free(p);
}  

/* free a pathlist, leaving errno untouched. */
void pathlist_free(path_t *plist) {
  path_t *p;

  list_forall_unlink(p, plist) {
    path_free(p);
  }
}

/* ---------------------------------------------------------------------- */
/* initialize and finalize curve structures */

typedef dpoint_t dpoint3_t[3];

/* initialize the members of the given curve structure to size m.
   Return 0 on success, 1 on error with errno set. */
int privcurve_init(privcurve_t *curve, int n) {
  memset(curve, 0, sizeof(privcurve_t));
  curve->n = n;
  SAFE_MALLOC(curve->tag, n, int);
  SAFE_MALLOC(curve->c, n, dpoint3_t);
  SAFE_MALLOC(curve->vertex, n, dpoint_t);
  SAFE_MALLOC(curve->alpha, n, double);
  SAFE_MALLOC(curve->alpha0, n, double);
  SAFE_MALLOC(curve->beta, n, double);
  return 0;

 malloc_error:
  free(curve->tag);
  free(curve->c);
  free(curve->vertex);
  free(curve->alpha);
  free(curve->alpha0);
  free(curve->beta);
  return 1;
}

/* copy private to public curve structure */
void privcurve_to_curve(privcurve_t *pc, potrace_curve_t *c) {
  c->n = pc->n;
  c->tag = pc->tag;
  c->c = pc->c;
}
    
