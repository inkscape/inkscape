/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * This file is part of the GNOME Devtools Libraries.
 *
 * Copyright (C) 2002 Gustavo Giráldez <gustavo.giraldez@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gdl-i18n.h"
#include <string.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <gtk/gtk.h>

#include "gdl-dock-layout.h"
#include "gdl-tools.h"
#include "gdl-dock-placeholder.h"


/* ----- Private variables ----- */

enum {
    PROP_0,
    PROP_MASTER,
    PROP_DIRTY
};

#define ROOT_ELEMENT         "dock-layout"
#define DEFAULT_LAYOUT       "__default__"
#define LAYOUT_ELEMENT_NAME  "layout"
#define NAME_ATTRIBUTE_NAME  "name"

#define LAYOUT_UI_FILE    "layout.ui"

enum {
    COLUMN_NAME,
    COLUMN_SHOW,
    COLUMN_LOCKED,
    COLUMN_ITEM
};

#define COLUMN_EDITABLE COLUMN_SHOW

struct _GdlDockLayoutPrivate {
    xmlDocPtr         doc;

    /* layout list models */
    GtkListStore     *items_model;
    GtkListStore     *layouts_model;

    /* idle control */
    gboolean          idle_save_pending;
};

typedef struct _GdlDockLayoutUIData GdlDockLayoutUIData;

struct _GdlDockLayoutUIData {
    GdlDockLayout    *layout;
    GtkWidget        *locked_check;
    GtkTreeSelection *selection;
};


/* ----- Private prototypes ----- */

static void     gdl_dock_layout_class_init      (GdlDockLayoutClass *klass);

static void     gdl_dock_layout_instance_init   (GdlDockLayout      *layout);

static void     gdl_dock_layout_set_property    (GObject            *object,
                                                 guint               prop_id,
                                                 const GValue       *value,
                                                 GParamSpec         *pspec);

static void     gdl_dock_layout_get_property    (GObject            *object,
                                                 guint               prop_id,
                                                 GValue             *value,
                                                 GParamSpec         *pspec);

static void     gdl_dock_layout_dispose         (GObject            *object);

static void     gdl_dock_layout_build_doc       (GdlDockLayout      *layout);

static xmlNodePtr gdl_dock_layout_find_layout   (GdlDockLayout      *layout, 
                                                 const gchar        *name);

static void     gdl_dock_layout_build_models    (GdlDockLayout      *layout);


/* ----- Private implementation ----- */

GDL_CLASS_BOILERPLATE (GdlDockLayout, gdl_dock_layout, GObject, G_TYPE_OBJECT);

static void
gdl_dock_layout_class_init (GdlDockLayoutClass *klass)
{
    GObjectClass *g_object_class = (GObjectClass *) klass;

    g_object_class->set_property = gdl_dock_layout_set_property;
    g_object_class->get_property = gdl_dock_layout_get_property;
    g_object_class->dispose = gdl_dock_layout_dispose;

    g_object_class_install_property (
        g_object_class, PROP_MASTER,
        g_param_spec_object ("master", _("Master"),
                             _("GdlDockMaster object which the layout object "
                               "is attached to"),
                             GDL_TYPE_DOCK_MASTER, 
                             G_PARAM_READWRITE));

    g_object_class_install_property (
        g_object_class, PROP_DIRTY,
        g_param_spec_boolean ("dirty", _("Dirty"),
                              _("True if the layouts have changed and need to be "
                                "saved to a file"),
                              FALSE,
                              G_PARAM_READABLE));
}

static void
gdl_dock_layout_instance_init (GdlDockLayout *layout)
{
    layout->master = NULL;
    layout->dirty = FALSE;
    layout->_priv = g_new0 (GdlDockLayoutPrivate, 1);
    layout->_priv->idle_save_pending = FALSE;

    gdl_dock_layout_build_models (layout);
}

static void
gdl_dock_layout_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
    GdlDockLayout *layout = GDL_DOCK_LAYOUT (object);

    switch (prop_id) {
        case PROP_MASTER:
            gdl_dock_layout_attach (layout, g_value_get_object (value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    };
}

static void
gdl_dock_layout_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
    GdlDockLayout *layout = GDL_DOCK_LAYOUT (object);

    switch (prop_id) {
        case PROP_MASTER:
            g_value_set_object (value, layout->master);
            break;
        case PROP_DIRTY:
            g_value_set_boolean (value, layout->dirty);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    };
}

static void
gdl_dock_layout_dispose (GObject *object)
{
    GdlDockLayout *layout;

    g_return_if_fail (object != NULL);
    g_return_if_fail (GDL_IS_DOCK_LAYOUT (object));

    layout = GDL_DOCK_LAYOUT (object);
    
    if (layout->master)
        gdl_dock_layout_attach (layout, NULL);

    if (layout->_priv) {
        if (layout->_priv->idle_save_pending) {
            layout->_priv->idle_save_pending = FALSE;
            g_idle_remove_by_data (layout);
        }
        
        if (layout->_priv->doc) {
            xmlFreeDoc (layout->_priv->doc);
            layout->_priv->doc = NULL;
        }

        if (layout->_priv->items_model) {
            g_object_unref (layout->_priv->items_model);
            g_object_unref (layout->_priv->layouts_model);
            layout->_priv->items_model = NULL;
            layout->_priv->layouts_model = NULL;
        }
        
        xmlFreeDoc(layout->_priv->doc);
        g_free (layout->_priv);
        layout->_priv = NULL;
    }
}

static void
gdl_dock_layout_build_doc (GdlDockLayout *layout)
{
    g_return_if_fail (layout->_priv->doc == NULL);

    layout->_priv->doc = xmlNewDoc (BAD_CAST "1.0");
    layout->_priv->doc->children = xmlNewDocNode (layout->_priv->doc, NULL, 
                                                  BAD_CAST ROOT_ELEMENT, NULL);
}

static xmlNodePtr
gdl_dock_layout_find_layout (GdlDockLayout *layout, 
                             const gchar   *name)
{
    xmlNodePtr node;
    gboolean   found = FALSE;

    g_return_val_if_fail (layout != NULL, NULL);
    
    if (!layout->_priv->doc)
        return NULL;

    /* get document root */
    node = layout->_priv->doc->children;
    for (node = node->children; node; node = node->next) {
        xmlChar *layout_name;
        
        if (strcmp ((char*)node->name, LAYOUT_ELEMENT_NAME))
            /* skip non-layout element */
            continue;

        /* we want the first layout */
        if (!name)
            break;

        layout_name = xmlGetProp (node, BAD_CAST NAME_ATTRIBUTE_NAME);
        if (!strcmp (name, (char*)layout_name))
            found = TRUE;
        xmlFree (layout_name);

        if (found)
            break;
    };
    return node;
}

static void
gdl_dock_layout_build_models (GdlDockLayout *layout)
{
    if (!layout->_priv->items_model) {
        layout->_priv->items_model = gtk_list_store_new (4, 
                                                         G_TYPE_STRING, 
                                                         G_TYPE_BOOLEAN,
                                                         G_TYPE_BOOLEAN,
                                                         G_TYPE_POINTER);
        gtk_tree_sortable_set_sort_column_id (
            GTK_TREE_SORTABLE (layout->_priv->items_model), 
            COLUMN_NAME, GTK_SORT_ASCENDING);
    }

    if (!layout->_priv->layouts_model) {
        layout->_priv->layouts_model = gtk_list_store_new (2, G_TYPE_STRING,
                                                           G_TYPE_BOOLEAN);
        gtk_tree_sortable_set_sort_column_id (
            GTK_TREE_SORTABLE (layout->_priv->layouts_model),
            COLUMN_NAME, GTK_SORT_ASCENDING);
    }
}

static void
build_list (GdlDockObject *object, GList **list)
{
    /* add only items, not toplevels */
    if (GDL_IS_DOCK_ITEM (object))
        *list = g_list_prepend (*list, object);
}

static void
update_items_model (GdlDockLayout *layout)
{
    GList *items, *l;
    GtkTreeIter iter;
    GtkListStore *store;
    gchar *long_name;
    gboolean locked;
    
    g_return_if_fail (layout != NULL);
    g_return_if_fail (layout->_priv->items_model != NULL);

    if (!layout->master)
        return;
    
    /* build items list */
    items = NULL;
    gdl_dock_master_foreach (layout->master, (GFunc) build_list, &items);

    /* walk the current model */
    store = layout->_priv->items_model;
    
    /* update items model data after a layout load */
    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &iter)) {
        gboolean valid = TRUE;
        
        while (valid) {
            GdlDockItem *item;
            
            gtk_tree_model_get (GTK_TREE_MODEL (store), &iter,
                                COLUMN_ITEM, &item,
                                -1);
            if (item) {
                /* look for the object in the items list */
                for (l = items; l && l->data != item; l = l->next);

                if (l) {
                    /* found, update data */
                    g_object_get (item, 
                                  "long-name", &long_name, 
                                  "locked", &locked, 
                                  NULL);
                    gtk_list_store_set (store, &iter, 
                                        COLUMN_NAME, long_name,
                                        COLUMN_SHOW, GDL_DOCK_OBJECT_ATTACHED (item),
                                        COLUMN_LOCKED, locked,
                                        -1);
                    g_free (long_name);

                    /* remove the item from the linked list and keep on walking the model */
                    items = g_list_delete_link (items, l);
                    valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (store), &iter);

                } else {
                    /* not found, which means the item has been removed */
                    valid = gtk_list_store_remove (store, &iter);
                    
                }

            } else {
                /* not a valid row */
                valid = gtk_list_store_remove (store, &iter);
            }
        }
    }

    /* add any remaining objects */
    for (l = items; l; l = l->next) {
        GdlDockObject *object = l->data;
        
        g_object_get (object, 
                      "long-name", &long_name, 
                      "locked", &locked, 
                      NULL);
        gtk_list_store_append (store, &iter);
        gtk_list_store_set (store, &iter, 
                            COLUMN_ITEM, object,
                            COLUMN_NAME, long_name,
                            COLUMN_SHOW, GDL_DOCK_OBJECT_ATTACHED (object),
                            COLUMN_LOCKED, locked,
                            -1);
        g_free (long_name);
    }
    
    g_list_free (items);
}

static void
update_layouts_model (GdlDockLayout *layout)
{
    GList *items, *l;
    GtkTreeIter iter;
    
    g_return_if_fail (layout != NULL);
    g_return_if_fail (layout->_priv->layouts_model != NULL);

    /* build layouts list */
    gtk_list_store_clear (layout->_priv->layouts_model);
    items = gdl_dock_layout_get_layouts (layout, FALSE);
    for (l = items; l; l = l->next) {
        gtk_list_store_append (layout->_priv->layouts_model, &iter);
        gtk_list_store_set (layout->_priv->layouts_model, &iter,
                            COLUMN_NAME, l->data, COLUMN_EDITABLE, TRUE,
                            -1);
        g_free (l->data);
    };
    g_list_free (items);
}


/* ------- UI functions & callbacks ------ */

static void 
load_layout_cb (GtkWidget *w,
                gpointer   data)
{
    GdlDockLayoutUIData *ui_data = (GdlDockLayoutUIData *) data;

    GtkTreeModel  *model;
    GtkTreeIter    iter;
    GdlDockLayout *layout = ui_data->layout;
    gchar         *name;

    g_return_if_fail (layout != NULL);
    
    if (gtk_tree_selection_get_selected (ui_data->selection, &model, &iter)) {
        gtk_tree_model_get (model, &iter,
                            COLUMN_NAME, &name,
                            -1);
        gdl_dock_layout_load_layout (layout, name);
        g_free (name);
    }
}

static void
delete_layout_cb (GtkWidget *w, gpointer data)
{
    GdlDockLayoutUIData *ui_data = (GdlDockLayoutUIData *) data;

    GtkTreeModel  *model;
    GtkTreeIter    iter;
    GdlDockLayout *layout = ui_data->layout;
    gchar         *name;

    g_return_if_fail (layout != NULL);
    
    if (gtk_tree_selection_get_selected (ui_data->selection, &model, &iter)) {
        gtk_tree_model_get (model, &iter,
                            COLUMN_NAME, &name,
                            -1);
        gdl_dock_layout_delete_layout (layout, name);
        gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
        g_free (name);
    };
}

static void
show_toggled_cb (GtkCellRendererToggle *renderer,
                 gchar                 *path_str,
                 gpointer               data)
{
    GdlDockLayoutUIData *ui_data = (GdlDockLayoutUIData *) data;

    GdlDockLayout *layout = ui_data->layout;
    GtkTreeModel  *model;
    GtkTreeIter    iter;
    GtkTreePath   *path = gtk_tree_path_new_from_string (path_str);
    gboolean       value;
    GdlDockItem   *item;

    g_return_if_fail (layout != NULL);
    
    model = GTK_TREE_MODEL (layout->_priv->items_model);
    gtk_tree_model_get_iter (model, &iter, path);
    gtk_tree_model_get (model, &iter, 
                        COLUMN_SHOW, &value, 
                        COLUMN_ITEM, &item, 
                        -1);

    value = !value;
    if (value)
        gdl_dock_item_show_item (item);
    else
        gdl_dock_item_hide_item (item);

    gtk_tree_path_free (path);
}

static void
all_locked_toggled_cb (GtkWidget *widget,
                       gpointer   data)
{
    GdlDockLayoutUIData *ui_data = (GdlDockLayoutUIData *) data;
    GdlDockMaster       *master;
    gboolean             locked;
    
    g_return_if_fail (ui_data->layout != NULL);
    master = ui_data->layout->master;
    g_return_if_fail (master != NULL);

    locked = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    g_object_set (master, "locked", locked ? 1 : 0, NULL);
}

static void
layout_ui_destroyed (GtkWidget *widget,
                     gpointer   user_data)
{
    GdlDockLayoutUIData *ui_data;
    
    /* widget is the GtkContainer */
    ui_data = g_object_get_data (G_OBJECT (widget), "ui_data");
    if (ui_data) {
        if (ui_data->layout) {
            if (ui_data->layout->master)
                /* disconnet the notify handler */
                g_signal_handlers_disconnect_matched (ui_data->layout->master,
                                                      G_SIGNAL_MATCH_DATA,
                                                      0, 0, NULL, NULL,
                                                      ui_data);
            
            g_object_remove_weak_pointer (G_OBJECT (ui_data->layout),
                                          (gpointer *) &ui_data->layout);
            ui_data->layout = NULL;
        }
        g_object_set_data (G_OBJECT (widget), "ui_data", NULL);
        g_free (ui_data);
    }
}

static void
master_locked_notify_cb (GdlDockMaster *master,
                         GParamSpec    *pspec,
                         gpointer       user_data)
{
    GdlDockLayoutUIData *ui_data = (GdlDockLayoutUIData *) user_data;
    gint locked;

    g_object_get (master, "locked", &locked, NULL);
    if (locked == -1) {
        gtk_toggle_button_set_inconsistent (
            GTK_TOGGLE_BUTTON (ui_data->locked_check), TRUE);
    }
    else {
        gtk_toggle_button_set_inconsistent (
            GTK_TOGGLE_BUTTON (ui_data->locked_check), FALSE);
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (ui_data->locked_check), (locked == 1));
    }
}

static GtkBuilder *
load_interface ()
{
    GtkBuilder *gui;
    gchar    *gui_file;
    GError* error = NULL;

    /* load ui */
    gui_file = g_build_filename (GDL_UIDIR, LAYOUT_UI_FILE, NULL);
    gui = gtk_builder_new();
    gtk_builder_add_from_file (gui, gui_file, &error);
    g_free (gui_file);
    if (error) {
        g_warning (_("Could not load layout user interface file '%s'"), 
                   LAYOUT_UI_FILE);
        g_object_unref (gui);
        g_error_free (error);
        return NULL;
    };
    return gui;
}

static GtkWidget *
gdl_dock_layout_construct_items_ui (GdlDockLayout *layout)
{
    GtkBuilder          *gui;
    GtkWidget           *dialog;
    GtkWidget           *items_list;
    GtkCellRenderer     *renderer;
    GtkTreeViewColumn   *column;

    GdlDockLayoutUIData *ui_data;
    
    /* load the interface if it wasn't provided */
    gui = load_interface ();
    
    if (!gui)
        return NULL;
    
    /* get the container */
    dialog = GTK_WIDGET (gtk_builder_get_object (gui, "layout_dialog"));

    ui_data = g_new0 (GdlDockLayoutUIData, 1);
    ui_data->layout = layout;
    g_object_add_weak_pointer (G_OBJECT (layout),
                               (gpointer *) &ui_data->layout);
    g_object_set_data (G_OBJECT (dialog), "ui_data", ui_data);
    
    /* get ui widget references */
    ui_data->locked_check = GTK_WIDGET (gtk_builder_get_object (gui, "locked_check"));
    items_list = GTK_WIDGET (gtk_builder_get_object(gui, "items_list"));

    /* locked check connections */
    g_signal_connect (ui_data->locked_check, "toggled",
                      (GCallback) all_locked_toggled_cb, ui_data);
    if (layout->master) {
        g_signal_connect (layout->master, "notify::locked",
                          (GCallback) master_locked_notify_cb, ui_data);
        /* force update now */
        master_locked_notify_cb (layout->master, NULL, ui_data);
    }
    
    /* set models */
    gtk_tree_view_set_model (GTK_TREE_VIEW (items_list),
                             GTK_TREE_MODEL (layout->_priv->items_model));

    /* construct list views */
    renderer = gtk_cell_renderer_toggle_new ();
    g_signal_connect (renderer, "toggled", 
                      G_CALLBACK (show_toggled_cb), ui_data);
    column = gtk_tree_view_column_new_with_attributes (_("Visible"),
                                                       renderer,
                                                       "active", COLUMN_SHOW,
                                                       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (items_list), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Item"),
                                                       renderer,
                                                       "text", COLUMN_NAME,
                                                       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (items_list), column);

    /* connect signals */
    g_signal_connect (dialog, "destroy", (GCallback) layout_ui_destroyed, NULL);

    g_object_unref (gui);

    return dialog;
}

static void
cell_edited_cb (GtkCellRendererText *cell,
                const gchar         *path_string,
                const gchar         *new_text,
                gpointer             data)
{
    GdlDockLayoutUIData *ui_data = data;
    GtkTreeModel *model;
    GtkTreePath *path;
    GtkTreeIter iter;
    gchar *name;
    xmlNodePtr node;

    model = GTK_TREE_MODEL (ui_data->layout->_priv->layouts_model);
    path = gtk_tree_path_new_from_string (path_string);

    gtk_tree_model_get_iter (model, &iter, path);
    gtk_tree_model_get (model, &iter, COLUMN_NAME, &name, -1);

    node = gdl_dock_layout_find_layout (ui_data->layout, name);
    g_free (name);
    g_return_if_fail (node != NULL);

    xmlSetProp (node, BAD_CAST NAME_ATTRIBUTE_NAME, BAD_CAST new_text);
    gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_NAME, new_text,
                        COLUMN_EDITABLE, TRUE, -1);

    gdl_dock_layout_save_layout (ui_data->layout, new_text);

    gtk_tree_path_free (path);
}

static GtkWidget *
gdl_dock_layout_construct_layouts_ui (GdlDockLayout *layout)
{
    GtkBuilder          *gui;
    GtkWidget           *container;
    GtkWidget           *layouts_list;
    GtkCellRenderer     *renderer;
    GtkTreeViewColumn   *column;
    GtkWidget           *load_button;
    GtkWidget           *delete_button;

    GdlDockLayoutUIData *ui_data;
    
    /* load the interface if it wasn't provided */
    gui = load_interface ();
    
    if (!gui)
        return NULL;
    
    /* get the container */
    container = GTK_WIDGET (gtk_builder_get_object(gui, "layouts_vbox"));

    ui_data = g_new0 (GdlDockLayoutUIData, 1);
    ui_data->layout = layout;
    g_object_add_weak_pointer (G_OBJECT (layout),
                               (gpointer *) &ui_data->layout);
    g_object_set_data (G_OBJECT (container), "ui-data", ui_data);
    
    /* get ui widget references */
    layouts_list = GTK_WIDGET (gtk_builder_get_object(gui, "layouts_list"));

    /* set models */
    gtk_tree_view_set_model (GTK_TREE_VIEW (layouts_list),
                             GTK_TREE_MODEL (layout->_priv->layouts_model));

    /* construct list views */
    renderer = gtk_cell_renderer_text_new ();
    g_signal_connect (G_OBJECT (renderer), "edited",
                      G_CALLBACK (cell_edited_cb), ui_data);
    column = gtk_tree_view_column_new_with_attributes (_("Name"), renderer,
                                                       "text", COLUMN_NAME,
                                                       "editable", COLUMN_EDITABLE,
                                                       NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (layouts_list), column);

    ui_data->selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (layouts_list));

    /* connect signals */
    load_button = GTK_WIDGET (gtk_builder_get_object(gui, "load_button"));
    delete_button = GTK_WIDGET (gtk_builder_get_object(gui, "delete_button"));

    g_signal_connect (load_button, "clicked", (GCallback) load_layout_cb, ui_data);
    g_signal_connect (delete_button, "clicked", (GCallback) delete_layout_cb, ui_data);
    

    g_signal_connect (container, "destroy", (GCallback) layout_ui_destroyed, NULL);

    g_object_unref (gui);

    return container;
}

/* ----- Save & Load layout functions --------- */

#define GDL_DOCK_PARAM_CONSTRUCTION(p) \
    (((p)->flags & (G_PARAM_CONSTRUCT | G_PARAM_CONSTRUCT_ONLY)) != 0)

static GdlDockObject * 
gdl_dock_layout_setup_object (GdlDockMaster *master,
                              xmlNodePtr     node,
                              gint          *n_after_params,
                              GParameter   **after_params)
{
    GdlDockObject *object = NULL;
    GType          object_type;
    xmlChar       *object_name;
    GObjectClass  *object_class = NULL;

    GParamSpec   **props;
    guint          n_props, i;
    GParameter    *params = NULL;
    gint           n_params = 0;
    GValue         serialized = { 0, };
    
    object_name = xmlGetProp (node, BAD_CAST GDL_DOCK_NAME_PROPERTY);
    if (object_name && strlen ((char*)object_name) > 0) {
        /* the object must already be bound to the master */
        object = gdl_dock_master_get_object (master, (char*)object_name);

        xmlFree (object_name);
        object_type = object ? G_TYPE_FROM_INSTANCE (object) : G_TYPE_NONE;
    }
    else {
        /* the object should be automatic, so create it by
           retrieving the object type from the dock registry */
        object_type = gdl_dock_object_type_from_nick ((char*)node->name);
        if (object_type == G_TYPE_NONE) {
            g_warning (_("While loading layout: don't know how to create "
                         "a dock object whose nick is '%s'"), node->name);
        }
    }
    
    if (object_type == G_TYPE_NONE || !G_TYPE_IS_CLASSED (object_type))
        return NULL;

    object_class = g_type_class_ref (object_type);
    props = g_object_class_list_properties (object_class, &n_props);
        
    /* create parameter slots */
    /* extra parameter is the master */
    params = g_new0 (GParameter, n_props + 1);
    *after_params = g_new0 (GParameter, n_props);
    *n_after_params = 0;
    
    /* initialize value used for transformations */
    g_value_init (&serialized, GDL_TYPE_DOCK_PARAM);
    
    for (i = 0; i < n_props; i++) {
        xmlChar *xml_prop;

        /* process all exported properties, skip
           GDL_DOCK_NAME_PROPERTY, since named items should
           already by in the master */
        if (!(props [i]->flags & GDL_DOCK_PARAM_EXPORT) ||
            !strcmp (props [i]->name, GDL_DOCK_NAME_PROPERTY))
            continue;

        /* get the property from xml if there is one */
        xml_prop = xmlGetProp (node, BAD_CAST props [i]->name);
        if (xml_prop) {
            g_value_set_static_string (&serialized, (char*)xml_prop);
            
            if (!GDL_DOCK_PARAM_CONSTRUCTION (props [i]) &&
                (props [i]->flags & GDL_DOCK_PARAM_AFTER)) {
                (*after_params) [*n_after_params].name = props [i]->name;
                g_value_init (&((* after_params) [*n_after_params].value),
                              props [i]->value_type);
                g_value_transform (&serialized,
                                   &((* after_params) [*n_after_params].value));
                (*n_after_params)++;
            }
            else if (!object || (!GDL_DOCK_PARAM_CONSTRUCTION (props [i]) && object)) {
                params [n_params].name = props [i]->name;
                g_value_init (&(params [n_params].value), props [i]->value_type);
                g_value_transform (&serialized, &(params [n_params].value));
                n_params++;
            }
            xmlFree (xml_prop);
        }
    }
    g_value_unset (&serialized);
    g_free (props);

    if (!object) {
        params [n_params].name = GDL_DOCK_MASTER_PROPERTY;
        g_value_init (&params [n_params].value, GDL_TYPE_DOCK_MASTER);
        g_value_set_object (&params [n_params].value, master);
        n_params++;
        
        /* construct the object if we have to */
        /* set the master, so toplevels are created correctly and
           other objects are bound */
        object = g_object_newv (object_type, n_params, params);
    }
    else {
        /* set the parameters to the existing object */
        for (i = 0; i < n_params; i++)
            g_object_set_property (G_OBJECT (object),
                                   params [i].name,
                                   &params [i].value);
    }

    /* free the parameters (names are static/const strings) */
    for (i = 0; i < n_params; i++)
        g_value_unset (&params [i].value);
    g_free (params);

    /* finally unref object class */
    g_type_class_unref (object_class);

    return object;
}

static void
gdl_dock_layout_recursive_build (GdlDockMaster *master,
                                 xmlNodePtr     parent_node,
                                 GdlDockObject *parent)
{
    GdlDockObject *object;
    xmlNodePtr     node;
    
    g_return_if_fail (master != NULL && parent_node != NULL);

    /* if parent is NULL we should build toplevels */
    for (node = parent_node->children; node; node = node->next) {
        GParameter *after_params = NULL;
        gint        n_after_params = 0, i;

        object = gdl_dock_layout_setup_object (master, node,
                                               &n_after_params,
                                               &after_params);
        
        if (object) {
            gdl_dock_object_freeze (object);

            /* recurse here to catch placeholders */
            gdl_dock_layout_recursive_build (master, node, object);
            
            if (GDL_IS_DOCK_PLACEHOLDER (object))
                /* placeholders are later attached to the parent */
                gdl_dock_object_detach (object, FALSE);
            
            /* apply "after" parameters */
            for (i = 0; i < n_after_params; i++) {
                g_object_set_property (G_OBJECT (object),
                                       after_params [i].name,
                                       &after_params [i].value);
                /* unset and free the value */
                g_value_unset (&after_params [i].value);
            }
            g_free (after_params);
            
            /* add the object to the parent */
            if (parent) {
                if (GDL_IS_DOCK_PLACEHOLDER (object))
                    gdl_dock_placeholder_attach (GDL_DOCK_PLACEHOLDER (object),
                                                 parent);
                else if (gdl_dock_object_is_compound (parent)) {
                    gtk_container_add (GTK_CONTAINER (parent), GTK_WIDGET (object));
                    if (GTK_WIDGET_VISIBLE (parent))
                        gtk_widget_show (GTK_WIDGET (object));
                }
            }
            else {
                GdlDockObject *controller = gdl_dock_master_get_controller (master);
                if (controller != object && GTK_WIDGET_VISIBLE (controller))
                    gtk_widget_show (GTK_WIDGET (object));
            }
                
            /* call reduce just in case any child is missing */
            if (gdl_dock_object_is_compound (object))
                gdl_dock_object_reduce (object);

            gdl_dock_object_thaw (object);
        }
    }
}

static void
_gdl_dock_layout_foreach_detach (GdlDockObject *object)
{
    gdl_dock_object_detach (object, TRUE);
}

static void
gdl_dock_layout_foreach_toplevel_detach (GdlDockObject *object)
{
    gtk_container_foreach (GTK_CONTAINER (object),
                           (GtkCallback) _gdl_dock_layout_foreach_detach,
                           NULL);
}

static void
gdl_dock_layout_load (GdlDockMaster *master, xmlNodePtr node)
{
    g_return_if_fail (master != NULL && node != NULL);

    /* start by detaching all items from the toplevels */
    gdl_dock_master_foreach_toplevel (master, TRUE,
                                      (GFunc) gdl_dock_layout_foreach_toplevel_detach,
                                      NULL);
    
    gdl_dock_layout_recursive_build (master, node, NULL);
}

static void 
gdl_dock_layout_foreach_object_save (GdlDockObject *object,
                                     gpointer       user_data)
{
    struct {
        xmlNodePtr  where;
        GHashTable *placeholders;
    } *info = user_data, info_child;

    xmlNodePtr   node;
    guint        n_props, i;
    GParamSpec **props;
    GValue       attr = { 0, };
    
    g_return_if_fail (object != NULL && GDL_IS_DOCK_OBJECT (object));
    g_return_if_fail (info->where != NULL);
    
    node = xmlNewChild (info->where,
                        NULL,               /* ns */
                        BAD_CAST gdl_dock_object_nick_from_type (G_TYPE_FROM_INSTANCE (object)),
                        BAD_CAST NULL);     /* contents */

    /* get object exported attributes */
    props = g_object_class_list_properties (G_OBJECT_GET_CLASS (object),
                                            &n_props);
    g_value_init (&attr, GDL_TYPE_DOCK_PARAM);
    for (i = 0; i < n_props; i++) {
        GParamSpec *p = props [i];

        if (p->flags & GDL_DOCK_PARAM_EXPORT) {
            GValue v = { 0, };
            
            /* export this parameter */
            /* get the parameter value */
            g_value_init (&v, p->value_type);
            g_object_get_property (G_OBJECT (object),
                                   p->name,
                                   &v);

            /* only save the object "name" if it is set
               (i.e. don't save the empty string) */
            if (strcmp (p->name, GDL_DOCK_NAME_PROPERTY) ||
                g_value_get_string (&v)) {
                if (g_value_transform (&v, &attr))
                    xmlSetProp (node, BAD_CAST p->name, BAD_CAST g_value_get_string (&attr));
            }
            
            /* free the parameter value */
            g_value_unset (&v);
        }
    }
    g_value_unset (&attr);
    g_free (props);

    info_child = *info;
    info_child.where = node;

    /* save placeholders for the object */
    if (info->placeholders && !GDL_IS_DOCK_PLACEHOLDER (object)) {
        GList *lph = g_hash_table_lookup (info->placeholders, object);
        for (; lph; lph = lph->next)
            gdl_dock_layout_foreach_object_save (GDL_DOCK_OBJECT (lph->data),
                                                 (gpointer) &info_child);
    }
    
    /* recurse the object if appropiate */
    if (gdl_dock_object_is_compound (object)) {
        gtk_container_foreach (GTK_CONTAINER (object),
                               (GtkCallback) gdl_dock_layout_foreach_object_save,
                               (gpointer) &info_child);
    }
}

static void
add_placeholder (GdlDockObject *object,
                 GHashTable    *placeholders)
{
    if (GDL_IS_DOCK_PLACEHOLDER (object)) {
        GdlDockObject *host;
        GList *l;
        
        g_object_get (object, "host", &host, NULL);
        if (host) {
            l = g_hash_table_lookup (placeholders, host);
            /* add the current placeholder to the list of placeholders
               for that host */
            if (l)
                g_hash_table_steal (placeholders, host);
            
            l = g_list_prepend (l, object);
            g_hash_table_insert (placeholders, host, l);
            g_object_unref (host);
        }
    }
}

static void 
gdl_dock_layout_save (GdlDockMaster *master,
                      xmlNodePtr     where)
{
    struct {
        xmlNodePtr  where;
        GHashTable *placeholders;
    } info;
    
    GHashTable *placeholders;
    
    g_return_if_fail (master != NULL && where != NULL);

    /* build the placeholder's hash: the hash keeps lists of
     * placeholders associated to each object, so that we can save the
     * placeholders when we are saving the object (since placeholders
     * don't show up in the normal widget hierarchy) */
    placeholders = g_hash_table_new_full (g_direct_hash, g_direct_equal,
                                          NULL, (GDestroyNotify) g_list_free);
    gdl_dock_master_foreach (master, (GFunc) add_placeholder, placeholders);
    
    /* save the layout recursively */
    info.where = where;
    info.placeholders = placeholders;
    
    gdl_dock_master_foreach_toplevel (master, TRUE,
                                      (GFunc) gdl_dock_layout_foreach_object_save,
                                      (gpointer) &info);

    g_hash_table_destroy (placeholders);
}


/* ----- Public interface ----- */

/**
 * gdl_dock_layout_new:
 * @dock: The dock item.
 * Creates a new #GdlDockLayout
 *
 * Returns: New #GdlDockLayout item.
 */
GdlDockLayout *
gdl_dock_layout_new (GdlDock *dock)
{
    GdlDockMaster *master = NULL;
    
    /* get the master of the given dock */
    if (dock)
        master = GDL_DOCK_OBJECT_GET_MASTER (dock);
    
    return g_object_new (GDL_TYPE_DOCK_LAYOUT,
                         "master", master,
                         NULL);
}

static gboolean
gdl_dock_layout_idle_save (GdlDockLayout *layout)
{
    /* save default layout */
    gdl_dock_layout_save_layout (layout, NULL);
    
    layout->_priv->idle_save_pending = FALSE;
    
    return FALSE;
}

static void
gdl_dock_layout_layout_changed_cb (GdlDockMaster *master,
                                   GdlDockLayout *layout)
{
    /* update model */
    update_items_model (layout);

    if (!layout->_priv->idle_save_pending) {
        g_idle_add ((GSourceFunc) gdl_dock_layout_idle_save, layout);
        layout->_priv->idle_save_pending = TRUE;
    }
}


/**
 * gdl_dock_layout_attach:
 * @layout: The layout item
 * @master: The master item to which the layout will be attached
 *
 * Attach the @layout to the @master and delete the reference to
 * the master that the layout attached previously
 */
void
gdl_dock_layout_attach (GdlDockLayout *layout,
                        GdlDockMaster *master)
{
    g_return_if_fail (layout != NULL);
    g_return_if_fail (master == NULL || GDL_IS_DOCK_MASTER (master));
    
    if (layout->master) {
        g_signal_handlers_disconnect_matched (layout->master, G_SIGNAL_MATCH_DATA,
                                              0, 0, NULL, NULL, layout);
        g_object_unref (layout->master);
    }
    
    gtk_list_store_clear (layout->_priv->items_model);
    
    layout->master = master;
    if (layout->master) {
        g_object_ref (layout->master);
        g_signal_connect (layout->master, "layout-changed",
                          (GCallback) gdl_dock_layout_layout_changed_cb,
                          layout);
    }

    update_items_model (layout);
}

/** 
* gdl_dock_layout_load_layout:
* @layout: The dock item. 
* @name: The name of the layout to load.
*
* Loads the layout with the given name to the memory.
* This will set #GdlDockLayout:dirty to %TRUE.
* 
* See also gdl_dock_layout_load_from_file()
* Returns: %TRUE if layout successfully loaded else %FALSE
*/
gboolean
gdl_dock_layout_load_layout (GdlDockLayout *layout,
                             const gchar   *name)
{
    xmlNodePtr  node;
    gchar      *layout_name;

    g_return_val_if_fail (layout != NULL, FALSE);
    
    if (!layout->_priv->doc || !layout->master)
        return FALSE;

    if (!name)
        layout_name = DEFAULT_LAYOUT;
    else
        layout_name = (gchar *) name;

    node = gdl_dock_layout_find_layout (layout, layout_name);
    if (!node && !name)
        /* return the first layout if the default name failed to load */
        node = gdl_dock_layout_find_layout (layout, NULL);

    if (node) {
        gdl_dock_layout_load (layout->master, node);
        return TRUE;
    } else
        return FALSE;
}

/** 
* gdl_dock_layout_save_layout:
* @layout: The dock item. 
* @name: The name of the layout to save.
*
* Saves the @layout with the given name to the memory.
* This will set #GdlDockLayout:dirty to %TRUE.
* 
* See also gdl_dock_layout_save_to_file().
*/

void
gdl_dock_layout_save_layout (GdlDockLayout *layout,
                             const gchar   *name)
{
    xmlNodePtr  node;
    gchar      *layout_name;

    g_return_if_fail (layout != NULL);
    g_return_if_fail (layout->master != NULL);
    
    if (!layout->_priv->doc)
        gdl_dock_layout_build_doc (layout);

    if (!name)
        layout_name = DEFAULT_LAYOUT;
    else
        layout_name = (gchar *) name;

    /* delete any previously node with the same name */
    node = gdl_dock_layout_find_layout (layout, layout_name);
    if (node) {
        xmlUnlinkNode (node);
        xmlFreeNode (node);
    };

    /* create the new node */
    node = xmlNewChild (layout->_priv->doc->children, NULL, 
                        BAD_CAST LAYOUT_ELEMENT_NAME, NULL);
    xmlSetProp (node, BAD_CAST NAME_ATTRIBUTE_NAME, BAD_CAST layout_name);

    /* save the layout */
    gdl_dock_layout_save (layout->master, node);
    layout->dirty = TRUE;
    g_object_notify (G_OBJECT (layout), "dirty");
}

/** 
* gdl_dock_layout_delete_layout:
* @layout: The dock item. 
* @name: The name of the layout to delete.
*
* Deletes the layout with the given name from the memory.
* This will set #GdlDockLayout:dirty to %TRUE.
*/

void
gdl_dock_layout_delete_layout (GdlDockLayout *layout,
                               const gchar   *name)
{
    xmlNodePtr node;

    g_return_if_fail (layout != NULL);

    /* don't allow the deletion of the default layout */
    if (!name || !strcmp (DEFAULT_LAYOUT, name))
        return;
    
    node = gdl_dock_layout_find_layout (layout, name);
    if (node) {
        xmlUnlinkNode (node);
        xmlFreeNode (node);
        layout->dirty = TRUE;
        g_object_notify (G_OBJECT (layout), "dirty");
    }
}

/** 
* gdl_dock_layout_run_manager:
* @layout: The dock item. 
*
* Runs the layout manager.
*/

void
gdl_dock_layout_run_manager (GdlDockLayout *layout)
{
    GtkWidget *dialog;
    GtkWidget *parent = NULL;
    
    g_return_if_fail (layout != NULL);

    if (!layout->master)
        /* not attached to a dock yet */
        return;

    dialog = gdl_dock_layout_construct_items_ui (layout);
    
    gtk_dialog_run (GTK_DIALOG (dialog));

    gtk_widget_destroy (dialog);
}

/** 
* gdl_dock_layout_load_from_file:
* @layout: The layout item. 
* @filename: The name of the file to load.
*
* Loads the layout from file with the given @filename.
* This will set #GdlDockLayout:dirty to %FALSE.
*
* Returns: %TRUE if @layout successfully loaded else %FALSE
*/

gboolean
gdl_dock_layout_load_from_file (GdlDockLayout *layout,
                                const gchar   *filename)
{
    gboolean retval = FALSE;

    if (layout->_priv->doc) {
        xmlFreeDoc (layout->_priv->doc);
        layout->_priv->doc = NULL;
        layout->dirty = FALSE;
        g_object_notify (G_OBJECT (layout), "dirty");
    }

    /* FIXME: cannot open symlinks */
    if (g_file_test (filename, G_FILE_TEST_IS_REGULAR)) {
        layout->_priv->doc = xmlParseFile (filename);
        if (layout->_priv->doc) {
            xmlNodePtr root = layout->_priv->doc->children;
            /* minimum validation: test the root element */
            if (root && !strcmp ((char*)root->name, ROOT_ELEMENT)) {
                update_layouts_model (layout);
                retval = TRUE;
            } else {
                xmlFreeDoc (layout->_priv->doc);
                layout->_priv->doc = NULL;
            }		
        }
    }

    return retval;
}

/** 
 * gdl_dock_layout_save_to_file:
 * @layout: The layout item.
 * @filename: Name of the file we want to save in layout
 *
 * This function saves the current layout in XML format to 
 * the file with the given @filename.
 *
 * Returns: %TRUE if @layout successfuly save to the file, otherwise %FALSE.
 */
gboolean
gdl_dock_layout_save_to_file (GdlDockLayout *layout,
                              const gchar   *filename)
{
    FILE     *file_handle;
    int       bytes;
    gboolean  retval = FALSE;

    g_return_val_if_fail (layout != NULL, FALSE);
    g_return_val_if_fail (filename != NULL, FALSE);

    /* if there is still no xml doc, create an empty one */
    if (!layout->_priv->doc)
        gdl_dock_layout_build_doc (layout);

    file_handle = fopen (filename, "w");
    if (file_handle) {
        bytes = xmlDocDump (file_handle, layout->_priv->doc);
        if (bytes >= 0) {
            layout->dirty = FALSE;
            g_object_notify (G_OBJECT (layout), "dirty");
            retval = TRUE;
        };
        fclose (file_handle);
    };

    return retval;
}

/**
 * gdl_dock_layout_is_dirty:
 * @layout: The layout item.
 *
 * Checks whether the XML tree in memory is different from the file where the layout was saved.
 * Returns: %TRUE is the layout in the memory is different from the file, else %FALSE.
 */
gboolean
gdl_dock_layout_is_dirty (GdlDockLayout *layout)
{
    g_return_val_if_fail (layout != NULL, FALSE);

    return layout->dirty;
};

GList *
gdl_dock_layout_get_layouts (GdlDockLayout *layout,
                             gboolean       include_default)
{
    GList      *retval = NULL;
    xmlNodePtr  node;

    g_return_val_if_fail (layout != NULL, NULL);

    if (!layout->_priv->doc)
        return NULL;

    node = layout->_priv->doc->children;
    for (node = node->children; node; node = node->next) {
        xmlChar *name;

        if (strcmp ((char*)node->name, LAYOUT_ELEMENT_NAME))
            continue;

        name = xmlGetProp (node, BAD_CAST NAME_ATTRIBUTE_NAME);
        if (include_default || strcmp ((char*)name, DEFAULT_LAYOUT))
            retval = g_list_prepend (retval, g_strdup ((char*)name));
        xmlFree (name);
    };
    retval = g_list_reverse (retval);

    return retval;
}

GtkWidget *
gdl_dock_layout_get_layouts_ui (GdlDockLayout *layout)
{
    GtkWidget *ui;

    g_return_val_if_fail (layout != NULL, NULL);
    ui = gdl_dock_layout_construct_layouts_ui (layout);

    return ui;
}
