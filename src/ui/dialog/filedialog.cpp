/**
 * Implementation of the file dialog interfaces defined in filedialog.h
 *
 * Authors:
 *   Bob Jamison
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2004-2007 Bob Jamison
 * Copyright (C) 2006 Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) 2004-2007 The Inkscape Organization
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "filedialog.h"
#include "filedialogimpl-gtkmm.h"

#include "gc-core.h"
#include <dialogs/dialog-events.h>

namespace Inkscape
{
namespace UI
{
namespace Dialog
{

/*#########################################################################
### F I L E    O P E N
#########################################################################*/

/**
 * Public factory.  Called by file.cpp, among others.
 */
FileOpenDialog *FileOpenDialog::create(Gtk::Window &parentWindow,
		                               const Glib::ustring &path,
                                       FileDialogType fileTypes,
                                       const Glib::ustring &title)
{
    FileOpenDialog *dialog = new FileOpenDialogImplGtk(parentWindow, path, fileTypes, title);
    return dialog;
}

//########################################################################
//# F I L E    S A V E
//########################################################################

/**
 * Public factory method.  Used in file.cpp
 */
FileSaveDialog *FileSaveDialog::create(Gtk::Window& parentWindow, 
									   const Glib::ustring &path,
                                       FileDialogType fileTypes,
                                       const Glib::ustring &title,
                                       const Glib::ustring &default_key)
{
    FileSaveDialog *dialog = new FileSaveDialogImplGtk(parentWindow, path, fileTypes, title, default_key);
    return dialog;
}

//########################################################################
//# F I L E     E X P O R T
//########################################################################

/**
 * Public factory method.  Used in file.cpp
 */
 FileExportDialog *FileExportDialog::create(Gtk::Window& parentWindow, 
										   const Glib::ustring &path,
                                           FileDialogType fileTypes,
                                           const Glib::ustring &title,
                                           const Glib::ustring &default_key)
{
    FileExportDialog *dialog = new FileExportDialogImpl(parentWindow, path, fileTypes, title, default_key);
    return dialog;
}


//########################################################################
//# F I L E    E X P O R T   T O   O C A L
//########################################################################


/**
 * Public factory method.  Used in file.cpp
 */

 FileExportToOCALDialog *FileExportToOCALDialog::create(Gtk::Window& parentWindow, 
                                           FileDialogType fileTypes,
                                           const Glib::ustring &title,
                                           const Glib::ustring &default_key)
{
    FileExportToOCALDialog *dialog = new FileExportToOCALDialogImpl(parentWindow, fileTypes, title, default_key);
    return dialog;
}

//#########################################################################
//### F I L E    I M P O R T  F R O M  O C A L
//#########################################################################

/**
 * Public factory.  Called by file.cpp.
 */
FileImportFromOCALDialog *FileImportFromOCALDialog::create(Gtk::Window &parentWindow,
		                       const Glib::ustring &path,
                                       FileDialogType fileTypes,
                                       const Glib::ustring &title)
{
    FileImportFromOCALDialog *dialog = new FileImportFromOCALDialogImplGtk(parentWindow, path, fileTypes, title);
    return dialog;
}


} //namespace Dialog
} //namespace UI
} //namespace Inkscape



/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
