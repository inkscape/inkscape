#ifndef COMMON_CONTEXT_H_SEEN
#define COMMON_CONTEXT_H_SEEN

/*
 * Common drawing mode
 *
 * Authors:
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * The original dynadraw code:
 *   Paul Haeberli <paul@sgi.com>
 *
 * Copyright (C) 1998 The Free Software Foundation
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 * Copyright (C) 2008 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "event-context.h"
#include "display/curve.h"
#include "display/display-forward.h"
#include <2geom/point.h>

#define SP_TYPE_COMMON_CONTEXT (sp_common_context_get_type())
#define SP_COMMON_CONTEXT(o) (GTK_CHECK_CAST((o), SP_TYPE_COMMON_CONTEXT, SPCommonContext))
#define SP_COMMON_CONTEXT_CLASS(k) (GTK_CHECK_CLASS_CAST((k), SP_TYPE_COMMON_CONTEXT, SPCommonContextClass))
#define SP_IS_COMMON_CONTEXT(o) (GTK_CHECK_TYPE((o), SP_TYPE_COMMON_CONTEXT))
#define SP_IS_COMMON_CONTEXT_CLASS(k) (GTK_CHECK_CLASS_TYPE((k), SP_TYPE_COMMON_CONTEXT))

class SPCommonContext;
class SPCommonContextClass;

#define SAMPLING_SIZE 8        /* fixme: ?? */


struct SPCommonContext : public SPEventContext {
    /** accumulated shape which ultimately goes in svg:path */
    SPCurve *accumulated;

    /** canvas items for "comitted" segments */
    GSList *segments;

    /** canvas item for red "leading" segment */
    SPCanvasItem *currentshape;
    /** shape of red "leading" segment */
    SPCurve *currentcurve;

    /** left edge of the stroke; combined to get accumulated */
    SPCurve *cal1;

    /** right edge of the stroke; combined to get accumulated */
    SPCurve *cal2;

    /** left edge points for this segment */
    Geom::Point point1[SAMPLING_SIZE];

    /** right edge points for this segment */
    Geom::Point point2[SAMPLING_SIZE];

    /** number of edge points for this segment */
    gint npoints;

    /* repr */
    Inkscape::XML::Node *repr;

    /* common */
    Geom::Point cur;
    Geom::Point vel;
    double vel_max;
    Geom::Point acc;
    Geom::Point ang;
    Geom::Point last;
    Geom::Point del;

    /* extended input data */
    gdouble pressure;
    gdouble xtilt;
    gdouble ytilt;

    /* attributes */
    guint dragging : 1;           /* mouse state: mouse is dragging */
    guint usepressure : 1;
    guint usetilt : 1;
    double mass, drag;
    double angle;
    double width;

    double vel_thin;
    double flatness;
    double tremor;
    double cap_rounding;

    Inkscape::MessageContext *_message_context;

    bool is_drawing;

    /** uses absolute width independent of zoom */
    bool abs_width;
};

struct SPCommonContextClass : public SPEventContextClass{};

GType sp_common_context_get_type(void);

#endif // COMMON_CONTEXT_H_SEEN

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

