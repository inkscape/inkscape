#ifndef __FILE_DIALOG_H__
#define __FILE_DIALOG_H__

/** \file Defines classes FileOpenDialog, FileSaveDialog,
 *    and enums FileDialogType, FileDialogSelectionType. */

#include <glib/gtypes.h>
#include <glibmm/slisthandle.h>

namespace Inkscape {

namespace Extension {
class Extension;
}

namespace UI {
namespace Dialog {


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
    static FileOpenDialog *create(const Glib::ustring &path,
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

    virtual gchar * getFilename () =0;

    virtual Glib::SListHandle<Glib::ustring> getFilenames () = 0;

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
    static FileSaveDialog *create(const Glib::ustring &path,
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

    virtual gchar * getFilename () =0;


}; //FileSaveDialog




/**
 * This class provides an implementation-independent API for
 * file "Export" dialogs.  Saving as these types will not affect
 * the original file.
 */
class FileExportDialog
{
public:

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
    static FileExportDialog *create(const Glib::ustring &path,
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

    virtual gchar * getFilename () =0;


}; //FileSaveDialog


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
