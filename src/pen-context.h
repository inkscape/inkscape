#ifndef SEEN_PEN_CONTEXT_H
#define SEEN_PEN_CONTEXT_H

/** \file 
 * SPPenContext: a context for pen tool events.
 */

#include "draw-context.h"

namespace Inkscape {
namespace LivePathEffect {
class Effect;
}
}

#define SP_TYPE_PEN_CONTEXT (sp_pen_context_get_type())
#define SP_PEN_CONTEXT(o) (G_TYPE_CHECK_INSTANCE_CAST((o), SP_TYPE_PEN_CONTEXT, SPPenContext))
#define SP_PEN_CONTEXT_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), SP_TYPE_PEN_CONTEXT, SPPenContextClass))
#define SP_IS_PEN_CONTEXT(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), SP_TYPE_PEN_CONTEXT))
#define SP_IS_PEN_CONTEXT_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE((k), SP_TYPE_PEN_CONTEXT))

enum {
    SP_PEN_CONTEXT_POINT,
    SP_PEN_CONTEXT_CONTROL,
    SP_PEN_CONTEXT_CLOSE,
    SP_PEN_CONTEXT_STOP
};

enum {
    SP_PEN_CONTEXT_MODE_CLICK,
    SP_PEN_CONTEXT_MODE_DRAG
};

/**
 * SPPenContext: a context for pen tool events.
 */
struct SPPenContext : public SPDrawContext {
    NR::Point p[5];

    /** \invar npoints in {0, 2, 5}. */
    gint npoints;

    unsigned int mode : 1;
    unsigned int state : 2;

    bool polylines_only;

    unsigned int expecting_clicks_for_LPE; // if positive, finish the path after this many clicks
    Inkscape::LivePathEffect::Effect *waiting_LPE;
    SPLPEItem *waiting_item;

    SPCanvasItem *c0, *c1, *cl0, *cl1;
    
    unsigned int events_disabled : 1;
};

/// The SPPenContext vtable (empty).
struct SPPenContextClass : public SPEventContextClass { };

GType sp_pen_context_get_type();


#endif /* !SEEN_PEN_CONTEXT_H */

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
