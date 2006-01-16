/** \file
 * Plugin prototypes.
 *
 * This header describes which prototypes plugins should use when
 * creating their functions.  This header is also used by the internal
 * plugins code to guarantee consistency.
 *
 * Author:  Ted Gould <ted@gould.cx>
 * Copyright (c) 2004-2005
 *
 * This code is licensed under the GNU GPL.  See COPYING for details.
 */

#ifndef __INKSCAPE_EXTENSION_IMPLEMENTATION_PLUGIN_LINK_H__
#define __INKSCAPE_EXTENSION_IMPLEMENTATION_PLUGIN_LINK_H__

#include <gtk/gtkdialog.h>
#include <gtkmm/widget.h>

/** \todo  This needs to go away eventually. */
#include "document.h"

/** \brief  A simple typedef to make it so that inkscape_extension can
            be used before I figure out what makes sense here */
typedef void inkscape_extension;
/** \brief  The C prototype of a load function.  */
typedef int (*inkscape_plugin_load)(inkscape_extension * in_ext);
/** \brief  The C prototype of an unload function.  */
typedef void (*inkscape_plugin_unload)(inkscape_extension * in_ext);
/** \brief  The C prototype of an open function.  */
typedef SPDocument *(*inkscape_plugin_open)(inkscape_extension * in_ext, const gchar * filename);
/** \brief  The C prototype of an input prefs function.  */
typedef Gtk::Widget * (*inkscape_plugin_prefs_input)(inkscape_extension * in_ext, gchar const * filename);
/** \brief  The C prototype of an effect function.  */
typedef void (*inkscape_plugin_effect)(inkscape_extension * in_ext, Inkscape::UI::View::View * view);
/** \brief  The C prototype of an effect prefs function.  */
typedef Gtk::Widget * (*inkscape_plugin_prefs_effect)(inkscape_extension * in_ext, Inkscape::UI::View::View * view);

/** \brief  The name of the symbol for the plugin.  Should match
            \c INKSCAPE_PLUGIN_NAME_STR (minus the quotes). */
#define INKSCAPE_PLUGIN_NAME     inkscape_plugin_table
/** \brief  The name of the table to define the plugin as a string.  This
            should be the same as \c INKSCAPE_PLUGIN_NAME but with quotes. */
#define INKSCAPE_PLUGIN_NAME_STR "inkscape_plugin_table"
/** \brief  The version of the plugin interface that is being used.  This
            should always be used in the version entry in the \c inkscape_plugin_function_table
			version entry.  This way compiled plugins can be detected. */
#define INKSCAPE_PLUGIN_VERSION  0

/** \brief  A structure containing all the functions that should be called
            to make the plugin work. */
typedef struct {
	int version;                             /**< The interface version used.  Should
											      always be \c INKSCAPE_PLUGIN_VERSION. */
	inkscape_plugin_load load;               /**< Load function, called on first use */
	inkscape_plugin_unload unload;           /**< Unload function, called when Inkscape is
											      finished with the plugin */
	inkscape_plugin_open open;               /**< Open function, called to open a file
											      for Inkscape */
	inkscape_plugin_prefs_input prefs_input; /**< Input preferences function, called to get
											      further parameters for an input plugin. */
	inkscape_plugin_effect effect;           /**< Effect function, called to cause an effect
											      on a document. */
	inkscape_plugin_prefs_effect prefs_effect;/**< Effect preferences, on call could cause settings
											      on a document. */
} inkscape_plugin_function_table;

#endif /* __INKSCAPE_EXTENSION_IMPLEMENTATION_PLUGIN_LINK_H__ */
