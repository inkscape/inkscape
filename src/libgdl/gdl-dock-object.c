/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- 
 *
 * gdl-dock-object.c - Abstract base class for all dock related objects
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
#include <stdlib.h>
#include <string.h>

#include "gdl-dock-object.h"
#include "gdl-dock-master.h"
#include "libgdltypebuiltins.h"
#include "libgdlmarshal.h"

/* for later use by the registry */
#include "gdl-dock.h"
#include "gdl-dock-item.h"
#include "gdl-dock-paned.h"
#include "gdl-dock-notebook.h"
#include "gdl-dock-placeholder.h"


/* ----- Private prototypes ----- */

static void     gdl_dock_object_class_init         (GdlDockObjectClass *klass);

static void     gdl_dock_object_set_property       (GObject            *g_object,
                                                    guint               prop_id,
                                                    const GValue       *value,
                                                    GParamSpec         *pspec);
static void     gdl_dock_object_get_property       (GObject            *g_object,
                                                    guint               prop_id,
                                                    GValue             *value,
                                                    GParamSpec         *pspec);
static void     gdl_dock_object_finalize           (GObject            *g_object);

static void     gdl_dock_object_destroy            (GtkObject          *gtk_object);

static void     gdl_dock_object_show               (GtkWidget          *widget);
static void     gdl_dock_object_hide               (GtkWidget          *widget);

static void     gdl_dock_object_real_detach        (GdlDockObject      *object,
                                                    gboolean            recursive);
static void     gdl_dock_object_real_reduce        (GdlDockObject      *object);
static void     gdl_dock_object_dock_unimplemented (GdlDockObject     *object,
                                                    GdlDockObject     *requestor,
                                                    GdlDockPlacement   position,
                                                    GValue            *other_data);
static void     gdl_dock_object_real_present       (GdlDockObject     *object,
                                                    GdlDockObject     *child);


/* ----- Private data types and variables ----- */

enum {
    PROP_0,
    PROP_NAME,
    PROP_LONG_NAME,
    PROP_STOCK_ID,
    PROP_PIXBUF_ICON,
    PROP_MASTER,
    PROP_EXPORT_PROPERTIES
};

enum {
    DETACH,
    DOCK,
    LAST_SIGNAL
};

static guint gdl_dock_object_signals [LAST_SIGNAL] = { 0 };

struct DockRegisterItem {
	gchar* nick;
	gpointer type;
};

static GArray *dock_register = NULL;

/* ----- Private interface ----- */

G_DEFINE_TYPE (GdlDockObject, gdl_dock_object, GTK_TYPE_CONTAINER);

static void
gdl_dock_object_class_init (GdlDockObjectClass *klass)
{
    GObjectClass      *g_object_class;
    GtkObjectClass    *object_class;
    GtkWidgetClass    *widget_class;
    GtkContainerClass *container_class;

    g_object_class = G_OBJECT_CLASS (klass);
    object_class = GTK_OBJECT_CLASS (klass);
    widget_class = GTK_WIDGET_CLASS (klass);
    container_class = GTK_CONTAINER_CLASS (klass);

    g_object_class->set_property = gdl_dock_object_set_property;
    g_object_class->get_property = gdl_dock_object_get_property;
    g_object_class->finalize = gdl_dock_object_finalize;

    g_object_class_install_property (
        g_object_class, PROP_NAME,
        g_param_spec_string (GDL_DOCK_NAME_PROPERTY, _("Name"),
                             _("Unique name for identifying the dock object"),
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                             GDL_DOCK_PARAM_EXPORT));

    g_object_class_install_property (
        g_object_class, PROP_LONG_NAME,
        g_param_spec_string ("long-name", _("Long name"),
                             _("Human readable name for the dock object"),
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (
        g_object_class, PROP_STOCK_ID,
        g_param_spec_string ("stock-id", _("Stock Icon"),
                             _("Stock icon for the dock object"),
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (
        g_object_class, PROP_PIXBUF_ICON,
        g_param_spec_pointer ("pixbuf-icon", _("Pixbuf Icon"),
                              _("Pixbuf icon for the dock object"),
                              G_PARAM_READWRITE));

    g_object_class_install_property (
        g_object_class, PROP_MASTER,
        g_param_spec_object ("master", _("Dock master"),
                             _("Dock master this dock object is bound to"),
                             GDL_TYPE_DOCK_MASTER,
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
    
    object_class->destroy = gdl_dock_object_destroy;
    
    widget_class->show = gdl_dock_object_show;
    widget_class->hide = gdl_dock_object_hide;
    
    klass->is_compound = TRUE;
    
    klass->detach = gdl_dock_object_real_detach;
    klass->reduce = gdl_dock_object_real_reduce;
    klass->dock_request = NULL;
    klass->dock = gdl_dock_object_dock_unimplemented;
    klass->reorder = NULL;
    klass->present = gdl_dock_object_real_present;
    klass->child_placement = NULL;
    
    gdl_dock_object_signals [DETACH] =
        g_signal_new ("detach",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GdlDockObjectClass, detach),
                      NULL,
                      NULL,
                      gdl_marshal_VOID__BOOLEAN,
                      G_TYPE_NONE,
                      1,
                      G_TYPE_BOOLEAN);

    gdl_dock_object_signals [DOCK] =
        g_signal_new ("dock",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GdlDockObjectClass, dock),
                      NULL,
                      NULL,
                      gdl_marshal_VOID__OBJECT_ENUM_BOXED,
                      G_TYPE_NONE,
                      3,
                      GDL_TYPE_DOCK_OBJECT,
                      GDL_TYPE_DOCK_PLACEMENT,
                      G_TYPE_VALUE);
}

static void
gdl_dock_object_init (GdlDockObject *object)
{
    object->flags = GDL_DOCK_AUTOMATIC;
    object->freeze_count = 0;
}

static void
gdl_dock_object_set_property  (GObject      *g_object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
    GdlDockObject *object = GDL_DOCK_OBJECT (g_object);

    switch (prop_id) {
    case PROP_NAME:
        g_free (object->name);
        object->name = g_value_dup_string (value);
        break;
    case PROP_LONG_NAME:
        g_free (object->long_name);
        object->long_name = g_value_dup_string (value);
        break;
    case PROP_STOCK_ID:
        g_free (object->stock_id);
        object->stock_id = g_value_dup_string (value);
        break;
    case PROP_PIXBUF_ICON:
        object->pixbuf_icon = g_value_get_pointer (value);
        break;
    case PROP_MASTER:
        if (g_value_get_object (value)) 
            gdl_dock_object_bind (object, g_value_get_object (value));
        else
            gdl_dock_object_unbind (object);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
gdl_dock_object_get_property  (GObject      *g_object,
                               guint         prop_id,
                               GValue       *value,
                               GParamSpec   *pspec)
{
    GdlDockObject *object = GDL_DOCK_OBJECT (g_object);

    switch (prop_id) {
    case PROP_NAME:
        g_value_set_string (value, object->name);
        break;
    case PROP_LONG_NAME:
        g_value_set_string (value, object->long_name);
        break;
    case PROP_STOCK_ID:
        g_value_set_string (value, object->stock_id);
        break;
    case PROP_PIXBUF_ICON:
        g_value_set_pointer (value, object->pixbuf_icon);
        break;
    case PROP_MASTER:
        g_value_set_object (value, object->master);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
gdl_dock_object_finalize (GObject *g_object)
{
    GdlDockObject *object;
    
    g_return_if_fail (g_object != NULL && GDL_IS_DOCK_OBJECT (g_object));

    object = GDL_DOCK_OBJECT (g_object);

    g_free (object->name);
    object->name = NULL;
    g_free (object->long_name);
    object->long_name = NULL;
    g_free (object->stock_id);
    object->stock_id = NULL;
    object->pixbuf_icon = NULL;

    G_OBJECT_CLASS (gdl_dock_object_parent_class)->finalize (g_object);
}

static void
gdl_dock_object_foreach_detach (GdlDockObject *object,
                                gpointer       user_data)
{
    gdl_dock_object_detach (object, TRUE);
}

static void
gdl_dock_object_destroy (GtkObject *gtk_object)
{
    GdlDockObject *object;

    g_return_if_fail (GDL_IS_DOCK_OBJECT (gtk_object));

    object = GDL_DOCK_OBJECT (gtk_object);
    if (gdl_dock_object_is_compound (object)) {
        /* detach our dock object children if we have some, and even
           if we are not attached, so they can get notification */
        gdl_dock_object_freeze (object);
        gtk_container_foreach (GTK_CONTAINER (object),
                               (GtkCallback) gdl_dock_object_foreach_detach,
                               NULL);
        object->reduce_pending = FALSE;
        gdl_dock_object_thaw (object);
    }
    if (GDL_DOCK_OBJECT_ATTACHED (object)) {
        /* detach ourselves */
        gdl_dock_object_detach (object, FALSE);
    }
    
    /* finally unbind us */
    if (object->master)
        gdl_dock_object_unbind (object);
        
    GTK_OBJECT_CLASS(gdl_dock_object_parent_class)->destroy (gtk_object);
}

static void
gdl_dock_object_foreach_automatic (GdlDockObject *object,
                                   gpointer       user_data)
{
    void (* function) (GtkWidget *) = user_data;

    if (GDL_DOCK_OBJECT_AUTOMATIC (object))
        (* function) (GTK_WIDGET (object));
}

static void
gdl_dock_object_show (GtkWidget *widget)
{
    if (gdl_dock_object_is_compound (GDL_DOCK_OBJECT (widget))) {
        gtk_container_foreach (GTK_CONTAINER (widget),
                               (GtkCallback) gdl_dock_object_foreach_automatic,
                               gtk_widget_show);
    }
    GTK_WIDGET_CLASS (gdl_dock_object_parent_class)->show (widget);
}

static void
gdl_dock_object_hide (GtkWidget *widget)
{
    if (gdl_dock_object_is_compound (GDL_DOCK_OBJECT (widget))) {
        gtk_container_foreach (GTK_CONTAINER (widget),
                               (GtkCallback) gdl_dock_object_foreach_automatic,
                               gtk_widget_hide);
    }
    GTK_WIDGET_CLASS (gdl_dock_object_parent_class)->hide (widget);
}

static void
gdl_dock_object_real_detach (GdlDockObject *object,
                             gboolean       recursive)
{
    GdlDockObject *parent;
    GtkWidget     *widget;
    
    g_return_if_fail (object != NULL);

    /* detach children */
    if (recursive && gdl_dock_object_is_compound (object)) {
        gtk_container_foreach (GTK_CONTAINER (object),
                               (GtkCallback) gdl_dock_object_detach,
                               GINT_TO_POINTER (recursive));
    }
    
    /* detach the object itself */
    GDL_DOCK_OBJECT_UNSET_FLAGS (object, GDL_DOCK_ATTACHED);
    parent = gdl_dock_object_get_parent_object (object);
    widget = GTK_WIDGET (object);
    if (gtk_widget_get_parent (widget))
        gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (GTK_WIDGET (widget))), widget);
    if (parent)
        gdl_dock_object_reduce (parent);
}

static void
gdl_dock_object_real_reduce (GdlDockObject *object)
{
    GdlDockObject *parent;
    GList         *children;
    
    g_return_if_fail (object != NULL);

    if (!gdl_dock_object_is_compound (object))
        return;

    parent = gdl_dock_object_get_parent_object (object);
    children = gtk_container_get_children (GTK_CONTAINER (object));
    if (g_list_length (children) <= 1) {
        GList *l;
        GList *dchildren = NULL;
        
        /* detach ourselves and then re-attach our children to our
           current parent.  if we are not currently attached, the
           children are detached */
        if (parent)
            gdl_dock_object_freeze (parent);
        gdl_dock_object_freeze (object);
        /* Detach the children before detaching this object, since in this
         * way the children can have access to the whole object hierarchy.
         * Set the InDetach flag now, so the children know that this object
         * is going to be detached. */
        
        
        GDL_DOCK_OBJECT_SET_FLAGS (object, GDL_DOCK_IN_DETACH);
        
        for (l = children; l; l = l->next) {
            GdlDockObject *child;
            
            if (!GDL_IS_DOCK_OBJECT (l->data))
                continue;
            
            child = GDL_DOCK_OBJECT (l->data);

            g_object_ref (child);
            gdl_dock_object_detach (child, FALSE);
            GDL_DOCK_OBJECT_SET_FLAGS (child, GDL_DOCK_IN_REFLOW);
            if (parent)
                dchildren = g_list_append (dchildren, child);
            GDL_DOCK_OBJECT_UNSET_FLAGS (child, GDL_DOCK_IN_REFLOW);
        }
        /* Now it can be detached */
        gdl_dock_object_detach (object, FALSE);
        
        /* After detaching the reduced object, we can add the
        children (the only child in fact) to the new parent */
        for (l = dchildren; l; l = l->next) {
            gtk_container_add (GTK_CONTAINER (parent), l->data);
            g_object_unref (l->data);
        }
        g_list_free (dchildren);
        
        
        /* sink the widget, so any automatic floating widget is destroyed */
        g_object_ref_sink (object);
        /* don't reenter */
        object->reduce_pending = FALSE;
        gdl_dock_object_thaw (object);
        if (parent)
            gdl_dock_object_thaw (parent);
    }
    g_list_free (children);
}

static void
gdl_dock_object_dock_unimplemented (GdlDockObject    *object,
                                    GdlDockObject    *requestor,
                                    GdlDockPlacement  position,
                                    GValue           *other_data)
{
    g_warning (_("Call to gdl_dock_object_dock in a dock object %p "
                 "(object type is %s) which hasn't implemented this method"),
               object, G_OBJECT_TYPE_NAME (object));
}

static void 
gdl_dock_object_real_present (GdlDockObject *object,
                              GdlDockObject *child)
{
    gtk_widget_show (GTK_WIDGET (object));
}


/* ----- Public interface ----- */

gboolean
gdl_dock_object_is_compound (GdlDockObject *object)
{
    GdlDockObjectClass *klass;

    g_return_val_if_fail (object != NULL, FALSE);
    g_return_val_if_fail (GDL_IS_DOCK_OBJECT (object), FALSE);

    klass = GDL_DOCK_OBJECT_GET_CLASS (object);
    return klass->is_compound;
}

void
gdl_dock_object_detach (GdlDockObject *object,
                        gboolean       recursive)
{
    g_return_if_fail (object != NULL);

    if (!GDL_IS_DOCK_OBJECT (object))
        return;
    
    if (!GDL_DOCK_OBJECT_ATTACHED (object))
        return;
    
    /* freeze the object to avoid reducing while detaching children */
    gdl_dock_object_freeze (object);
    GDL_DOCK_OBJECT_SET_FLAGS (object, GDL_DOCK_IN_DETACH);
    g_signal_emit (object, gdl_dock_object_signals [DETACH], 0, recursive);
    GDL_DOCK_OBJECT_UNSET_FLAGS (object, GDL_DOCK_IN_DETACH);
    gdl_dock_object_thaw (object);
}

GdlDockObject *
gdl_dock_object_get_parent_object (GdlDockObject *object)
{
    GtkWidget *parent;
    
    g_return_val_if_fail (object != NULL, NULL);

    parent = gtk_widget_get_parent (GTK_WIDGET (object));
    while (parent && !GDL_IS_DOCK_OBJECT (parent)) {
        parent = gtk_widget_get_parent (parent);
    }
    
    return parent ? GDL_DOCK_OBJECT (parent) : NULL;
}

void
gdl_dock_object_freeze (GdlDockObject *object)
{
    g_return_if_fail (object != NULL);
    
    if (object->freeze_count == 0) {
        g_object_ref (object);   /* dock objects shouldn't be
                                    destroyed if they are frozen */
    }
    object->freeze_count++;
}

void
gdl_dock_object_thaw (GdlDockObject *object)
{
    g_return_if_fail (object != NULL);
    g_return_if_fail (object->freeze_count > 0);
    
    object->freeze_count--;
    if (object->freeze_count == 0) {
        if (object->reduce_pending) {
            object->reduce_pending = FALSE;
            gdl_dock_object_reduce (object);
        }
        g_object_unref (object);
    }
}

void
gdl_dock_object_reduce (GdlDockObject *object)
{
    g_return_if_fail (object != NULL);

    if (GDL_DOCK_OBJECT_FROZEN (object)) {
        object->reduce_pending = TRUE;
        return;
    }

    if (GDL_DOCK_OBJECT_GET_CLASS (object)->reduce)
        GDL_DOCK_OBJECT_GET_CLASS (object)->reduce (object);
}

gboolean
gdl_dock_object_dock_request (GdlDockObject  *object,
                              gint            x,
                              gint            y,
                              GdlDockRequest *request)
{
    g_return_val_if_fail (object != NULL && request != NULL, FALSE);

    if (GDL_DOCK_OBJECT_GET_CLASS (object)->dock_request)
        return GDL_DOCK_OBJECT_GET_CLASS (object)->dock_request (object, x, y, request);
    else
        return FALSE;
}

/**
 * gdl_dock_object_dock:
 * @object:
 * @requestor:
 * @position:
 * @other_data: (allow-none):
 **/
void
gdl_dock_object_dock (GdlDockObject    *object,
                      GdlDockObject    *requestor,
                      GdlDockPlacement  position,
                      GValue           *other_data)
{
    GdlDockObject *parent;
    
    g_return_if_fail (object != NULL && requestor != NULL);
        
    if (object == requestor)
        return;
    
    if (!object->master)
        g_warning (_("Dock operation requested in a non-bound object %p. "
                     "The application might crash"), object);
        
    if (!gdl_dock_object_is_bound (requestor))
        gdl_dock_object_bind (requestor, object->master);

    if (requestor->master != object->master) {
        g_warning (_("Cannot dock %p to %p because they belong to different masters"),
                   requestor, object);
        return;
    }

    /* first, see if we can optimize things by reordering */
    if (position != GDL_DOCK_NONE) {
        parent = gdl_dock_object_get_parent_object (object);
        if (gdl_dock_object_reorder (object, requestor, position, other_data) ||
            (parent && gdl_dock_object_reorder (parent, requestor, position, other_data)))
            return;
    }
    
    /* freeze the object, since under some conditions it might be destroyed when
       detaching the requestor */
    gdl_dock_object_freeze (object);

    /* detach the requestor before docking */
    g_object_ref (requestor);
    if (GDL_DOCK_OBJECT_ATTACHED (requestor))
        gdl_dock_object_detach (requestor, FALSE);
    
    if (position != GDL_DOCK_NONE)
        g_signal_emit (object, gdl_dock_object_signals [DOCK], 0,
                       requestor, position, other_data);

    g_object_unref (requestor);
    gdl_dock_object_thaw (object);
}

void
gdl_dock_object_bind (GdlDockObject *object,
                      GObject       *master)
{
    g_return_if_fail (object != NULL && master != NULL);
    g_return_if_fail (GDL_IS_DOCK_MASTER (master));
    
    if (object->master == master)
        /* nothing to do here */
        return;
    
    if (object->master) {
        g_warning (_("Attempt to bind to %p an already bound dock object %p "
                     "(current master: %p)"), master, object, object->master);
        return;
    }

    gdl_dock_master_add (GDL_DOCK_MASTER (master), object);
    object->master = master;
    g_object_add_weak_pointer (master, (gpointer *) &object->master);

    g_object_notify (G_OBJECT (object), "master");
}

void
gdl_dock_object_unbind (GdlDockObject *object)
{
    g_return_if_fail (object != NULL);

    g_object_ref (object);

    /* detach the object first */
    if (GDL_DOCK_OBJECT_ATTACHED (object))
        gdl_dock_object_detach (object, TRUE);
    
    if (object->master) {
        GObject *master = object->master;
        g_object_remove_weak_pointer (master, (gpointer *) &object->master);
        object->master = NULL;
        gdl_dock_master_remove (GDL_DOCK_MASTER (master), object);
        g_object_notify (G_OBJECT (object), "master");
    }
    g_object_unref (object);
}

gboolean
gdl_dock_object_is_bound (GdlDockObject *object)
{
    g_return_val_if_fail (object != NULL, FALSE);
    return (object->master != NULL);
}

gboolean
gdl_dock_object_reorder (GdlDockObject    *object,
                         GdlDockObject    *child,
                         GdlDockPlacement  new_position,
                         GValue           *other_data)
{
    g_return_val_if_fail (object != NULL && child != NULL, FALSE);

    if (GDL_DOCK_OBJECT_GET_CLASS (object)->reorder)
            return GDL_DOCK_OBJECT_GET_CLASS (object)->reorder (object, child, new_position, other_data);
    else
        return FALSE;
}

void 
gdl_dock_object_present (GdlDockObject *object,
                         GdlDockObject *child)
{
    GdlDockObject *parent;
    
    g_return_if_fail (object != NULL && GDL_IS_DOCK_OBJECT (object));

    parent = gdl_dock_object_get_parent_object (object);
    if (parent)
        /* chain the call to our parent */
        gdl_dock_object_present (parent, object);

   if (GDL_DOCK_OBJECT_GET_CLASS (object)->present)
        GDL_DOCK_OBJECT_GET_CLASS (object)->present (object, child);
}

/**
 * gdl_dock_object_child_placement:
 * @object: the dock object we are asking for child placement
 * @child: the child of the @object we want the placement for
 * @placement: where to return the placement information
 *
 * This function returns information about placement of a child dock
 * object inside another dock object.  The function returns %TRUE if
 * @child is effectively a child of @object.  @placement should
 * normally be initially setup to %GDL_DOCK_NONE.  If it's set to some
 * other value, this function will not touch the stored value if the
 * specified placement is "compatible" with the actual placement of
 * the child.
 *
 * @placement can be %NULL, in which case the function simply tells if
 * @child is attached to @object.
 *
 * Returns: %TRUE if @child is a child of @object.
 */
gboolean 
gdl_dock_object_child_placement (GdlDockObject    *object,
                                 GdlDockObject    *child,
                                 GdlDockPlacement *placement)
{
    g_return_val_if_fail (object != NULL && child != NULL, FALSE);

    /* simple case */
    if (!gdl_dock_object_is_compound (object))
        return FALSE;
    
    if (GDL_DOCK_OBJECT_GET_CLASS (object)->child_placement)
        return GDL_DOCK_OBJECT_GET_CLASS (object)->child_placement (object, child, placement);
    else
        return FALSE;
}


/* ----- dock param type functions start here ------ */

static void 
gdl_dock_param_export_int (const GValue *src,
                           GValue       *dst)
{
    dst->data [0].v_pointer = g_strdup_printf ("%d", src->data [0].v_int);
}

static void 
gdl_dock_param_export_uint (const GValue *src,
                            GValue       *dst)
{
    dst->data [0].v_pointer = g_strdup_printf ("%u", src->data [0].v_uint);
}

static void 
gdl_dock_param_export_string (const GValue *src,
                              GValue       *dst)
{
    dst->data [0].v_pointer = g_strdup (src->data [0].v_pointer);
}

static void 
gdl_dock_param_export_bool (const GValue *src,
                            GValue       *dst)
{
    dst->data [0].v_pointer = g_strdup_printf ("%s", src->data [0].v_int ? "yes" : "no");
}

static void 
gdl_dock_param_export_placement (const GValue *src,
                                 GValue       *dst)
{
    switch (src->data [0].v_int) {
        case GDL_DOCK_NONE:
            dst->data [0].v_pointer = g_strdup ("");
            break;
        case GDL_DOCK_TOP:
            dst->data [0].v_pointer = g_strdup ("top");
            break;
        case GDL_DOCK_BOTTOM:
            dst->data [0].v_pointer = g_strdup ("bottom");
            break;
        case GDL_DOCK_LEFT:
            dst->data [0].v_pointer = g_strdup ("left");
            break;
        case GDL_DOCK_RIGHT:
            dst->data [0].v_pointer = g_strdup ("right");
            break;
        case GDL_DOCK_CENTER:
            dst->data [0].v_pointer = g_strdup ("center");
            break;
        case GDL_DOCK_FLOATING:
            dst->data [0].v_pointer = g_strdup ("floating");
            break;
    }
}

static void 
gdl_dock_param_import_int (const GValue *src,
                           GValue       *dst)
{
    dst->data [0].v_int = atoi (src->data [0].v_pointer);
}

static void 
gdl_dock_param_import_uint (const GValue *src,
                            GValue       *dst)
{
    dst->data [0].v_uint = (guint) atoi (src->data [0].v_pointer);
}

static void 
gdl_dock_param_import_string (const GValue *src,
                              GValue       *dst)
{
    dst->data [0].v_pointer = g_strdup (src->data [0].v_pointer);
}

static void 
gdl_dock_param_import_bool (const GValue *src,
                            GValue       *dst)
{
    dst->data [0].v_int = !strcmp (src->data [0].v_pointer, "yes");
}

static void 
gdl_dock_param_import_placement (const GValue *src,
                                 GValue       *dst)
{
    if (!strcmp (src->data [0].v_pointer, "top"))
        dst->data [0].v_int = GDL_DOCK_TOP;
    else if (!strcmp (src->data [0].v_pointer, "bottom"))
        dst->data [0].v_int = GDL_DOCK_BOTTOM;
    else if (!strcmp (src->data [0].v_pointer, "center"))
        dst->data [0].v_int = GDL_DOCK_CENTER;
    else if (!strcmp (src->data [0].v_pointer, "left"))
        dst->data [0].v_int = GDL_DOCK_LEFT;
    else if (!strcmp (src->data [0].v_pointer, "right"))
        dst->data [0].v_int = GDL_DOCK_RIGHT;
    else if (!strcmp (src->data [0].v_pointer, "floating"))
        dst->data [0].v_int = GDL_DOCK_FLOATING;
    else
        dst->data [0].v_int = GDL_DOCK_NONE;
}

GType
gdl_dock_param_get_type (void)
{
    static GType our_type = 0;

    if (our_type == 0) {
        GTypeInfo tinfo = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        our_type = g_type_register_static (G_TYPE_STRING, "GdlDockParam", &tinfo, 0);

        /* register known transform functions */
        /* exporters */
        g_value_register_transform_func (G_TYPE_INT, our_type, gdl_dock_param_export_int);
        g_value_register_transform_func (G_TYPE_UINT, our_type, gdl_dock_param_export_uint);
        g_value_register_transform_func (G_TYPE_STRING, our_type, gdl_dock_param_export_string);
        g_value_register_transform_func (G_TYPE_BOOLEAN, our_type, gdl_dock_param_export_bool);
        g_value_register_transform_func (GDL_TYPE_DOCK_PLACEMENT, our_type, gdl_dock_param_export_placement);
        /* importers */
        g_value_register_transform_func (our_type, G_TYPE_INT, gdl_dock_param_import_int);
        g_value_register_transform_func (our_type, G_TYPE_UINT, gdl_dock_param_import_uint);
        g_value_register_transform_func (our_type, G_TYPE_STRING, gdl_dock_param_import_string);
        g_value_register_transform_func (our_type, G_TYPE_BOOLEAN, gdl_dock_param_import_bool);
        g_value_register_transform_func (our_type, GDL_TYPE_DOCK_PLACEMENT, gdl_dock_param_import_placement);
    }

    return our_type;
}

/* -------------- nick <-> type conversion functions --------------- */

static void
gdl_dock_object_register_init (void)
{
    const size_t n_default = 5;
    guint i = 0;
    struct DockRegisterItem default_items[n_default];

    if (dock_register)
        return;

    dock_register 
        = g_array_new (FALSE, FALSE, sizeof (struct DockRegisterItem));
    
    /* add known types */
    default_items[0].nick = "dock";
    default_items[0].type = (gpointer) GDL_TYPE_DOCK;
    default_items[1].nick = "item";
    default_items[1].type = (gpointer) GDL_TYPE_DOCK_ITEM;
    default_items[2].nick = "paned";
    default_items[2].type = (gpointer) GDL_TYPE_DOCK_PANED;
    default_items[3].nick = "notebook";
    default_items[3].type = (gpointer) GDL_TYPE_DOCK_NOTEBOOK;
    default_items[4].nick = "placeholder";
    default_items[4].type = (gpointer) GDL_TYPE_DOCK_PLACEHOLDER;

    for (i = 0; i < n_default; i++)
        g_array_append_val (dock_register, default_items[i]);
}

/**
 * gdl_dock_object_nick_from_type:
 * @type: The type for which to find the nickname
 *
 * Finds the nickname for a given type
 *
 * Returns: If the object has a nickname, then it is returned.
 *   Otherwise, the type name.
 */
const gchar *
gdl_dock_object_nick_from_type (GType type)
{
    gchar *nick = NULL;
    guint i = 0;
    
    if (!dock_register)
        gdl_dock_object_register_init ();

    for (i=0; i < dock_register->len; i++) {
        struct DockRegisterItem item 
            = g_array_index (dock_register, struct DockRegisterItem, i);

	if (g_direct_equal (item.type, (gpointer) type))
		nick = g_strdup (item.nick);
    }
    
    return nick ? nick : g_type_name (type);
}

/**
 * gdl_dock_object_type_from_nick:
 * @nick: The nickname for the object type
 *
 * Finds the object type assigned to a given nickname.
 *
 * Returns: If the nickname has previously been assigned, then the corresponding
 * object type is returned.  Otherwise, %G_TYPE_NONE.
 */
GType
gdl_dock_object_type_from_nick (const gchar *nick)
{
    GType type = G_TYPE_NONE;
    gboolean nick_is_in_register = FALSE;
    guint i = 0;
    
    if (!dock_register)
        gdl_dock_object_register_init ();

    for (i = 0; i < dock_register->len; i++) {
        struct DockRegisterItem item 
		= g_array_index (dock_register, struct DockRegisterItem, i);

	if (!g_strcmp0 (nick, item.nick)) {
	    nick_is_in_register = TRUE;
	    type = (GType) item.type;
	}
    }
    if (!nick_is_in_register) {
        /* try searching in the glib type system */
        type = g_type_from_name (nick);
    }
    
    return type;
}

/**
 * gdl_dock_object_set_type_for_nick:
 * @nick: The nickname for the object type
 * @type: The object type
 *
 * Assigns an object type to a given nickname.  If the nickname already exists,
 * then it reassigns it to a new object type.
 *
 * Returns: If the nick was previously assigned, the old type is returned.
 * Otherwise, %G_TYPE_NONE.
 */
GType
gdl_dock_object_set_type_for_nick (const gchar *nick,
                                   GType        type)
{
    GType old_type = G_TYPE_NONE;
    guint i = 0;
    struct DockRegisterItem new_item;
    new_item.nick = g_strdup(nick);
    new_item.type = (gpointer) type;
    
    if (!dock_register)
        gdl_dock_object_register_init ();

    g_return_val_if_fail (g_type_is_a (type, GDL_TYPE_DOCK_OBJECT), G_TYPE_NONE);
    
    for (i = 0; i < dock_register->len; i++) {
        struct DockRegisterItem item 
            = g_array_index (dock_register, struct DockRegisterItem, i);

	if (!g_strcmp0 (nick, item.nick)) {
            old_type = (GType) item.type;
	    g_array_insert_val (dock_register, i, new_item);
	}
    }

    return old_type;
}

