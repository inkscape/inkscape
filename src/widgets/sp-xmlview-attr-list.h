#ifndef __SP_XMLVIEW_ATTR_LIST_H__
#define __SP_XMLVIEW_ATTR_LIST_H__

/*
 * Specialization of GtkCList for editing XML node attributes
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2002 MenTaLguY
 *
 * Released under the GNU GPL; see COPYING for details
 */

#include <stdio.h>
#include <gtk/gtkctree.h>
#include <gtk/gtk.h>
#include "../xml/repr.h"




#define SP_TYPE_XMLVIEW_ATTR_LIST (sp_xmlview_attr_list_get_type ())
#define SP_XMLVIEW_ATTR_LIST(o) (GTK_CHECK_CAST ((o), SP_TYPE_XMLVIEW_ATTR_LIST, SPXMLViewAttrList))
#define SP_IS_XMLVIEW_ATTR_LIST(o) (GTK_CHECK_TYPE ((o), SP_TYPE_XMLVIEW_ATTR_LIST))
#define SP_XMLVIEW_ATTR_LIST_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_XMLVIEW_ATTR_LIST))

struct SPXMLViewAttrList
{
	GtkCList list;

	Inkscape::XML::Node * repr;
};

struct SPXMLViewAttrListClass
{
	GtkCListClass parent_class;

	void (* row_changed) (SPXMLViewAttrList *list, gint row);
};

GtkType sp_xmlview_attr_list_get_type (void);
GtkWidget * sp_xmlview_attr_list_new (Inkscape::XML::Node * repr);

#define SP_XMLVIEW_ATTR_LIST_GET_REPR(list) (SP_XMLVIEW_ATTR_LIST (list)->repr)

#define sp_xmlview_attr_list_get_row_key(list, row) (GPOINTER_TO_INT (gtk_clist_get_row_data ((list), (row))))
#define sp_xmlview_attr_list_find_row_from_key(list, key) (gtk_clist_find_row_from_data ((list), GINT_TO_POINTER ((key))))

void sp_xmlview_attr_list_set_repr (SPXMLViewAttrList * list, Inkscape::XML::Node * repr);



#endif
