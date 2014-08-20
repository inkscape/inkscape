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

#include "ui/tools/tool-base.h"
#include <2geom/point.h>

#define SAMPLING_SIZE 8        /* fixme: ?? */

#define TC_MIN_PRESSURE      0.0
#define TC_MAX_PRESSURE      1.0
#define TC_DEFAULT_PRESSURE  0.35

namespace Inkscape {
namespace UI {
namespace Tools {

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

class TweakTool : public ToolBase {
public:
	TweakTool();
	virtual ~TweakTool();

    /* extended input data */
    gdouble pressure;

    /* attributes */
    bool dragging;           /* mouse state: mouse is dragging */
    bool usepressure;
    bool usetilt;

    double width;
    double force;
    double fidelity;

    gint mode;

    bool is_drawing;

    bool is_dilating;
    bool has_dilated;
    Geom::Point last_push;
    SPCanvasItem *dilate_area;

    bool do_h;
    bool do_s;
    bool do_l;
    bool do_o;

  	sigc::connection style_set_connection;

	static const std::string prefsPath;

	virtual void setup();
	virtual void set(const Inkscape::Preferences::Entry& val);
	virtual bool root_handler(GdkEvent* event);

	virtual const std::string& getPrefsPath();

	void update_cursor(bool with_shift);

private:
	bool set_style(const SPCSSAttr* css);
};

}
}
}

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
