#ifndef SEEN_SP_FILE_H
#define SEEN_SP_FILE_H

/*
 * File/Print operations
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Chema Celorio <chema@celorio.com>
 *
 * Copyright (C) 2006 Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) 2001-2002 Ximian, Inc.
 * Copyright (C) 1999-2002 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/ustring.h>
#include <string>
#include "extension/system.h"

class SPDesktop;
class SPDocument;
class SPObject;

namespace Inkscape {
    namespace Extension {
        class Extension;
    }
}

namespace Gtk {
class Window;
}

// Get the name of the default template uri
Glib::ustring sp_file_default_template_uri();

/*######################
## N E W
######################*/

/**
 * Creates a new Inkscape document and window.
 * Return value is a pointer to the newly created desktop.
 */
SPDesktop* sp_file_new (const std::string &templ);
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
void sp_file_open_dialog (Gtk::Window &parentWindow, void* object, void* data);

/**
 * Reverts file to disk-copy on "YES"
 */
void sp_file_revert_dialog ();

/*######################
## S A V E
######################*/

/*
 * Added to make only the remote savings.
 */

bool file_save_remote(SPDocument *doc, const Glib::ustring &uri,
		 Inkscape::Extension::Extension *key, bool saveas, bool official);

/**
 *
 */
bool sp_file_save (Gtk::Window &parentWindow, void* object, void* data);

/**
 *  Saves the given document.  Displays a file select dialog
 *  to choose the new name.
 */
bool sp_file_save_as (Gtk::Window &parentWindow, void* object, void* data);

/**
 *  Saves a copy of the given document.  Displays a file select dialog
 *  to choose a name for the copy.
 */
bool sp_file_save_a_copy (Gtk::Window &parentWindow, void* object, void* data);


/**
 *  Saves the given document.  Displays a file select dialog
 *  if needed.
 */
bool sp_file_save_document (Gtk::Window &parentWindow, SPDocument *document);

/* Do the saveas dialog with a document as the parameter */
bool sp_file_save_dialog (Gtk::Window &parentWindow, SPDocument *doc, Inkscape::Extension::FileSaveMethod save_method);


/*######################
## I M P O R T
######################*/

void sp_import_document(SPDesktop *desktop, SPDocument *clipdoc, bool in_place);

/**
 * Displays a file selector dialog, to allow the
 * user to import data into the current document.
 */
void sp_file_import (Gtk::Window &parentWindow);

/**
 * Imports a resource
 */
SPObject* file_import(SPDocument *in_doc, const Glib::ustring &uri,
                 Inkscape::Extension::Extension *key);

/*######################
## E X P O R T
######################*/

/**
 * Displays a FileExportDialog for the user, with an
 * additional type selection, to allow the user to export
 * the a document as a given type.
 */
//bool sp_file_export_dialog (Gtk::Window &parentWindow);


/*######################
## E X P O R T  T O  O C A L
######################*/

/**
 * Export the current document to OCAL
 */
//void sp_file_export_to_ocal (Gtk::Window &parentWindow );


/**
 * Export the current document to OCAL
 */
//bool sp_file_export_to_ocal_dialog (void *widget);


/*######################
## I M P O R T  F R O M  O C A L
######################*/

/**
 * Import a document from OCAL
 */
void on_import_from_ocal_response(Glib::ustring filename);

/**
 * Import a document from OCAL
 */
void sp_file_import_from_ocal (Gtk::Window &parentWindow );


/*######################
## P R I N T
######################*/

/* These functions are redundant now, but
would be useful as instance methods
*/

/**
 *
 */
void sp_file_print (Gtk::Window& parentWindow);

/*#####################
## U T I L I T Y
#####################*/

/**
 * clean unused defs out of file
 */
void sp_file_vacuum (SPDocument *doc);


#endif // SEEN_SP_FILE_H


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vi: set autoindent shiftwidth=4 tabstop=8 filetype=cpp expandtab softtabstop=4 encoding=utf-8 textwidth=99 :
