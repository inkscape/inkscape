/*  -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * 
 * This file is part of the GNOME Devtool Libraries
 * 
 * Copyright (C) 1999-2000 Dave Camp <dave@helixcode.com>
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

/* Miscellaneous GDL tools/macros */

#ifndef __GDL_TOOLS_H__
#define __GDL_TOOLS_H__

#include <glib.h>
#include <gtk/gtkwidget.h>

/* FIXME: Toggle this */

G_BEGIN_DECLS

#define DO_GDL_TRACE

#ifdef DO_GDL_TRACE

#ifdef __GNUC__

#define GDL_TRACE()  G_STMT_START {             \
    g_log (G_LOG_DOMAIN,                      \
	   G_LOG_LEVEL_DEBUG,                 \
	   "file %s: line %d (%s)",           \
	   __FILE__,                          \
	   __LINE__,                          \
	   __PRETTY_FUNCTION__); } G_STMT_END 

#define GDL_TRACE_EXTRA(format, args...) G_STMT_START {     \
    g_log (G_LOG_DOMAIN,                      \
	   G_LOG_LEVEL_DEBUG,                 \
	   "file %s: line %d (%s): "format,   \
	   __FILE__,                          \
	   __LINE__,                          \
	   __PRETTY_FUNCTION__,               \
	   ##args); } G_STMT_END                   
    
#else /* __GNUC__ */

#define GDL_TRACE()  G_STMT_START {             \
    g_log (G_LOG_DOMAIN,                      \
	   G_LOG_LEVEL_DEBUG,                 \
	   "file %s: line %d",                \
	   __FILE__,                          \
	   __LINE__); } G_STMT_END 

#define GDL_TRACE_EXTRA(format, args...) G_STMT_START {     \
    g_log (G_LOG_DOMAIN,                      \
	   G_LOG_LEVEL_DEBUG,                 \
	   "file %s: line %d: "format,        \
	   __FILE__,                          \
	   __LINE__,                          \
	   ##args); } G_STMT_END                       
#endif /* __GNUC__ */

#else /* DO_GDL_TRACE */

#define GDL_TRACE()
#define GDL_TRACE_EXTRA()

#endif /* DO_GDL_TRACE */

/**
 * Class boilerplate and base class call macros copied from
 * bonobo/bonobo-macros.h.  Original copyright follows.
 *
 * Author:
 *   Darin Adler <darin@bentspoon.com>
 *
 * Copyright 2001 Ben Tea Spoons, Inc.
 */

/* Macros for defining classes.  Ideas taken from Nautilus and GOB. */

/* Define the boilerplate type stuff to reduce typos and code size.  Defines
 * the get_type method and the parent_class static variable. */

#define GDL_BOILERPLATE(type, type_as_function, corba_type,                     \
			parent_type, parent_type_macro,                         \
			register_type_macro)                                    \
static void type_as_function ## _class_init    (type ## Class *klass);          \
static void type_as_function ## _instance_init (type          *object);         \
static parent_type ## Class *parent_class = NULL;                               \
static void                                                                     \
type_as_function ## _class_init_trampoline (gpointer klass,                     \
					    gpointer data)                      \
{                                                                               \
	parent_class = (parent_type ## Class *)g_type_class_ref (               \
		parent_type_macro);                                             \
	type_as_function ## _class_init ((type ## Class *)klass);               \
}                                                                               \
GType                                                                           \
type_as_function ## _get_type (void)                                            \
{                                                                               \
	static GType object_type = 0;                                           \
	if (object_type == 0) {                                                 \
		static const GTypeInfo object_info = {                          \
		    sizeof (type ## Class),                                     \
		    NULL,		/* base_init */                         \
		    NULL,		/* base_finalize */                     \
		    type_as_function ## _class_init_trampoline,                 \
		    NULL,		/* class_finalize */                    \
		    NULL,               /* class_data */                        \
		    sizeof (type),                                              \
		    0,                  /* n_preallocs */                       \
		    (GInstanceInitFunc) type_as_function ## _instance_init      \
		};                                                              \
		object_type = register_type_macro                               \
			(type, type_as_function, corba_type,                    \
			 parent_type, parent_type_macro);                       \
	}                                                                       \
	return object_type;                                                     \
}

/* Just call the parent handler.  This assumes that there is a variable
 * named parent_class that points to the (duh!) parent class.  Note that
 * this macro is not to be used with things that return something, use
 * the _WITH_DEFAULT version for that */
#define GDL_CALL_PARENT(parent_class_cast, name, args)          \
	((parent_class_cast(parent_class)->name != NULL) ?      \
	 parent_class_cast(parent_class)->name args : (void)0)

#define GDL_CALL_PARENT_GBOOLEAN(parent_class_cast, name, args)          \
       ((parent_class_cast(parent_class)->name != NULL) ?      \
        parent_class_cast(parent_class)->name args : (gboolean)0)


/* Same as above, but in case there is no implementation, it evaluates
 * to def_return */
#define GDL_CALL_PARENT_WITH_DEFAULT(parent_class_cast,                 \
				     name, args, def_return)            \
	((parent_class_cast(parent_class)->name != NULL) ?              \
	 parent_class_cast(parent_class)->name args : def_return)

/* Define the boilerplate type stuff to reduce typos and code size.  Defines
 * the get_type method and the parent_class static variable. */
#define GDL_CLASS_BOILERPLATE(type, type_as_function,           \
				parent_type, parent_type_macro) \
	GDL_BOILERPLATE(type, type_as_function, type,           \
			  parent_type, parent_type_macro,       \
			  GDL_REGISTER_TYPE)
#define GDL_REGISTER_TYPE(type, type_as_function, corba_type,                   \
			  parent_type, parent_type_macro)                       \
	g_type_register_static (parent_type_macro, #type, &object_info, 0)


#define GDL_CALL_VIRTUAL(object, get_class_cast, method, args) \
    (get_class_cast (object)->method ? (* get_class_cast (object)->method) args : (void)0)
#define GDL_CALL_VIRTUAL_WITH_DEFAULT(object, get_class_cast, method, args, default) \
    (get_class_cast (object)->method ? (* get_class_cast (object)->method) args : default)

/* GdlPixmap structure and method have been copied from Evolution. */
typedef struct _GdlPixmap {
	const char *path;
	const char *fname;
	char       *pixbuf;
} GdlPixmap;

#define GDL_PIXMAP(path,fname)	{ (path), (fname), NULL }
#define GDL_PIXMAP_END		{ NULL, NULL, NULL }

G_END_DECLS

#endif

