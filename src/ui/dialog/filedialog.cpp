/**
 * @file
 * Implementation of the file dialog interfaces defined in filedialog.h.
 */
/* Authors:
 *   Bob Jamison
 *   Joel Holdsworth
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2004-2007 Bob Jamison
 * Copyright (C) 2006 Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) 2007-2008 Joel Holdsworth
 * Copyright (C) 2004-2008 The Inkscape Organization
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "filedialogimpl-win32.h"
#include "filedialogimpl-gtkmm.h"
#include "filedialog.h"

#include "inkgc/gc-core.h"
#include "ui/dialog-events.h"
#include "extension/output.h"
#include "preferences.h"

#include <glibmm/convert.h>

namespace Inkscape
{
namespace UI
{
namespace Dialog
{

/*#########################################################################
### U T I L I T Y
#########################################################################*/

bool hasSuffix(const Glib::ustring &str, const Glib::ustring &ext)
{
    int strLen = str.length();
    int extLen = ext.length();
    if (extLen > strLen)
        return false;
    int strpos = strLen-1;
    for (int extpos = extLen-1 ; extpos>=0 ; extpos--, strpos--)
        {
        Glib::ustring::value_type ch = str[strpos];
        if (ch != ext[extpos])
            {
            if ( ((ch & 0xff80) != 0) ||
                 static_cast<Glib::ustring::value_type>( g_ascii_tolower( static_cast<gchar>(0x07f & ch) ) ) != ext[extpos] )
                {
                return false;
                }
            }
        }
    return true;
}

bool isValidImageFile(const Glib::ustring &fileName)
{
    std::vector<Gdk::PixbufFormat>formats = Gdk::Pixbuf::get_formats();
    for (unsigned int i=0; i<formats.size(); i++)
        {
        Gdk::PixbufFormat format = formats[i];
        std::vector<Glib::ustring>extensions = format.get_extensions();
        for (unsigned int j=0; j<extensions.size(); j++)
            {
            Glib::ustring ext = extensions[j];
            if (hasSuffix(fileName, ext))
                return true;
            }
        }
    return false;
}

/*#########################################################################
### F I L E    O P E N
#########################################################################*/

/**
 * Public factory.  Called by file.cpp, among others.
 */
FileOpenDialog *FileOpenDialog::create(Gtk::Window &parentWindow,
		                               const Glib::ustring &path,
                                       FileDialogType fileTypes,
                                       const char *title)
{
#ifdef WIN32
    FileOpenDialog *dialog = NULL;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool( "/options/desktopintegration/value")) {
        dialog = new FileOpenDialogImplWin32(parentWindow, path, fileTypes, title);
    } else {
        dialog = new FileOpenDialogImplGtk(parentWindow, path, fileTypes, title);
    }
#else
    FileOpenDialog *dialog = new FileOpenDialogImplGtk(parentWindow, path, fileTypes, title);
#endif

	return dialog;
}

Glib::ustring FileOpenDialog::getFilename()
{
    return myFilename;
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
                                       const char *title,
                                       const Glib::ustring &default_key,
                                       const gchar *docTitle,
                                       const Inkscape::Extension::FileSaveMethod save_method)
{
#ifdef WIN32
    FileSaveDialog *dialog = NULL;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool( "/options/desktopintegration/value")) {
        dialog = new FileSaveDialogImplWin32(parentWindow, path, fileTypes, title, default_key, docTitle, save_method);
    } else {
        dialog = new FileSaveDialogImplGtk(parentWindow, path, fileTypes, title, default_key, docTitle, save_method);
    }
#else
    FileSaveDialog *dialog = new FileSaveDialogImplGtk(parentWindow, path, fileTypes, title, default_key, docTitle, save_method);
#endif
    return dialog;
}

Glib::ustring FileSaveDialog::getFilename()
{
    return myFilename;
}

Glib::ustring FileSaveDialog::getDocTitle()
{
	return myDocTitle;
}

//void FileSaveDialog::change_path(const Glib::ustring& path)
//{
//	myFilename = path;
//}

void FileSaveDialog::appendExtension(Glib::ustring& path, Inkscape::Extension::Output* outputExtension)
{
	if (!outputExtension)
		return;

	try {
		bool appendExtension = true;
		Glib::ustring utf8Name = Glib::filename_to_utf8( path );
		Glib::ustring::size_type pos = utf8Name.rfind('.');
		if ( pos != Glib::ustring::npos ) {
			Glib::ustring trail = utf8Name.substr( pos );
			Glib::ustring foldedTrail = trail.casefold();
			if ( (trail == ".")
				 | (foldedTrail != Glib::ustring( outputExtension->get_extension() ).casefold()
					&& ( knownExtensions.find(foldedTrail) != knownExtensions.end() ) ) ) {
				utf8Name = utf8Name.erase( pos );
			} else {
				appendExtension = false;
			}
		}

		if (appendExtension) {
			utf8Name = utf8Name + outputExtension->get_extension();
			myFilename = Glib::filename_from_utf8( utf8Name );
		}
	} catch ( Glib::ConvertError& e ) {
		// ignore
	}
}

//########################################################################
//# F I L E     E X P O R T
//########################################################################

#ifdef NEW_EXPORT_DIALOG

/**
 * Public factory method.  Used in file.cpp
 */
FileExportDialog *FileExportDialog::create(Gtk::Window& parentWindow,
                                           const Glib::ustring &path,
                                           FileDialogType fileTypes,
                                           const char *title,
                                           const Glib::ustring &default_key)
{
    FileExportDialog *dialog = new FileExportDialogImpl(parentWindow, path, fileTypes, title, default_key);
    return dialog;
}

#endif // NEW_EXPORT_DIALOG


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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
