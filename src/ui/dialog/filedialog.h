#ifndef __FILE_DIALOG_H__
#define __FILE_DIALOG_H__
/**
 * Defines the FileOpenDialog, FileSaveDialog, and FileExportDialog
 * and their supporting classes.
 *
 * Authors:
 *   Bob Jamison <rwjj@earthlink.net>
 *   Inkscape Guys
 *
 * Copyright (C) 2006 Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) 2004-2006, Inkscape Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm.h>
#include <vector>
#include <gtkmm.h>

namespace Inkscape {
namespace Extension {
class Extension;
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
    EXPORT_TYPES
    } FileDialogType;

/**
 * Used for returning the type selected in a SaveAs
 */
typedef enum {
    SVG_NAMESPACE,
    SVG_NAMESPACE_WITH_EXTENSIONS
    } FileDialogSelectionType;

/**
 * Architecture-specific data
 */
typedef struct FileOpenNativeData_def FileOpenNativeData;


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
                                  const Glib::ustring &title);


    /**
     * Destructor.
     * Perform any necessary cleanups.
     */
    virtual ~FileOpenDialog() {};

    /**
     * Show an OpenFile file selector.
     * @return the selected path if user selected one, else NULL
     */
    virtual bool show() =0;

    /**
     * Return the 'key' (filetype) of the selection, if any
     * @return a pointer to a string if successful (which must
     * be later freed with g_free(), else NULL.
     */
    virtual Inkscape::Extension::Extension * getSelectionType() = 0;

    virtual Glib::ustring getFilename () =0;

    virtual std::vector<Glib::ustring> getFilenames () = 0;

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
                                  const Glib::ustring &title,
                                  const Glib::ustring &default_key);


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

    virtual Glib::ustring getFilename () =0;

    /**
     * Change the window title.
     */
    virtual void change_title(const Glib::ustring& title) =0;

    /**
     * Change the default save path location.
     */
    virtual void change_path(const Glib::ustring& path) =0;

}; //FileSaveDialog




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
                                    const Glib::ustring &title,
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


/**
 * This class provides an implementation-independent API for
 * file "ExportToOCAL" dialogs.
 */
class FileExportToOCALDialog
{
public:

    /**
     * Constructor.  Do not call directly .   Use the factory.
     * @param fileTypes one of FileDialogTypes
     * @param title the title of the dialog
     * @param key a list of file types from which the user can select
     */
    FileExportToOCALDialog ()
        {};

    /**
     * Factory.
     * @param fileTypes one of FileDialogTypes
     * @param title the title of the dialog
     * @param key a list of file types from which the user can select
     */
    static FileExportToOCALDialog *create(Gtk::Window& parentWindow, 
                                     FileDialogType fileTypes,
                                     const Glib::ustring &title,
                                     const Glib::ustring &default_key);


    /**
     * Destructor.
     * Perform any necessary cleanups.
     */
    virtual ~FileExportToOCALDialog() {};


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

    virtual Glib::ustring getFilename () =0;

    /**
     * Change the window title.
     */
    virtual void change_title(const Glib::ustring& title) =0;


}; //FileExportToOCAL



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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
