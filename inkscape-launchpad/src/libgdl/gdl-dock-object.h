/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- 
 *
 * gdl-dock-object.h - Abstract base class for all dock related objects
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

#ifndef __GDL_DOCK_OBJECT_H__
#define __GDL_DOCK_OBJECT_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

/* standard macros */
#define GDL_TYPE_DOCK_OBJECT             (gdl_dock_object_get_type ())
#define GDL_DOCK_OBJECT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GDL_TYPE_DOCK_OBJECT, GdlDockObject))
#define GDL_DOCK_OBJECT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GDL_TYPE_DOCK_OBJECT, GdlDockObjectClass))
#define GDL_IS_DOCK_OBJECT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GDL_TYPE_DOCK_OBJECT))
#define GDL_IS_DOCK_OBJECT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GDL_TYPE_DOCK_OBJECT))
#define GDL_DOCK_OBJECT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_DOCK_OBJECT, GdlDockObjectClass))

/* data types & structures */
typedef enum {
    /* the parameter is to be exported for later layout rebuilding */
    GDL_DOCK_PARAM_EXPORT = 1 << G_PARAM_USER_SHIFT,
    /* the parameter must be set after adding the children objects */
    GDL_DOCK_PARAM_AFTER  = 1 << (G_PARAM_USER_SHIFT + 1)
} GdlDockParamFlags;

#define GDL_DOCK_NAME_PROPERTY    "name"
#define GDL_DOCK_MASTER_PROPERTY  "master"

typedef enum {
    GDL_DOCK_AUTOMATIC  = 1 << 0,
    GDL_DOCK_ATTACHED   = 1 << 1,
    GDL_DOCK_IN_REFLOW  = 1 << 2,
    GDL_DOCK_IN_DETACH  = 1 << 3
} GdlDockObjectFlags;

#define GDL_DOCK_OBJECT_FLAGS_SHIFT 8

typedef enum {
    GDL_DOCK_NONE = 0,
    GDL_DOCK_TOP,
    GDL_DOCK_BOTTOM,
    GDL_DOCK_RIGHT,
    GDL_DOCK_LEFT,
    GDL_DOCK_CENTER,
    GDL_DOCK_FLOATING
} GdlDockPlacement;

typedef struct _GdlDockObject      GdlDockObject;
typedef struct _GdlDockObjectClass GdlDockObjectClass;
typedef struct _GdlDockRequest     GdlDockRequest;

struct _GdlDockRequest {
    GdlDockObject    *applicant;
    GdlDockObject    *target;
    GdlDockPlacement  position;
    GdkRectangle      rect;
    GValue            extra;
};

struct _GdlDockObject {
    GtkContainer        container;

    GdlDockObjectFlags  flags;
    gint                freeze_count;
    
    GObject            *master;
    gchar              *name;
    gchar              *long_name;
    gchar              *stock_id;
    GdkPixbuf          *pixbuf_icon;
    
    gboolean            reduce_pending;
};

struct _GdlDockObjectClass {
    GtkContainerClass parent_class;

    gboolean          is_compound;
    
    void     (* detach)          (GdlDockObject    *object,
                                  gboolean          recursive);
    void     (* reduce)          (GdlDockObject    *object);

    gboolean (* dock_request)    (GdlDockObject    *object,
                                  gint              x,
                                  gint              y,
                                  GdlDockRequest   *request);

    void     (* dock)            (GdlDockObject    *object,
                                  GdlDockObject    *requestor,
                                  GdlDockPlacement  position,
                                  GValue           *other_data);
    
    gboolean (* reorder)         (GdlDockObject    *object,
                                  GdlDockObject    *child,
                                  GdlDockPlacement  new_position,
                                  GValue           *other_data);

    void     (* present)         (GdlDockObject    *object,
                                  GdlDockObject    *child);

    gboolean (* child_placement) (GdlDockObject    *object,
                                  GdlDockObject    *child,
                                  GdlDockPlacement *placement);
};

/* additional macros */
#define GDL_DOCK_OBJECT_FLAGS(obj)  (GDL_DOCK_OBJECT (obj)->flags)
#define GDL_DOCK_OBJECT_AUTOMATIC(obj) \
    ((GDL_DOCK_OBJECT_FLAGS (obj) & GDL_DOCK_AUTOMATIC) != 0)
#define GDL_DOCK_OBJECT_ATTACHED(obj) \
    ((GDL_DOCK_OBJECT_FLAGS (obj) & GDL_DOCK_ATTACHED) != 0)
#define GDL_DOCK_OBJECT_IN_REFLOW(obj) \
    ((GDL_DOCK_OBJECT_FLAGS (obj) & GDL_DOCK_IN_REFLOW) != 0)
#define GDL_DOCK_OBJECT_IN_DETACH(obj) \
    ((GDL_DOCK_OBJECT_FLAGS (obj) & GDL_DOCK_IN_DETACH) != 0)

#define GDL_DOCK_OBJECT_SET_FLAGS(obj,flag) \
    G_STMT_START { (GDL_DOCK_OBJECT_FLAGS (obj) |= (flag)); } G_STMT_END
#define GDL_DOCK_OBJECT_UNSET_FLAGS(obj,flag) \
    G_STMT_START { (GDL_DOCK_OBJECT_FLAGS (obj) &= ~(flag)); } G_STMT_END
 
#define GDL_DOCK_OBJECT_FROZEN(obj) (GDL_DOCK_OBJECT (obj)->freeze_count > 0)


/* public interface */
 
GType          gdl_dock_object_get_type          (void);

gboolean       gdl_dock_object_is_compound       (GdlDockObject    *object);

void           gdl_dock_object_detach            (GdlDockObject    *object,
                                                  gboolean          recursive);

GdlDockObject *gdl_dock_object_get_parent_object (GdlDockObject    *object);

void           gdl_dock_object_freeze            (GdlDockObject    *object);
void           gdl_dock_object_thaw              (GdlDockObject    *object);

void           gdl_dock_object_reduce            (GdlDockObject    *object);

gboolean       gdl_dock_object_dock_request      (GdlDockObject    *object,
                                                  gint              x,
                                                  gint              y,
                                                  GdlDockRequest   *request);
void           gdl_dock_object_dock              (GdlDockObject    *object,
                                                  GdlDockObject    *requestor,
                                                  GdlDockPlacement  position,
                                                  GValue           *other_data);

void           gdl_dock_object_bind              (GdlDockObject    *object,
                                                  GObject          *master);
void           gdl_dock_object_unbind            (GdlDockObject    *object);
gboolean       gdl_dock_object_is_bound          (GdlDockObject    *object);

gboolean       gdl_dock_object_reorder           (GdlDockObject    *object,
                                                  GdlDockObject    *child,
                                                  GdlDockPlacement  new_position,
                                                  GValue           *other_data);

void           gdl_dock_object_present           (GdlDockObject    *object,
                                                  GdlDockObject    *child);

gboolean       gdl_dock_object_child_placement   (GdlDockObject    *object,
                                                  GdlDockObject    *child,
                                                  GdlDockPlacement *placement);

/* other types */

/* this type derives from G_TYPE_STRING and is meant to be the basic
   type for serializing object parameters which are exported
   (i.e. those that are needed for layout rebuilding) */
#define GDL_TYPE_DOCK_PARAM   (gdl_dock_param_get_type ())

GType gdl_dock_param_get_type (void);

/* functions for setting/retrieving nick names for serializing GdlDockObject types */
const gchar          *gdl_dock_object_nick_from_type    (GType        type);
GType                 gdl_dock_object_type_from_nick    (const gchar *nick);
GType                 gdl_dock_object_set_type_for_nick (const gchar *nick,
                                                         GType        type);


/* helper macros */
#define GDL_TRACE_OBJECT(object, format, args...) \
    G_STMT_START {                            \
    g_log (G_LOG_DOMAIN,                      \
	   G_LOG_LEVEL_DEBUG,                 \
           "%s:%d (%s) %s [%p %d%s:%d]: " format, \
	   __FILE__,                          \
	   __LINE__,                          \
	   __PRETTY_FUNCTION__,               \
           G_OBJECT_TYPE_NAME (object), object, \
           G_OBJECT (object)->ref_count, \
           (GTK_IS_OBJECT (object) && g_object_is_floating (object)) ? "(float)" : "", \
           GDL_IS_DOCK_OBJECT (object) ? GDL_DOCK_OBJECT (object)->freeze_count : -1, \
	   ##args); } G_STMT_END                   
    


G_END_DECLS

#endif  /* __GDL_DOCK_OBJECT_H__ */

