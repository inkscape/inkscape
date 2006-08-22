#ifndef __SP_FILE_H__
#define __SP_FILE_H__

/*
 * File/Print operations
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Chema Celorio <chema@celorio.com>
 *
 * Copyright (C) 1999-2002 Authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtkmm.h>
#include <glib/gslist.h>
#include <gtk/gtkwidget.h>

#include "extension/extension-forward.h"

struct SPDesktop;
struct SPDocument;

namespace Inkscape {
    namespace Extension {
        struct Extension;
    }
}

/*######################
## N E W
######################*/

/**
 * Creates a new Inkscape document and window.
 * Return value is a pointer to the newly created desktop.
 */
SPDesktop* sp_file_new (const Glib::ustring &templ);
SPDesktop* sp_file_new_default (void);

/*######################
## D E L E T E
######################*/

/**
 * Close the document/view
 */
void sp_file_exit (void);

/*######################
## O P E N
######################*/

/**
 * Opens a new file and window from the given URI
 */
bool sp_file_open(
    const Glib::ustring &uri,
    Inkscape::Extension::Extension *key,
    bool add_to_recent = true,
    bool replace_empty = true
    );

/**
 * Displays a file open dialog. Calls sp_file_open on
 * an OK.
 */
void sp_file_open_dialog (gpointer object, gpointer data);

/**
 * Reverts file to disk-copy on "YES"
 */
void sp_file_revert_dialog ();

/*######################
## S A V E
######################*/

/**
 *
 */
bool sp_file_save (gpointer object, gpointer data);

/**
 *  Saves the given document.  Displays a file select dialog
 *  to choose the new name.
 */
bool sp_file_save_as (gpointer object, gpointer data);

/**
 *  Saves a copy of the given document.  Displays a file select dialog
 *  to choose a name for the copy.
 */
bool sp_file_save_a_copy (gpointer object, gpointer data);


/**
 *  Saves the given document.  Displays a file select dialog
 *  if needed.
 */
bool sp_file_save_document (SPDocument *document);

/* Do the saveas dialog with a document as the parameter */
bool sp_file_save_dialog (SPDocument *doc, bool bAsCopy = FALSE);


/*######################
## I M P O R T
######################*/

/**
 * Displays a file selector dialog, to allow the
 * user to import data into the current document.
 */
void sp_file_import (GtkWidget * widget);

/**
 * Imports a resource
 */
void file_import(SPDocument *in_doc, const Glib::ustring &uri,
                 Inkscape::Extension::Extension *key);

/*######################
## E X P O R T
######################*/

/**
 * Displays a FileExportDialog for the user, with an
 * additional type selection, to allow the user to export
 * the a document as a given type.
 */
bool sp_file_export_dialog (void *widget);



/*######################
## P R I N T
######################*/

/* These functions are redundant now, but
would be useful as instance methods
*/

/**
 *
 */
void sp_file_print (void);

/**
 *
 */
void sp_file_print_direct (void);

/**
 *
 */
void sp_file_print_preview (gpointer object, gpointer data);

/*#####################
## U T I L I T Y
#####################*/

/**
 * clean unused defs out of file
 */
void sp_file_vacuum ();


namespace Inkscape {
namespace IO {

void fixupHrefs( SPDocument *doc, const gchar *uri, gboolean spns );

}
}


#endif
