#ifndef SEEN_PENCIL_CONTEXT_H
#define SEEN_PENCIL_CONTEXT_H

/** \file
 * SPPencilContext: a context for pencil tool events
 */

#include "draw-context.h"

#define SP_PENCIL_CONTEXT(obj) (dynamic_cast<SPPencilContext*>((SPEventContext*)obj))
#define SP_IS_PENCIL_CONTEXT(obj) (dynamic_cast<const SPPencilContext*>((const SPEventContext*)obj) != NULL)

enum PencilState {
    SP_PENCIL_CONTEXT_IDLE,
    SP_PENCIL_CONTEXT_ADDLINE,
    SP_PENCIL_CONTEXT_FREEHAND,
    SP_PENCIL_CONTEXT_SKETCH
};

/**
 * SPPencilContext: a context for pencil tool events
 */
class SPPencilContext : public SPDrawContext {
public:
	SPPencilContext();
	virtual ~SPPencilContext();

    Geom::Point p[16];
    gint npoints;
    PencilState state;
    Geom::Point req_tangent;

    bool is_drawing;

    std::vector<Geom::Point> ps;

    Geom::Piecewise<Geom::D2<Geom::SBasis> > sketch_interpolation; // the current proposal from the sketched paths
    unsigned sketch_n; // number of sketches done

	static const std::string prefsPath;

	virtual const std::string& getPrefsPath();

protected:
	virtual void setup();
	virtual bool root_handler(GdkEvent* event);
};

#endif /* !SEEN_PENCIL_CONTEXT_H */

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
