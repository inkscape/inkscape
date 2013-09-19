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
#include "event-context.h"

#define SP_GRADIENT_CONTEXT(obj) (dynamic_cast<SPGradientContext*>((SPEventContext*)obj))
#define SP_IS_GRADIENT_CONTEXT(obj) (dynamic_cast<const SPGradientContext*>((const SPEventContext*)obj) != NULL)

class SPGradientContext : public SPEventContext {
public:
	SPGradientContext();
	virtual ~SPGradientContext();

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

void sp_gradient_context_select_next (SPEventContext *event_context);
void sp_gradient_context_select_prev (SPEventContext *event_context);
void sp_gradient_context_add_stops_between_selected_stops (SPGradientContext *rc);

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
