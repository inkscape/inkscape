/** @file
 * @brief Virtual base definitions for native file dialogs
 */
/* Authors:
 *   Bob Jamison <rwjj@earthlink.net>
 *   Joel Holdsworth
 *   Inkscape Guys
 *
 * Copyright (C) 2006 Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) 2007-2008 Joel Holdsworth
 * Copyright (C) 2004-2008, Inkscape Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __FILE_DIALOG_H__
#define __FILE_DIALOG_H__

#include <vector>
#include <set>

#include "extension/system.h"

#include <glibmm/ustring.h>

class SPDocument;

namespace Inkscape {
namespace Extension {
class Extension;
class Output;
}
}

namespace Inkscape
{
namespace UI
{
namespace Dialog
{

/**
 * Used for setting filters and options, and
 * reading them back from user selections.
 */
typedef enum {
    SVG_TYPES,
    IMPORT_TYPES,
    EXPORT_TYPES,
    EXE_TYPES,
    SWATCH_TYPES,
    CUSTOM_TYPE
    } FileDialogType;

/**
 * Used for returning the type selected in a SaveAs
 */
typedef enum {
    SVG_NAMESPACE,
    SVG_NAMESPACE_WITH_EXTENSIONS
    } FileDialogSelectionType;


/**
 * Return true if the string ends with the given suffix
 */
bool hasSuffix(const Glib::ustring &str, const Glib::ustring &ext);

/**
 * Return true if the image is loadable by Gdk, else false
 */
bool isValidImageFile(const Glib::ustring &fileName);

/**
 * This class provides an implementation-independent API for
 * file "Open" dialogs.  Using a standard interface obviates the need
 * for ugly #ifdefs in file open code
 */
class FileOpenDialog
{
public:


    /**
     * Constructor ..  do not call directly
     * @param path the directory where to start searching
     * @param fileTypes one of FileDialogTypes
     * @param title the title of the dialog
     */
    FileOpenDialog()
        {};

    /**
     * Factory.
     * @param path the directory where to start searching
     * @param fileTypes one of FileDialogTypes
     * @param title the title of the dialog
     */
    static FileOpenDialog *create(Gtk::Window& parentWindow,
                                  const Glib::ustring &path,
                                  FileDialogType fileTypes,
                                  const char *title);


    /**
     * Destructor.
     * Perform any necessary cleanups.
     */
    virtual ~FileOpenDialog() {};

    /**
     * Show an OpenFile file selector.
     * @return the selected path if user selected one, else NULL
     */
    virtual bool show() = 0;

    /**
     * Return the 'key' (filetype) of the selection, if any
     * @return a pointer to a string if successful (which must
     * be later freed with g_free(), else NULL.
     */
    virtual Inkscape::Extension::Extension * getSelectionType() = 0;

    Glib::ustring getFilename();

    virtual std::vector<Glib::ustring> getFilenames() = 0;

    virtual Glib::ustring getCurrentDirectory() = 0;

    virtual void addFilterMenu(Glib::ustring name, Glib::ustring pattern) = 0;

protected:
    /**
     * Filename that was given
     */
    Glib::ustring myFilename;

}; //FileOpenDialog






/**
 * This class provides an implementation-independent API for
 * file "Save" dialogs.
 */
class FileSaveDialog
{
public:

    /**
     * Constructor.  Do not call directly .   Use the factory.
     * @param path the directory where to start searching
     * @param fileTypes one of FileDialogTypes
     * @param title the title of the dialog
     * @param key a list of file types from which the user can select
     */
    FileSaveDialog ()
        {};

    /**
     * Factory.
     * @param path the directory where to start searching
     * @param fileTypes one of FileDialogTypes
     * @param title the title of the dialog
     * @param key a list of file types from which the user can select
     */
    static FileSaveDialog *create(Gtk::Window& parentWindow,
                                  const Glib::ustring &path,
                                  FileDialogType fileTypes,
                                  const char *title,
                                  const Glib::ustring &default_key,
                                  const gchar *docTitle,
                                  const Inkscape::Extension::FileSaveMethod save_method);


    /**
     * Destructor.
     * Perform any necessary cleanups.
     */
    virtual ~FileSaveDialog() {};


    /**
     * Show an SaveAs file selector.
     * @return the selected path if user selected one, else NULL
     */
    virtual bool show() =0;

    /**
     * Return the 'key' (filetype) of the selection, if any
     * @return a pointer to a string if successful (which must
     * be later freed with g_free(), else NULL.
     */
    virtual Inkscape::Extension::Extension * getSelectionType() = 0;

    virtual void setSelectionType( Inkscape::Extension::Extension * key ) = 0;

    /**
     * Get the file name chosen by the user.   Valid after an [OK]
     */
    Glib::ustring getFilename ();

    /**
     * Get the document title chosen by the user.   Valid after an [OK]
     */
    Glib::ustring getDocTitle ();

    virtual Glib::ustring getCurrentDirectory() = 0;

    virtual void addFileType(Glib::ustring name, Glib::ustring pattern) = 0;

protected:

    /**
     * Filename that was given
     */
    Glib::ustring myFilename;

    /**
     * Doc Title that was given
     */
    Glib::ustring myDocTitle;

    /**
     * List of known file extensions.
     */
    std::set<Glib::ustring> knownExtensions;


    void appendExtension(Glib::ustring& path, Inkscape::Extension::Output* outputExtension);

}; //FileSaveDialog


//#define NEW_EXPORT_DIALOG

#ifdef NEW_EXPORT_DIALOG

/**
 * This class provides an implementation-independent API for
 * file "Export" dialogs.  Saving as these types will not affect
 * the original file.
 */
class FileExportDialog
{
public:

    typedef enum
        {
        SCOPE_DOCUMENT,
        SCOPE_PAGE,
        SCOPE_SELECTION,
        SCOPE_CUSTOM
        } ScopeType;

    /**
     * Constructor.  Do not call directly .   Use the factory.
     * @param path the directory where to start searching
     * @param fileTypes one of FileDialogTypes
     * @param title the title of the dialog
     * @param key a list of file types from which the user can select
     */
    FileExportDialog()
        {}

    /**
     * Factory.
     * @param path the directory where to start searching
     * @param fileTypes one of FileDialogTypes
     * @param title the title of the dialog
     * @param key a list of file types from which the user can select
     */
    static FileExportDialog *create(Gtk::Window& parentWindow,
                                    const Glib::ustring &path,
                                    FileDialogType fileTypes,
                                    const char *title,
                                    const Glib::ustring &default_key);


    /**
     * Destructor.
     * Perform any necessary cleanups.
     */
    virtual ~FileExportDialog () {};


    /**
     * Show an SaveAs file selector.
     * @return the selected path if user selected one, else NULL
     */
    virtual bool show() =0;

    /**
     * Return the 'key' (filetype) of the selection, if any
     * @return a pointer to a string if successful (which must
     * be later freed with g_free(), else NULL.
     */
    virtual Inkscape::Extension::Extension * getSelectionType() = 0;

    /**
     * Return the selected filename, if any.  If not, return ""
     */
    virtual Glib::ustring getFilename () =0;

    /**
     * Return the scope of the export.  One of the enumerated types
     * in ScopeType
     */
    virtual ScopeType getScope() = 0;

    /**
     * Return left side of the exported region
     */
    virtual double getSourceX() = 0;

    /**
     * Return the top of the exported region
     */
    virtual double getSourceY() = 0;

    /**
     * Return the width of the exported region
     */
    virtual double getSourceWidth() = 0;

    /**
     * Return the height of the exported region
     */
    virtual double getSourceHeight() = 0;

    /**
     * Return the units of the coordinates of exported region
     */
    virtual Glib::ustring getSourceUnits() = 0;

    /**
     * Return the width of the destination document
     */
    virtual double getDestinationWidth() = 0;

    /**
     * Return the height of the destination document
     */
    virtual double getDestinationHeight() = 0;

    /**
     * Return the height of the exported region
     */
    virtual Glib::ustring getDestinationUnits() = 0;

    /**
     * Return the destination DPI image resulution, if bitmap
     */
    virtual double getDestinationDPI() = 0;

    /**
     * Return whether we should use Cairo for rendering
     */
    virtual bool getUseCairo() = 0;

    /**
     * Return whether we should use antialiasing
     */
    virtual bool getUseAntialias() = 0;

    /**
     * Return the background color for exporting
     */
    virtual unsigned long getBackground() = 0;



}; //FileExportDialog

#endif // NEW_EXPORT_DIALOG


} //namespace Dialog
} //namespace UI
} //namespace Inkscape

#endif /* __FILE_DIALOG_H__ */

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
