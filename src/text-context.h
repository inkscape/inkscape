#ifndef __SP_TEXT_CONTEXT_H__
#define __SP_TEXT_CONTEXT_H__

/*
 * SPTextContext
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 1999-2005 authors
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/*  #include <gdk/gdkic.h> */
#include <sigc++/sigc++.h>
#include <gtk/gtkimcontext.h>

#include "event-context.h"
#include <display/display-forward.h>
#include <2geom/point.h>
#include "libnrtype/Layout-TNG.h"

#define SP_TYPE_TEXT_CONTEXT (sp_text_context_get_type ())
#define SP_TEXT_CONTEXT(obj) (GTK_CHECK_CAST ((obj), SP_TYPE_TEXT_CONTEXT, SPTextContext))
#define SP_TEXT_CONTEXT_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_TEXT_CONTEXT, SPTextContextClass))
#define SP_IS_TEXT_CONTEXT(obj) (GTK_CHECK_TYPE ((obj), SP_TYPE_TEXT_CONTEXT))
#define SP_IS_TEXT_CONTEXT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_TEXT_CONTEXT))

class SPTextContext;
class SPTextContextClass;
class SPCanvasArena;

struct SPTextContext : public SPEventContext {

    sigc::connection sel_changed_connection;
    sigc::connection sel_modified_connection;
    sigc::connection style_set_connection;
    sigc::connection style_query_connection;

    GtkIMContext *imc;

    SPItem *text; // the text we're editing, or NULL if none selected

    /* Text item position in root coordinates */
    Geom::Point pdoc;
    /* Insertion point position */
    Inkscape::Text::Layout::iterator text_sel_start;
    Inkscape::Text::Layout::iterator text_sel_end;

    gchar uni[9];
    bool unimode;
    guint unipos;

    SPCanvasItem *cursor;
    SPCanvasItem *indicator;
    SPCanvasItem *frame; // hiliting the first frame of flowtext; FIXME: make this a list to accommodate arbitrarily many chained shapes
    std::vector<SPCanvasItem*> text_selection_quads;
    gint timeout;
    guint show : 1;
    guint phase : 1;
    guint nascent_object : 1; // true if we're clicked on canvas to put cursor, but no text typed yet so ->text is still NULL

    guint over_text : 1; // true if cursor is over a text object

    guint dragging : 2; // dragging selection over text

    guint creating : 1; // dragging rubberband to create flowtext
    SPCanvasItem *grabbed; // we grab while we are creating, to get events even if the mouse goes out of the window
    Geom::Point p0; // initial point if the flowtext rect

    /* Preedit String */
    gchar* preedit_string;
};

struct SPTextContextClass {
    SPEventContextClass parent_class;
};

/* Standard Gtk function */
GtkType sp_text_context_get_type (void);

bool sp_text_paste_inline(SPEventContext *ec);
Glib::ustring sp_text_get_selected_text(SPEventContext const *ec);
SPCSSAttr *sp_text_get_style_at_cursor(SPEventContext const *ec);
bool sp_text_delete_selection(SPEventContext *ec);
void sp_text_context_place_cursor (SPTextContext *tc, SPObject *text, Inkscape::Text::Layout::iterator where);
void sp_text_context_place_cursor_at (SPTextContext *tc, SPObject *text, Geom::Point const p);
Inkscape::Text::Layout::iterator *sp_text_context_get_cursor_position(SPTextContext *tc, SPObject *text);

#endif
