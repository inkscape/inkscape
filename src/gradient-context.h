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

#include <sigc++/sigc++.h>
#include "event-context.h"

#define SP_TYPE_GRADIENT_CONTEXT            (sp_gradient_context_get_type())
#define SP_GRADIENT_CONTEXT(obj)            (GTK_CHECK_CAST((obj), SP_TYPE_GRADIENT_CONTEXT, SPGradientContext))
#define SP_GRADIENT_CONTEXT_CLASS(klass)    (GTK_CHECK_CLASS_CAST((klass), SP_TYPE_GRADIENT_CONTEXT, SPGradientContextClass))
#define SP_IS_GRADIENT_CONTEXT(obj)         (GTK_CHECK_TYPE((obj), SP_TYPE_GRADIENT_CONTEXT))
#define SP_IS_GRADIENT_CONTEXT_CLASS(klass) (GTK_CHECK_CLASS_TYPE((klass), SP_TYPE_GRADIENT_CONTEXT))

class SPGradientContext;
class SPGradientContextClass;

struct SPGradientContext : public SPEventContext {

    Geom::Point origin;

    bool cursor_addnode;

    bool node_added;

    Geom::Point mousepoint_doc; // stores mousepoint when over_line in doc coords

    Inkscape::MessageContext *_message_context;

    sigc::connection *selcon;
    sigc::connection *subselcon;
};

struct SPGradientContextClass {
    SPEventContextClass parent_class;
};

/* Standard Gtk function */
GtkType sp_gradient_context_get_type();

void sp_gradient_context_select_next (SPEventContext *event_context);
void sp_gradient_context_select_prev (SPEventContext *event_context);

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
