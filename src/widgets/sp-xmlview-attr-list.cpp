/*
 * Specialization of GtkCList for the XML tree view
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2002 MenTaLguY
 *
 * Released under the GNU GPL; see COPYING for details
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <cstring>
#include <glibmm/i18n.h>

#include "helper/sp-marshal.h"
#include "../xml/node-event-vector.h"
#include "sp-xmlview-attr-list.h"

static void sp_xmlview_attr_list_class_init (SPXMLViewAttrListClass * klass);
static void sp_xmlview_attr_list_init (SPXMLViewAttrList * list);
static void sp_xmlview_attr_list_destroy (GtkObject * object);

static void event_attr_changed (Inkscape::XML::Node * repr, const gchar * name, const gchar * old_value, const gchar * new_value, bool is_interactive, gpointer data);

static GtkCListClass * parent_class = NULL;

static Inkscape::XML::NodeEventVector repr_events = {
	NULL, /* child_added */
	NULL, /* child_removed */
	event_attr_changed,
	NULL, /* content_changed */
	NULL  /* order_changed */
};

GtkWidget *
sp_xmlview_attr_list_new (Inkscape::XML::Node * repr)
{
	SPXMLViewAttrList * list;

	list = (SPXMLViewAttrList*)g_object_new (SP_TYPE_XMLVIEW_ATTR_LIST, "n_columns", 2, NULL);

	gtk_clist_set_column_title (GTK_CLIST (list), 0, _("Attribute"));
	gtk_clist_set_column_title (GTK_CLIST (list), 1, _("Value"));
	gtk_clist_column_titles_show (GTK_CLIST (list));

	gtk_clist_column_titles_passive (GTK_CLIST (list));
	gtk_clist_set_column_auto_resize (GTK_CLIST (list), 0, TRUE);
	gtk_clist_set_column_auto_resize (GTK_CLIST (list), 1, TRUE);
	gtk_clist_set_sort_column (GTK_CLIST (list), 0);
	gtk_clist_set_auto_sort (GTK_CLIST (list), TRUE);

	sp_xmlview_attr_list_set_repr (list, repr);

	return (GtkWidget *) list;
}

void
sp_xmlview_attr_list_set_repr (SPXMLViewAttrList * list, Inkscape::XML::Node * repr)
{
	if ( repr == list->repr ) return;
	gtk_clist_freeze (GTK_CLIST (list));
	if (list->repr) {
		gtk_clist_clear (GTK_CLIST (list));
		sp_repr_remove_listener_by_data (list->repr, list);
		Inkscape::GC::release(list->repr);
	}
	list->repr = repr;
	if (repr) {
		Inkscape::GC::anchor(repr);
		sp_repr_add_listener (repr, &repr_events, list);
		sp_repr_synthesize_events (repr, &repr_events, list);
	}
	gtk_clist_thaw (GTK_CLIST (list));
}

GType sp_xmlview_attr_list_get_type(void)
{
    static GType type = 0;

    if (!type) {
        GTypeInfo info = {
            sizeof(SPXMLViewAttrListClass),
            0, // base_init
            0, // base_finalize
            (GClassInitFunc)sp_xmlview_attr_list_class_init,
            0, // class_finalize
            0, // class_data
            sizeof(SPXMLViewAttrList),
            0, // n_preallocs
            (GInstanceInitFunc)sp_xmlview_attr_list_init,
            0 // value_table
        };
        type = g_type_register_static(GTK_TYPE_CLIST, "SPXMLViewAttrList", &info, static_cast<GTypeFlags>(0));
    }

    return type;
}

void
sp_xmlview_attr_list_class_init (SPXMLViewAttrListClass * klass)
{
	GtkObjectClass * object_class;

	object_class = (GtkObjectClass *) klass;
	object_class->destroy = sp_xmlview_attr_list_destroy;

	parent_class = (GtkCListClass*)gtk_type_class (GTK_TYPE_CLIST);

        g_signal_new (  "row-value-changed",
			G_TYPE_FROM_CLASS(klass),
			G_SIGNAL_RUN_FIRST,
			G_STRUCT_OFFSET (SPXMLViewAttrListClass, row_changed),
			NULL, NULL,
			g_cclosure_marshal_VOID__UINT,
			G_TYPE_NONE, 1,
			G_TYPE_UINT);
}

void
sp_xmlview_attr_list_init (SPXMLViewAttrList * list)
{
	list->repr = NULL;
}

void
sp_xmlview_attr_list_destroy (GtkObject * object)
{
	SPXMLViewAttrList * list;

	list = SP_XMLVIEW_ATTR_LIST (object);

	sp_xmlview_attr_list_set_repr (list, NULL);

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

void
event_attr_changed (Inkscape::XML::Node * /*repr*/,
                    const gchar * name,
                    const gchar * /*old_value*/,
                    const gchar * new_value,
                    bool /*is_interactive*/,
                    gpointer data)
{
	gint row;
	SPXMLViewAttrList * list;
	gchar new_text[128 + 4];
	gchar *gtktext;

	list = SP_XMLVIEW_ATTR_LIST (data);

	gtk_clist_freeze (GTK_CLIST (list));

	if (new_value) {
		strncpy (new_text, new_value, 128);
		if (strlen (new_value) >= 128) {
			strcpy (new_text + 128, "...");
		}
		gtktext = new_text;
	} else {
		gtktext = NULL;
	}

	row = gtk_clist_find_row_from_data (GTK_CLIST (list), GINT_TO_POINTER (g_quark_from_string (name)));
	if (row != -1) {
		if (new_value) {
			gtk_clist_set_text (GTK_CLIST (list), row, 1, gtktext);
		} else {
			gtk_clist_remove (GTK_CLIST (list), row);
		}
	} else if (new_value != NULL) {
		const gchar * text[2];

		text[0] = name;
		text[1] = gtktext;

		row = gtk_clist_append (GTK_CLIST (list), (gchar **)text);
		gtk_clist_set_row_data (GTK_CLIST (list), row, GINT_TO_POINTER (g_quark_from_string (name)));
	}

	gtk_clist_thaw (GTK_CLIST (list));

	// send a "changed" signal so widget owners will know I've updated
	g_signal_emit_by_name(G_OBJECT (list), "row-value-changed", row );
}

