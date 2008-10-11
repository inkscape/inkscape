#ifndef __SP_TWEAK_CONTEXT_H__
#define __SP_TWEAK_CONTEXT_H__

/*
 * tweaking paths without node editing
 *
 * Authors:
 *   bulia byak 
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "event-context.h"
#include <display/display-forward.h>
#include <libnr/nr-point.h>

#define SP_TYPE_TWEAK_CONTEXT (sp_tweak_context_get_type())
#define SP_TWEAK_CONTEXT(o) (GTK_CHECK_CAST((o), SP_TYPE_TWEAK_CONTEXT, SPTweakContext))
#define SP_TWEAK_CONTEXT_CLASS(k) (GTK_CHECK_CLASS_CAST((k), SP_TYPE_TWEAK_CONTEXT, SPTweakContextClass))
#define SP_IS_TWEAK_CONTEXT(o) (GTK_CHECK_TYPE((o), SP_TYPE_TWEAK_CONTEXT))
#define SP_IS_TWEAK_CONTEXT_CLASS(k) (GTK_CHECK_CLASS_TYPE((k), SP_TYPE_TWEAK_CONTEXT))

class SPTweakContext;
class SPTweakContextClass;

#define SAMPLING_SIZE 8        /* fixme: ?? */

#define TC_MIN_PRESSURE      0.0
#define TC_MAX_PRESSURE      1.0
#define TC_DEFAULT_PRESSURE  0.35

enum {
    TWEAK_MODE_MOVE,
    TWEAK_MODE_MOVE_IN_OUT,
    TWEAK_MODE_MOVE_JITTER,
    TWEAK_MODE_SCALE,
    TWEAK_MODE_ROTATE,
    TWEAK_MODE_MORELESS,
    TWEAK_MODE_PUSH,
    TWEAK_MODE_SHRINK_GROW,
    TWEAK_MODE_ATTRACT_REPEL,
    TWEAK_MODE_ROUGHEN,
    TWEAK_MODE_COLORPAINT,
    TWEAK_MODE_COLORJITTER,
    TWEAK_MODE_BLUR
};

struct SPTweakContext
{
    SPEventContext event_context;

    /* extended input data */
    gdouble pressure;

    /* attributes */
    guint dragging : 1;           /* mouse state: mouse is dragging */
    guint usepressure : 1;
    guint usetilt : 1;

    double width;
    double force;
    double fidelity;

    gint mode;

    Inkscape::MessageContext *_message_context;

    bool is_drawing;

    bool is_dilating;
    bool has_dilated;
    NR::Point last_push;
    SPCanvasItem *dilate_area;

    bool do_h;
    bool do_s;
    bool do_l;
    bool do_o;

  	sigc::connection style_set_connection;
};

struct SPTweakContextClass
{
    SPEventContextClass parent_class;
};

GtkType sp_tweak_context_get_type(void);

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
