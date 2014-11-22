#ifndef __SP_XMLVIEW_ATTR_LIST_H__
#define __SP_XMLVIEW_ATTR_LIST_H__

/*
 * Specialization of GtkTreeView for editing XML node attributes
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2002 MenTaLguY
 *
 * Released under the GNU GPL; see COPYING for details
 */

#include <gtk/gtk.h>

#define SP_TYPE_XMLVIEW_ATTR_LIST (sp_xmlview_attr_list_get_type ())
#define SP_XMLVIEW_ATTR_LIST(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_XMLVIEW_ATTR_LIST, SPXMLViewAttrList))
#define SP_IS_XMLVIEW_ATTR_LIST(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_XMLVIEW_ATTR_LIST))
#define SP_XMLVIEW_ATTR_LIST_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), SP_TYPE_XMLVIEW_ATTR_LIST))

struct SPXMLViewAttrList
{
	GtkTreeView list;
	GtkListStore *store;

	Inkscape::XML::Node * repr;
};

struct SPXMLViewAttrListClass
{
	GtkTreeViewClass parent_class;

	void (* row_changed) (SPXMLViewAttrList *list, gint row);
};

GType sp_xmlview_attr_list_get_type (void);
GtkWidget * sp_xmlview_attr_list_new (Inkscape::XML::Node * repr);

#define SP_XMLVIEW_ATTR_LIST_GET_REPR(list) (SP_XMLVIEW_ATTR_LIST (list)->repr)

void sp_xmlview_attr_list_set_repr (SPXMLViewAttrList * list, Inkscape::XML::Node * repr);
void sp_xmlview_attr_list_select_row_by_key(SPXMLViewAttrList * list, const gchar *name);

/* Attribute list store columns */
enum {ATTR_COL_NAME=0, ATTR_COL_ATTR=1, ATTR_COL_VALUE=2, ATTR_N_COLS=3 };

#endif
