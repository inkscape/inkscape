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

#define SP_TYPE_MESH_CONTEXT            (sp_mesh_context_get_type())
#define SP_MESH_CONTEXT(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_MESH_CONTEXT, SPMeshContext))
#define SP_MESH_CONTEXT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_MESH_CONTEXT, SPMeshContextClass))
#define SP_IS_MESH_CONTEXT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_MESH_CONTEXT))
#define SP_IS_MESH_CONTEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_MESH_CONTEXT))

struct SPMeshContext : public SPEventContext {

    Geom::Point origin;

    bool cursor_addnode;

    bool node_added;

    Geom::Point mousepoint_doc; // stores mousepoint when over_line in doc coords

    Inkscape::MessageContext *_message_context;

    sigc::connection *selcon;
    sigc::connection *subselcon;
};

struct SPMeshContextClass {
    SPEventContextClass parent_class;
};

// Standard Gtk function
GType sp_mesh_context_get_type();

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
