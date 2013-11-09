#ifndef __SP_GRADIENT_CONTEXT_H__
#define __SP_GRADIENT_CONTEXT_H__

/*
 * Gradient drawing and editing tool
 *
 * Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Jon A. Cruz <jon@joncruz.org.
 *
 * Copyright (C) 2007 Johan Engelen
 * Copyright (C) 2005,2010 Authors
 *
 * Released under GNU GPL
 */

#include <stddef.h>
#include <sigc++/sigc++.h>
#include "ui/tools/tool-base.h"

#define SP_GRADIENT_CONTEXT(obj) (dynamic_cast<Inkscape::UI::Tools::GradientTool*>((Inkscape::UI::Tools::ToolBase*)obj))
#define SP_IS_GRADIENT_CONTEXT(obj) (dynamic_cast<const Inkscape::UI::Tools::GradientTool*>((const Inkscape::UI::Tools::ToolBase*)obj) != NULL)

namespace Inkscape {
namespace UI {
namespace Tools {

class GradientTool : public ToolBase {
public:
	GradientTool();
	virtual ~GradientTool();

    Geom::Point origin;

    bool cursor_addnode;

    bool node_added;

    Geom::Point mousepoint_doc; // stores mousepoint when over_line in doc coords

    sigc::connection *selcon;
    sigc::connection *subselcon;

	static const std::string prefsPath;

	virtual void setup();
	virtual bool root_handler(GdkEvent* event);

	virtual const std::string& getPrefsPath();

private:
	void selection_changed(Inkscape::Selection*);
};

void sp_gradient_context_select_next (ToolBase *event_context);
void sp_gradient_context_select_prev (ToolBase *event_context);
void sp_gradient_context_add_stops_between_selected_stops (GradientTool *rc);

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
