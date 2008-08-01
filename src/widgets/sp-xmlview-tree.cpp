#define __SP_XMLVIEW_TREE_C__

/*
 * Specialization of GtkCTree for the XML tree view
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2002 MenTaLguY
 *
 * Released under the GNU GPL; see COPYING for details
 */

#include <cstring>
#include <string>

#include "../xml/node-event-vector.h"
#include "sp-xmlview-tree.h"

struct NodeData {
	SPXMLViewTree * tree;
	GtkCTreeNode * node;
	Inkscape::XML::Node * repr;
};

#define NODE_DATA(node) ((NodeData *)(GTK_CTREE_ROW ((node))->row.data))

static void sp_xmlview_tree_class_init (SPXMLViewTreeClass * klass);
static void sp_xmlview_tree_init (SPXMLViewTree * tree);
static void sp_xmlview_tree_destroy (GtkObject * object);

static NodeData * node_data_new (SPXMLViewTree * tree, GtkCTreeNode * node, Inkscape::XML::Node * repr);
static void node_data_free (gpointer data);

static GtkCTreeNode * add_node (SPXMLViewTree * tree, GtkCTreeNode * parent, GtkCTreeNode * before, Inkscape::XML::Node * repr);

static void element_child_added (Inkscape::XML::Node * repr, Inkscape::XML::Node * child, Inkscape::XML::Node * ref, gpointer data);
static void element_attr_changed (Inkscape::XML::Node * repr, const gchar * key, const gchar * old_value, const gchar * new_value, bool is_interactive, gpointer data);
static void element_child_removed (Inkscape::XML::Node * repr, Inkscape::XML::Node * child, Inkscape::XML::Node * ref, gpointer data);
static void element_order_changed (Inkscape::XML::Node * repr, Inkscape::XML::Node * child, Inkscape::XML::Node * oldref, Inkscape::XML::Node * newref, gpointer data);

static void text_content_changed (Inkscape::XML::Node * repr, const gchar * old_content, const gchar * new_content, gpointer data);
static void comment_content_changed (Inkscape::XML::Node * repr, const gchar * old_content, const gchar * new_content, gpointer data);
static void pi_content_changed (Inkscape::XML::Node * repr, const gchar * old_content, const gchar * new_content, gpointer data);

static void tree_move (GtkCTree * tree, GtkCTreeNode * node, GtkCTreeNode * new_parent, GtkCTreeNode * new_sibling);

static gboolean check_drag (GtkCTree * tree, GtkCTreeNode * node, GtkCTreeNode * new_parent, GtkCTreeNode * new_sibling);

static GtkCTreeNode * ref_to_sibling (GtkCTreeNode * parent, Inkscape::XML::Node * ref);
static GtkCTreeNode * repr_to_child (GtkCTreeNode * parent, Inkscape::XML::Node * repr);
static Inkscape::XML::Node * sibling_to_ref (GtkCTreeNode * parent, GtkCTreeNode * sibling);

static gint match_node_data_by_repr(gconstpointer data_p, gconstpointer repr);

static const Inkscape::XML::NodeEventVector element_repr_events = {
        element_child_added,
        element_child_removed,
        element_attr_changed,
        NULL, /* content_changed */
        element_order_changed
};

static const Inkscape::XML::NodeEventVector text_repr_events = {
        NULL, /* child_added */
        NULL, /* child_removed */
        NULL, /* attr_changed */
        text_content_changed,
        NULL  /* order_changed */
};

static const Inkscape::XML::NodeEventVector comment_repr_events = {
        NULL, /* child_added */
        NULL, /* child_removed */
        NULL, /* attr_changed */
        comment_content_changed,
        NULL  /* order_changed */
};

static const Inkscape::XML::NodeEventVector pi_repr_events = {
        NULL, /* child_added */
        NULL, /* child_removed */
        NULL, /* attr_changed */
        pi_content_changed,
        NULL  /* order_changed */
};

static GtkCTreeClass * parent_class = NULL;

GtkWidget *
sp_xmlview_tree_new (Inkscape::XML::Node * repr, void * /*factory*/, void * /*data*/)
{
	SPXMLViewTree * tree;

	tree = (SPXMLViewTree*)g_object_new (SP_TYPE_XMLVIEW_TREE, "n_columns", 1, "tree_column", 0, NULL);

	gtk_clist_column_titles_hide (GTK_CLIST (tree));
	gtk_ctree_set_line_style (GTK_CTREE (tree), GTK_CTREE_LINES_NONE);
	gtk_ctree_set_expander_style (GTK_CTREE (tree), GTK_CTREE_EXPANDER_TRIANGLE);
	gtk_clist_set_column_auto_resize (GTK_CLIST (tree), 0, TRUE);
	gtk_clist_set_reorderable (GTK_CLIST (tree), TRUE);
	gtk_ctree_set_drag_compare_func (GTK_CTREE (tree), check_drag);

	sp_xmlview_tree_set_repr (tree, repr);

	return (GtkWidget *) tree;
}

void
sp_xmlview_tree_set_repr (SPXMLViewTree * tree, Inkscape::XML::Node * repr)
{
	if ( tree->repr == repr ) return;
	gtk_clist_freeze (GTK_CLIST (tree));
	if (tree->repr) {
		gtk_clist_clear (GTK_CLIST (tree));
		Inkscape::GC::release(tree->repr);
	}
	tree->repr = repr;
	if (repr) {
		GtkCTreeNode * node;
		Inkscape::GC::anchor(repr);
		node = add_node (tree, NULL, NULL, repr);
		gtk_ctree_expand (GTK_CTREE (tree), node);
	}
	gtk_clist_thaw (GTK_CLIST (tree));
}

GtkType
sp_xmlview_tree_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		static const GtkTypeInfo info = {
			"SPXMLViewTree",
			sizeof (SPXMLViewTree),
			sizeof (SPXMLViewTreeClass),
			(GtkClassInitFunc) sp_xmlview_tree_class_init,
			(GtkObjectInitFunc) sp_xmlview_tree_init,
			NULL, NULL, NULL
		};
		type = gtk_type_unique (GTK_TYPE_CTREE, &info);
	}

	return type;
}

void
sp_xmlview_tree_class_init (SPXMLViewTreeClass * klass)
{
	GtkObjectClass * object_class;

	object_class = (GtkObjectClass *) klass;
	parent_class = (GtkCTreeClass *) gtk_type_class (GTK_TYPE_CTREE);

	GTK_CTREE_CLASS (object_class)->tree_move = tree_move;

	object_class->destroy = sp_xmlview_tree_destroy;
}

void
sp_xmlview_tree_init (SPXMLViewTree * tree)
{
	tree->repr = NULL;
	tree->blocked = 0;
}

void
sp_xmlview_tree_destroy (GtkObject * object)
{
	SPXMLViewTree * tree;

	tree = SP_XMLVIEW_TREE (object);

	sp_xmlview_tree_set_repr (tree, NULL);

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

GtkCTreeNode *
add_node (SPXMLViewTree * tree, GtkCTreeNode * parent, GtkCTreeNode * before, Inkscape::XML::Node * repr)
{
	NodeData * data;
	GtkCTreeNode * node;
	const Inkscape::XML::NodeEventVector * vec;
	static const gchar *default_text[] = { "???" };

	g_assert (tree != NULL);
	g_assert (repr != NULL);

	node = gtk_ctree_insert_node (GTK_CTREE (tree), parent, before, (gchar **)default_text, 2, NULL, NULL, NULL, NULL, ( repr->type() != Inkscape::XML::ELEMENT_NODE ), FALSE);
	g_assert (node != NULL);

	data = node_data_new (tree, node, repr);
	g_assert (data != NULL);

	gtk_ctree_node_set_row_data_full (GTK_CTREE (tree), data->node, data, node_data_free);

	if ( repr->type() == Inkscape::XML::TEXT_NODE ) {
		vec = &text_repr_events;
	} else if ( repr->type() == Inkscape::XML::COMMENT_NODE ) {
		vec = &comment_repr_events;
	} else if ( repr->type() == Inkscape::XML::PI_NODE ) {
		vec = &pi_repr_events;
	} else if ( repr->type() == Inkscape::XML::ELEMENT_NODE ) {
		vec = &element_repr_events;
	} else {
		vec = NULL;
	}

	if (vec) {
		gtk_clist_freeze (GTK_CLIST (tree));
		/* cheat a little to get the id upated properly */
		if (repr->type() == Inkscape::XML::ELEMENT_NODE) {
			element_attr_changed (repr, "id", NULL, NULL, false, data);
		}
		sp_repr_add_listener (repr, vec, data);
		sp_repr_synthesize_events (repr, vec, data);
		gtk_clist_thaw (GTK_CLIST (tree));
	}

	return node;
}

NodeData *
node_data_new (SPXMLViewTree * tree, GtkCTreeNode * node, Inkscape::XML::Node * repr)
{
	NodeData * data;
	data = g_new (NodeData, 1);
	data->tree = tree;
	data->node = node;
	data->repr = repr;
	Inkscape::GC::anchor(repr);
	return data;
}

void
node_data_free (gpointer ptr) {
	NodeData * data;
	data = (NodeData *) ptr;
	sp_repr_remove_listener_by_data (data->repr, data);
	g_assert (data->repr != NULL);
	Inkscape::GC::release(data->repr);
	g_free (data);
}

void
element_child_added (Inkscape::XML::Node * /*repr*/, Inkscape::XML::Node * child, Inkscape::XML::Node * ref, gpointer ptr)
{
	NodeData * data;
	GtkCTreeNode * before;

	data = (NodeData *) ptr;

	if (data->tree->blocked) return;

	before = ref_to_sibling (data->node, ref);

	add_node (data->tree, data->node, before, child);
}

void
element_attr_changed (Inkscape::XML::Node * repr, const gchar * key, const gchar * /*old_value*/, const gchar * new_value, bool /*is_interactive*/, gpointer ptr)
{
	NodeData * data;
	gchar *label;

	data = (NodeData *) ptr;

	if (data->tree->blocked) return;

	if (strcmp (key, "id")) return;

	if (new_value) {
		label = g_strdup_printf ("<%s id=\"%s\">", repr->name(), new_value);
	} else {
		label = g_strdup_printf ("<%s>", repr->name());
	}
	gtk_ctree_node_set_text (GTK_CTREE (data->tree), data->node, 0, label);
	g_free (label);
}

void
element_child_removed (Inkscape::XML::Node * /*repr*/, Inkscape::XML::Node * child, Inkscape::XML::Node * /*ref*/, gpointer ptr)
{
	NodeData * data;

	data = (NodeData *) ptr;

	if (data->tree->blocked) return;

	gtk_ctree_remove_node (GTK_CTREE (data->tree), repr_to_child (data->node, child));
}

void
element_order_changed (Inkscape::XML::Node * /*repr*/, Inkscape::XML::Node * child, Inkscape::XML::Node * /*oldref*/, Inkscape::XML::Node * newref, gpointer ptr)
{
	NodeData * data;
	GtkCTreeNode * before, * node;

	data = (NodeData *) ptr;

	if (data->tree->blocked) return;

	before = ref_to_sibling (data->node, newref);
	node = repr_to_child (data->node, child);

	if ( before == node ) before = GTK_CTREE_ROW (before)->sibling;

	parent_class->tree_move (GTK_CTREE (data->tree), node, data->node, before);
}

void
text_content_changed (Inkscape::XML::Node * /*repr*/, const gchar * /*old_content*/, const gchar * new_content, gpointer ptr)
{
	NodeData *data;
	gchar *label;

	data = (NodeData *) ptr;

	if (data->tree->blocked) return;

	label = g_strdup_printf ("\"%s\"", new_content);
	gtk_ctree_node_set_text (GTK_CTREE (data->tree), data->node, 0, label);
	g_free (label);
}

void
comment_content_changed (Inkscape::XML::Node */*repr*/, const gchar * /*old_content*/, const gchar *new_content, gpointer ptr)
{
	NodeData *data;
	gchar *label;

	data = (NodeData *) ptr;

	if (data->tree->blocked) return;

	label = g_strdup_printf ("<!--%s-->", new_content);
	gtk_ctree_node_set_text (GTK_CTREE (data->tree), data->node, 0, label);
	g_free (label);
}

void
pi_content_changed(Inkscape::XML::Node *repr, const gchar * /*old_content*/, const gchar *new_content, gpointer ptr)
{
	NodeData *data;
	gchar *label;

	data = (NodeData *) ptr;

	if (data->tree->blocked) return;

	label = g_strdup_printf ("<?%s %s?>", repr->name(), new_content);
	gtk_ctree_node_set_text (GTK_CTREE (data->tree), data->node, 0, label);
	g_free (label);
}
void
tree_move (GtkCTree * tree, GtkCTreeNode * node, GtkCTreeNode * new_parent, GtkCTreeNode * new_sibling)
{
	GtkCTreeNode * old_parent;
	Inkscape::XML::Node * ref;

	old_parent = GTK_CTREE_ROW (node)->parent;
	if ( !old_parent || !new_parent ) return;

	ref = sibling_to_ref (new_parent, new_sibling);

	gtk_clist_freeze (GTK_CLIST (tree));

	SP_XMLVIEW_TREE (tree)->blocked++;
	if (new_parent == old_parent) {
		NODE_DATA (old_parent)->repr->changeOrder(NODE_DATA (node)->repr, ref);
	} else {
		NODE_DATA (old_parent)->repr->removeChild(NODE_DATA (node)->repr);
		NODE_DATA (new_parent)->repr->addChild(NODE_DATA (node)->repr, ref);
	}
	SP_XMLVIEW_TREE (tree)->blocked--;

	parent_class->tree_move (tree, node, new_parent, new_sibling);

	gtk_clist_thaw (GTK_CLIST (tree));
}

GtkCTreeNode *
ref_to_sibling (GtkCTreeNode * parent, Inkscape::XML::Node * ref)
{
	if (ref) {
		GtkCTreeNode * before;
		before = repr_to_child (parent, ref);
		g_assert (before != NULL);
		before = GTK_CTREE_ROW (before)->sibling;
		return before;
	} else {
		return GTK_CTREE_ROW (parent)->children;
	}
}

GtkCTreeNode *
repr_to_child (GtkCTreeNode * parent, Inkscape::XML::Node * repr)
{
	GtkCTreeNode * child;
	child = GTK_CTREE_ROW (parent)->children;
	while ( child && NODE_DATA (child)->repr != repr ) {
		child = GTK_CTREE_ROW (child)->sibling;
	}
	return child;
}

Inkscape::XML::Node *
sibling_to_ref (GtkCTreeNode * parent, GtkCTreeNode * sibling)
{
	GtkCTreeNode * child;
	child = GTK_CTREE_ROW (parent)->children;
	if ( child == sibling ) return NULL;
	while ( child && GTK_CTREE_ROW (child)->sibling != sibling ) {
		child = GTK_CTREE_ROW (child)->sibling;
	}
	return NODE_DATA (child)->repr;
}

gboolean
check_drag (GtkCTree * /*tree*/, GtkCTreeNode * node, GtkCTreeNode * new_parent, GtkCTreeNode * /*new_sibling*/)
{
	GtkCTreeNode * old_parent;

	old_parent = GTK_CTREE_ROW (node)->parent;

	if (!old_parent || !new_parent) return FALSE;
	if (NODE_DATA (new_parent)->repr->type() != Inkscape::XML::ELEMENT_NODE) return FALSE;

	/* fixme: we need add_child/remove_child/etc repr events without side-effects, so we can check here and give better visual feedback */

	return TRUE;
}

Inkscape::XML::Node *
sp_xmlview_tree_node_get_repr (SPXMLViewTree * /*tree*/, GtkCTreeNode * node)
{
	return NODE_DATA (node)->repr;
}

GtkCTreeNode *
sp_xmlview_tree_get_repr_node (SPXMLViewTree * tree, Inkscape::XML::Node * repr)
{
	return gtk_ctree_find_by_row_data_custom (GTK_CTREE (tree), NULL, repr, match_node_data_by_repr);
}

gint
match_node_data_by_repr(gconstpointer data_p, gconstpointer repr)
{
	return ((const NodeData *)data_p)->repr != (const Inkscape::XML::Node *)repr;
}
