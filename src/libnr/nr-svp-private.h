#ifndef __NR_SVP_PRIVATE_H__
#define __NR_SVP_PRIVATE_H__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <libnr/nr-svp.h>

#define NR_QUANT_X 16.0F
#define NR_QUANT_Y 16.0F
#define NR_COORD_X_FROM_ART(v) (floor (NR_QUANT_X * (v) + 0.5F) / NR_QUANT_X)
#define NR_COORD_Y_FROM_ART(v) (floor (NR_QUANT_Y * (v) + 0.5F) / NR_QUANT_Y)
#define NR_COORD_TO_ART(v) (v)

/* NRVertex */

NRVertex *nr_vertex_new (void);
NRVertex *nr_vertex_new_xy (NR::Coord x, NR::Coord y);
void nr_vertex_free_one (NRVertex *v);
void nr_vertex_free_list (NRVertex *v);

NRVertex *nr_vertex_reverse_list (NRVertex *v);

#endif
