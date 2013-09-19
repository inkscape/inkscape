#ifndef SEEN_PEN_CONTEXT_H
#define SEEN_PEN_CONTEXT_H

/** \file 
 * SPPenContext: a context for pen tool events.
 */

#include "draw-context.h"
#include "live_effects/effect.h"

#define SP_PEN_CONTEXT(obj) (dynamic_cast<SPPenContext*>((SPEventContext*)obj))
#define SP_IS_PEN_CONTEXT(obj) (dynamic_cast<const SPPenContext*>((const SPEventContext*)obj) != NULL)

struct SPCtrlLine;

/**
 * SPPenContext: a context for pen tool events.
 */
class SPPenContext : public SPDrawContext {
public:
	SPPenContext();
	virtual ~SPPenContext();

	enum Mode {
	    MODE_CLICK,
	    MODE_DRAG
	};

	enum State {
	    POINT,
	    CONTROL,
	    CLOSE,
	    STOP
	};

    Geom::Point p[5];

    /** \invar npoints in {0, 2, 5}. */
    // npoints somehow determines the type of the node (what does it mean, exactly? the number of Bezier handles?)
    gint npoints;

    Mode mode;
    State state;

    bool polylines_only;
    bool polylines_paraxial;
    int num_clicks;

    unsigned int expecting_clicks_for_LPE; // if positive, finish the path after this many clicks
    Inkscape::LivePathEffect::Effect *waiting_LPE; // if NULL, waiting_LPE_type in SPDrawContext is taken into account
    SPLPEItem *waiting_item;

    SPCanvasItem *c0;
    SPCanvasItem *c1;

    SPCtrlLine *cl0;
    SPCtrlLine *cl1;
    
    unsigned int events_disabled : 1;

	static const std::string prefsPath;

	virtual const std::string& getPrefsPath();

protected:
	virtual void setup();
	virtual void finish();
	virtual void set(const Inkscape::Preferences::Entry& val);
	virtual bool root_handler(GdkEvent* event);
	virtual bool item_handler(SPItem* item, GdkEvent* event);
};

inline bool sp_pen_context_has_waiting_LPE(SPPenContext *pc) {
    // note: waiting_LPE_type is defined in SPDrawContext
    return (pc->waiting_LPE != NULL ||
            pc->waiting_LPE_type != Inkscape::LivePathEffect::INVALID_LPE);
}

void sp_pen_context_set_polyline_mode(SPPenContext *const pc);
void sp_pen_context_wait_for_LPE_mouse_clicks(SPPenContext *pc, Inkscape::LivePathEffect::EffectType effect_type,
                                              unsigned int num_clicks, bool use_polylines = true);
void sp_pen_context_cancel_waiting_for_LPE(SPPenContext *pc);
void sp_pen_context_put_into_waiting_mode(SPDesktop *desktop, Inkscape::LivePathEffect::EffectType effect_type,
                                          unsigned int num_clicks, bool use_polylines = true);

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
