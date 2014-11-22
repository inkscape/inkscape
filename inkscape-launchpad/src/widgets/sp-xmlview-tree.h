/**
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2002 MenTaLguY
 *
 * Released under the GNU GPL; see COPYING for details
 */

#ifndef SEEN_SP_XMLVIEW_TREE_H
#define SEEN_SP_XMLVIEW_TREE_H

#include <gtk/gtk.h>
#include <glib.h>

/**
 * Specialization of GtkTreeView for the XML editor
 */

#define SP_TYPE_XMLVIEW_TREE (sp_xmlview_tree_get_type ())
#define SP_XMLVIEW_TREE(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), SP_TYPE_XMLVIEW_TREE, SPXMLViewTree))
#define SP_IS_XMLVIEW_TREE(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), SP_TYPE_XMLVIEW_TREE))
#define SP_XMLVIEW_TREE_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), SP_TYPE_XMLVIEW_TREE))

struct SPXMLViewTree;
struct SPXMLViewTreeClass;

struct SPXMLViewTree
{
	GtkTreeView tree;
	GtkTreeStore *store;
	Inkscape::XML::Node * repr;
	gint blocked;
    gboolean dndactive;
};

struct SPXMLViewTreeClass
{
	GtkTreeViewClass parent_class;
};

GType sp_xmlview_tree_get_type (void);
GtkWidget * sp_xmlview_tree_new (Inkscape::XML::Node * repr, void * factory, void * data);

#define SP_XMLVIEW_TREE_REPR(tree) (SP_XMLVIEW_TREE (tree)->repr)

void sp_xmlview_tree_set_repr (SPXMLViewTree * tree, Inkscape::XML::Node * repr);

Inkscape::XML::Node * sp_xmlview_tree_node_get_repr (GtkTreeModel *model, GtkTreeIter * node);
gboolean sp_xmlview_tree_get_repr_node (SPXMLViewTree * tree, Inkscape::XML::Node * repr, GtkTreeIter *node);


#endif // !SEEN_SP_XMLVIEW_TREE_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :
