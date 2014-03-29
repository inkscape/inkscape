#ifndef SEEN_PENCIL_CONTEXT_H
#define SEEN_PENCIL_CONTEXT_H

/** \file
 * PencilTool: a context for pencil tool events
 */

#include "ui/tools/freehand-base.h"

#include <2geom/piecewise.h>
#include <2geom/d2.h>
#include <2geom/sbasis.h>

#define SP_PENCIL_CONTEXT(obj) (dynamic_cast<Inkscape::UI::Tools::PencilTool*>((ToolBase*)obj))
#define SP_IS_PENCIL_CONTEXT(obj) (dynamic_cast<const Inkscape::UI::Tools::PencilTool*>((const ToolBase*)obj) != NULL)

namespace Inkscape {
namespace UI {
namespace Tools {

enum PencilState {
    SP_PENCIL_CONTEXT_IDLE,
    SP_PENCIL_CONTEXT_ADDLINE,
    SP_PENCIL_CONTEXT_FREEHAND,
    SP_PENCIL_CONTEXT_SKETCH
};

/**
 * PencilTool: a context for pencil tool events
 */
class PencilTool : public FreehandBase {
public:
	PencilTool();
	virtual ~PencilTool();

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

private:
	bool _handleButtonPress(GdkEventButton const &bevent);
	bool _handleMotionNotify(GdkEventMotion const &mevent);
	bool _handleButtonRelease(GdkEventButton const &revent);
	bool _handleKeyPress(GdkEventKey const &event);
	bool _handleKeyRelease(GdkEventKey const &event);

	void _setStartpoint(Geom::Point const &p);
	void _setEndpoint(Geom::Point const &p);
	void _finishEndpoint();
	void _addFreehandPoint(Geom::Point const &p, guint state);
	void _fitAndSplit();
	void _interpolate();
	void _sketchInterpolate();

	void _cancel();
	void _endpointSnap(Geom::Point &p, guint const state);
};

}
}
}

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
