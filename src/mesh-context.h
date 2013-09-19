#ifndef SEEN_SP_MESH_CONTEXT_H
#define SEEN_SP_MESH_CONTEXT_H

/*
 * Mesh drawing and editing tool
 *
 * Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Jon A. Cruz <jon@joncruz.org.
 *   Tavmjong Bah <tavmjong@free.fr>
 *
 * Copyright (C) 2012 Tavmjong Bah
 * Copyright (C) 2007 Johan Engelen
 * Copyright (C) 2005,2010 Authors
 *
 * Released under GNU GPL
 */

#include <stddef.h>
#include <sigc++/sigc++.h>
#include "event-context.h"

#define SP_MESH_CONTEXT(obj) (dynamic_cast<SPMeshContext*>((SPEventContext*)obj))
#define SP_IS_MESH_CONTEXT(obj) (dynamic_cast<const SPMeshContext*>((const SPEventContext*)obj) != NULL)

class SPMeshContext : public SPEventContext {
public:
	SPMeshContext();
	virtual ~SPMeshContext();

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
	void selection_changed(Inkscape::Selection* sel);
};

void sp_mesh_context_select_next(SPEventContext *event_context);
void sp_mesh_context_select_prev(SPEventContext *event_context);

#endif // SEEN_SP_MESH_CONTEXT_H


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
