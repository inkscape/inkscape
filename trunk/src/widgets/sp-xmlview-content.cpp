#define __SP_XMLVIEW_CONTENT_C__

/*
 * Specialization of GtkTextView for the XML tree view
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2002 MenTaLguY
 *
 * Released under the GNU GPL; see COPYING for details
 */

#include <cstring>
#include <glibmm/i18n.h>

#include "xml/node-event-vector.h"
#include "sp-xmlview-content.h"
#include "desktop-handles.h"
#include "document-private.h"
#include "inkscape.h"

static void sp_xmlview_content_class_init (SPXMLViewContentClass * klass);
static void sp_xmlview_content_init (SPXMLViewContent * text);
static void sp_xmlview_content_destroy (GtkObject * object);

void sp_xmlview_content_changed (GtkTextBuffer *tb, SPXMLViewContent *text);

static void event_content_changed (Inkscape::XML::Node * repr, const gchar * old_content, const gchar * new_content, gpointer data);

static GtkTextViewClass * parent_class = NULL;

static Inkscape::XML::NodeEventVector repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    NULL, /* attr_changed */
    event_content_changed,
    NULL  /* order_changed */
};

GtkWidget *
sp_xmlview_content_new (Inkscape::XML::Node * repr)
{
    GtkTextBuffer *tb;
    SPXMLViewContent *text;

    tb = gtk_text_buffer_new (NULL);
    text = (SPXMLViewContent*)gtk_type_new (SP_TYPE_XMLVIEW_CONTENT);
    gtk_text_view_set_buffer (GTK_TEXT_VIEW (text), tb);
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (text), GTK_WRAP_CHAR);

    g_signal_connect (G_OBJECT (tb), "changed", G_CALLBACK (sp_xmlview_content_changed), text);

    /* should we alter the scrolling adjustments here? */

    sp_xmlview_content_set_repr (text, repr);

    return (GtkWidget *) text;
}

void
sp_xmlview_content_set_repr (SPXMLViewContent * text, Inkscape::XML::Node * repr)
{
    if ( repr == text->repr ) return;
    if (text->repr) {
        sp_repr_remove_listener_by_data (text->repr, text);
        Inkscape::GC::release(text->repr);
    }
    text->repr = repr;
    if (repr) {
        Inkscape::GC::anchor(repr);
        sp_repr_add_listener (repr, &repr_events, text);
        sp_repr_synthesize_events (repr, &repr_events, text);
    } else {
        gtk_text_buffer_set_text (gtk_text_view_get_buffer (GTK_TEXT_VIEW (text)), "", 0);
        gtk_text_view_set_editable (GTK_TEXT_VIEW (text), FALSE);
    }
}

GType sp_xmlview_content_get_type(void)
{
    static GtkType type = 0;

    if (!type) {
        GTypeInfo info = {
            sizeof(SPXMLViewContentClass),
            0, // base_init
            0, // base_finalize
            (GClassInitFunc)sp_xmlview_content_class_init,
            0, // class_finalize
            0, // class_data
            sizeof(SPXMLViewContent),
            0, // n_preallocs
            (GInstanceInitFunc)sp_xmlview_content_init,
            0 // value_table
        };
        type = g_type_register_static(GTK_TYPE_TEXT_VIEW, "SPXMLViewContent", &info, static_cast<GTypeFlags>(0));
    }

    return type;
}

void
sp_xmlview_content_class_init (SPXMLViewContentClass * klass)
{
    GtkObjectClass * object_class;

    object_class = (GtkObjectClass *) klass;

    parent_class = (GtkTextViewClass*)gtk_type_class (GTK_TYPE_TEXT_VIEW);

    object_class->destroy = sp_xmlview_content_destroy;
}

void
sp_xmlview_content_init (SPXMLViewContent *text)
{
    text->repr = NULL;
    text->blocked = FALSE;
}

void
sp_xmlview_content_destroy (GtkObject * object)
{
    SPXMLViewContent * text;

    text = SP_XMLVIEW_CONTENT (object);

    sp_xmlview_content_set_repr (text, NULL);

    GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

void
event_content_changed (Inkscape::XML::Node * /*repr*/, const gchar * /*old_content*/, const gchar * new_content, gpointer data)
{
    SPXMLViewContent * text;
    text = SP_XMLVIEW_CONTENT (data);

    if (text->blocked) return;

    text->blocked = TRUE;

    if (new_content) {
        gtk_text_buffer_set_text (gtk_text_view_get_buffer (GTK_TEXT_VIEW (text)), new_content, strlen (new_content));
    } else {
        gtk_text_buffer_set_text (gtk_text_view_get_buffer (GTK_TEXT_VIEW (text)), "", 0);
    }
    gtk_text_view_set_editable (GTK_TEXT_VIEW (text), new_content != NULL);

    text->blocked = FALSE;
}

void
sp_xmlview_content_changed (GtkTextBuffer *tb, SPXMLViewContent *text)
{
    if (text->blocked) return;

    if (text->repr) {
        GtkTextIter start, end;
        gchar *data;
        text->blocked = TRUE;
        gtk_text_buffer_get_bounds (tb, &start, &end);
        data = gtk_text_buffer_get_text (tb, &start, &end, TRUE);
        text->repr->setContent(data);
        g_free (data);
        text->blocked = FALSE;
        sp_document_done (sp_desktop_document (SP_ACTIVE_DESKTOP), SP_VERB_DIALOG_XML_EDITOR,
                          _("Type text in a text node"));
    }
}
