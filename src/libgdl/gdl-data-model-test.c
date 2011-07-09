#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gdl-i18n.h"

#include "gdl-data-model-test.h"
#include "gdl-data-model.h"

#include <string.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtkcellrenderertoggle.h>

GObjectClass *parent_class;

typedef struct _DataItem {
	char *name;
	char *value;
	char *path;
	struct _DataItem *children;
} DataItem;

DataItem data1[] = { 
	{ "foo1", "foo1", "0:0", NULL}, 
	{ "bar1", "bar1", "0:1", NULL }, 
	{ "baz1", "baz1", "0:2", NULL },
	{ NULL, NULL, NULL, NULL }
 
};
DataItem data2[] = { 
	{ "foo2", "foo2", "1:0", NULL }, 
	{ "bar2", "bar2", "1:1", NULL }, 
	{ "baz2", "baz2", "1:2", NULL },
	{ NULL, NULL, NULL, NULL }
 
};

DataItem data5[] = { 
	{ "1", "1", "2:2:1:0", NULL }, 
	{ "2", "2", "2:2:1:1",  NULL },
	{ "3", "3", "2:2:1:2",  NULL },
	{ "4", "4", "2:2:1:3",  NULL },
	{ "5", "5", "2:2:1:4",  NULL },
	{ "6", "6", "2:2:1:5", NULL }, 
	{ NULL, NULL, NULL, NULL }
 
};
DataItem data4[] = { 
	{ "foo4", "foo4", "2:2:0", NULL }, 
	{ "bar4", "[...]", "2:2:1", data5 }, 
	{ "baz4", "baz4", "2:2:2",  NULL },
	{ NULL, NULL, NULL, NULL }
 
};
DataItem data3[] = { 
	{ "foo foo", "foo3", "2:0", NULL }, 
	{ "bar3", "1", "2:1", NULL }, 
	{ "baz3", "{...}", "2:2",  data4 },
	{ NULL, NULL, NULL, NULL }
 
};

DataItem root[] = { 
	{ "test-data", "value1", "0", NULL } ,
	{ "test-data2", "value2", "1", NULL } ,
	{ "test-data3", "{...}", "2", data3 } ,
	{ NULL, NULL, NULL }
};

static gboolean
get_iter (GdlDataModel *dm, GdlDataIter *iter, GtkTreePath *path)
{
	int *i = gtk_tree_path_get_indices (path);
	int n = gtk_tree_path_get_depth (path);
	DataItem *item;

	g_assert (i);
	item = &root[*i++];

	while (--n) {
		item = &item->children[*i++];
	}

	iter->data1 = item;

	return TRUE;
}

static GtkTreePath *
get_path (GdlDataModel *dm, GdlDataIter *iter)
{
	DataItem *item = iter->data1;
	return gtk_tree_path_new_from_string (item->path);
}

static void
get_name (GdlDataModel *dm, GdlDataIter *iter, char **name)
{
	DataItem *item = iter->data1;
	*name = item->name;
}

static void
get_value (GdlDataModel *dm, GdlDataIter *iter, GValue *value)
{
	DataItem *item = iter->data1;
	if (strcmp (item->name, "bar3")) {
		g_value_init (value, G_TYPE_STRING);
		g_value_set_string (value, item->value);
	} else {
		g_value_init (value, G_TYPE_BOOLEAN);
		g_value_set_boolean (value, !strcmp (item->value, "1"));
	}
}

static void
get_renderer (GdlDataModel *dm, GdlDataIter *iter, 
	      GtkCellRenderer **renderer, char **field,
	      gboolean *is_editable)
{
	DataItem *item = iter->data1;
	if (!strcmp (item->name, "bar3")) {
		*renderer = g_object_new (gtk_cell_renderer_toggle_get_type (),
					  "activatable", TRUE, NULL);
		*field = "active";
	} else {
		*renderer = g_object_new (gtk_cell_renderer_text_get_type (),
					  "editable", TRUE, NULL);
		*field = "text";
	}
	*is_editable = (item->children == NULL);
}

static gboolean
iter_next (GdlDataModel *dm, GdlDataIter *iter)
{
	DataItem *item = iter->data1;
	item++;
	if (item->name) {
		iter->data1 = item;
		return TRUE;
	} else {
		return FALSE;
	}
}

static gboolean
iter_children (GdlDataModel *dm, GdlDataIter *iter, GdlDataIter *parent)
{
	DataItem *item = parent->data1;

	item = &item->children[0];
	if (item) {
		iter->data1 = item;
		return TRUE;
	} else {
		return FALSE;
	}
}

static gboolean
iter_has_child (GdlDataModel *dm, GdlDataIter *iter)
{
	DataItem *item = iter->data1;
	if (item->children) {
		return TRUE;
	} else {
		return FALSE;
	}
}


static void 
gdl_data_model_test_instance_init (GdlDataModelTest *model)
{
}

static void
gdl_data_model_test_finalize (GObject *object)
{
	(*parent_class->finalize) (object);
}

static void 
gdl_data_model_test_class_init (GdlDataModelTestClass *klass)
{
	GObjectClass *object_class;
	parent_class = g_type_class_peek_parent (klass);
	object_class = (GObjectClass *)klass;
	object_class->finalize = gdl_data_model_test_finalize;
}

static void
gdl_data_model_test_data_model_init (GdlDataModelIface *iface)
{
	iface->get_iter  = get_iter;
	iface->get_path  = get_path;
	iface->get_name  = get_name;
	iface->get_value = get_value;
	iface->get_renderer = get_renderer;
	iface->iter_next = iter_next;
	iface->iter_children = iter_children;
	iface->iter_has_child = iter_has_child;
}

GType 
gdl_data_model_test_get_type (void)
{
	static GType type = 0;
	
	if (!type) {
		static const GTypeInfo data_model_test_info = {
			sizeof (GdlDataModelTestClass),
			NULL, NULL, 
			(GClassInitFunc) gdl_data_model_test_class_init,
			NULL, NULL,
			sizeof (GdlDataModelTest), 0, 
			(GInstanceInitFunc) gdl_data_model_test_instance_init
		};
		
		static const GInterfaceInfo data_model_info = {
			(GInterfaceInitFunc) gdl_data_model_test_data_model_init,
			NULL, NULL
		};
		
		type = g_type_register_static (G_TYPE_OBJECT, 
					       "GdlDataModelTest",
					       &data_model_test_info, 0);
		g_type_add_interface_static (type,
					     GDL_TYPE_DATA_MODEL,
					     &data_model_info);
	}
	return type;
}
		
GdlDataModelTest *
gdl_data_model_test_new (void)
{
	return GDL_DATA_MODEL_TEST (g_object_new (gdl_data_model_test_get_type (), NULL));
}
