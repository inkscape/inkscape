#ifndef SP_ERASER_CONTEXT_H_SEEN
#define SP_ERASER_CONTEXT_H_SEEN

/*
 * Handwriting-like drawing mode
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

#include "display/curve.h"
#include "event-context.h"
#include <display/display-forward.h>
#include <libnr/nr-point.h>

#define SP_TYPE_ERASER_CONTEXT (sp_eraser_context_get_type())
#define SP_ERASER_CONTEXT(o) (GTK_CHECK_CAST((o), SP_TYPE_ERASER_CONTEXT, SPEraserContext))
#define SP_ERASER_CONTEXT_CLASS(k) (GTK_CHECK_CLASS_CAST((k), SP_TYPE_ERASER_CONTEXT, SPEraserContextClass))
#define SP_IS_ERASER_CONTEXT(o) (GTK_CHECK_TYPE((o), SP_TYPE_ERASER_CONTEXT))
#define SP_IS_ERASER_CONTEXT_CLASS(k) (GTK_CHECK_CLASS_TYPE((k), SP_TYPE_ERASER_CONTEXT))

class SPEraserContext;
class SPEraserContextClass;

#define SAMPLING_SIZE 8        /* fixme: ?? */

#define ERC_MIN_PRESSURE      0.0
#define ERC_MAX_PRESSURE      1.0
#define ERC_DEFAULT_PRESSURE  1.0

#define ERC_MIN_TILT         -1.0
#define ERC_MAX_TILT          1.0
#define ERC_DEFAULT_TILT      0.0

struct SPEraserContext
{
    SPEventContext event_context;

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
    NR::Point point1[SAMPLING_SIZE];
    /** right edge points for this segment */
    NR::Point point2[SAMPLING_SIZE];
    /** number of edge points for this segment */
    gint npoints;

    /* repr */
    Inkscape::XML::Node *repr;

    /* Eraser */
    NR::Point cur;
    NR::Point vel;
    double vel_max;
    NR::Point acc;
    NR::Point ang;
    NR::Point last;
    NR::Point del;
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

struct SPEraserContextClass
{
    SPEventContextClass parent_class;
};

GtkType sp_eraser_context_get_type(void);

#endif // SP_ERASER_CONTEXT_H_SEEN

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
