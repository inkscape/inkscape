/*
 * Specialization of GtkTextView for the XML tree view
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2002 MenTaLguY
 *   Abhishek Sharma
 *
 * Released under the GNU GPL; see COPYING for details
 */

#include <cstring>
#include <glibmm/i18n.h>

#include "xml/node-event-vector.h"
#include "sp-xmlview-content.h"
#include "desktop.h"
#include "document-private.h"
#include "document-undo.h"
#include "inkscape.h"
#include "verbs.h"

using Inkscape::DocumentUndo;

#if GTK_CHECK_VERSION(3,0,0)
static void sp_xmlview_content_destroy(GtkWidget * object);
#else
static void sp_xmlview_content_destroy(GtkObject * object);
#endif

void sp_xmlview_content_changed (GtkTextBuffer *tb, SPXMLViewContent *text);

static void event_content_changed (Inkscape::XML::Node * repr, const gchar * old_content, const gchar * new_content, gpointer data);

static Inkscape::XML::NodeEventVector repr_events = {
    NULL, /* child_added */
    NULL, /* child_removed */
    NULL, /* attr_changed */
    event_content_changed,
    NULL  /* order_changed */
};

GtkWidget *sp_xmlview_content_new(Inkscape::XML::Node * repr)
{
    GtkTextBuffer *tb = gtk_text_buffer_new(NULL);
    SPXMLViewContent *text = SP_XMLVIEW_CONTENT(g_object_new(SP_TYPE_XMLVIEW_CONTENT, NULL));
    gtk_text_view_set_buffer (GTK_TEXT_VIEW (text), tb);
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (text), GTK_WRAP_CHAR);

    g_signal_connect (G_OBJECT (tb), "changed", G_CALLBACK (sp_xmlview_content_changed), text);

    /* should we alter the scrolling adjustments here? */

    sp_xmlview_content_set_repr (text, repr);

    return GTK_WIDGET(text);
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

G_DEFINE_TYPE(SPXMLViewContent, sp_xmlview_content, GTK_TYPE_TEXT_VIEW);

void sp_xmlview_content_class_init(SPXMLViewContentClass * klass)
{
#if GTK_CHECK_VERSION(3,0,0)
    GtkWidgetClass * widget_class = GTK_WIDGET_CLASS(klass);
    widget_class->destroy = sp_xmlview_content_destroy;
#else
    GtkObjectClass * object_class = GTK_OBJECT_CLASS(klass);
    object_class->destroy = sp_xmlview_content_destroy;
#endif
}

void
sp_xmlview_content_init (SPXMLViewContent *text)
{
    text->repr = NULL;
    text->blocked = FALSE;
}

#if GTK_CHECK_VERSION(3,0,0)
void sp_xmlview_content_destroy(GtkWidget * object)
#else
void sp_xmlview_content_destroy(GtkObject * object)
#endif
{
    SPXMLViewContent * text = SP_XMLVIEW_CONTENT (object);

    sp_xmlview_content_set_repr (text, NULL);

#if GTK_CHECK_VERSION(3,0,0)
    GTK_WIDGET_CLASS (sp_xmlview_content_parent_class)->destroy (object);
#else
    GTK_OBJECT_CLASS (sp_xmlview_content_parent_class)->destroy (object);
#endif
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
        DocumentUndo::done(SP_ACTIVE_DESKTOP->getDocument(), SP_VERB_DIALOG_XML_EDITOR,
			   _("Type text in a text node"));
    }
}
