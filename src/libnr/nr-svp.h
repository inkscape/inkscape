#ifndef __NR_SVP_H__
#define __NR_SVP_H__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

/* Sorted vector paths */

#include <glib/gtypes.h>
#include <libnr/nr-coord.h>
#include <libnr/nr-forward.h>

struct NRSVPSegment {
    gint16 wind;
    guint16 length;
    guint32 start;
    float x0, x1;
};

struct NRSVPFlat {
    gint16 wind;
    guint16 length;
    float y;
    float x0, x1;
};

struct NRSVP {
    unsigned int length;
    NR::Point *points;
    NRSVPSegment segments[1];
};

#define NR_SVPSEG_IS_FLAT(s,i) (!(s)->segments[i].length)

void nr_svp_free (NRSVP *svp);

void nr_svp_bbox (NRSVP *svp, NRRect *bbox, unsigned int clear);

/* Sorted vertex lists */

struct NRVertex {
    NRVertex *next;
    NR::Coord x, y;
};


#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
