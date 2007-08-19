#ifndef __OCAL_DIALOG_H__
#define __OCAL_DIALOG_H__
/**
 * Defines the FileExportToOCALDialog, FileImportFromOCALDialog and 
 * FileExportToOCALPasswordDialog and their supporting classes.
 *
 * Authors:
 *   Bruno Dilly <bruno.dilly@gmail.com>
 *   Inkscape Guys
 *
 * Copyright (C) 2007 Bruno Dilly <bruno.dilly@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm.h>
#include <vector>
#include <gtkmm.h>
#include "filedialogimpl-gtkmm.h"

//General includes
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <set>
#include <libxml/parser.h>
#include <libxml/tree.h>


//Gtk includes
#include <glibmm/i18n.h>
#include <glib/gstdio.h>

//Temporary ugly hack
//Remove this after the get_filter() calls in
//show() on both classes are fixed
#include <gtk/gtkfilechooser.h>

//Another hack
#include <gtk/gtkentry.h>
#include <gtk/gtkexpander.h>
#ifdef WITH_GNOME_VFS
#include <libgnomevfs/gnome-vfs-init.h>  // gnome_vfs_initialized
#include<libgnomevfs/gnome-vfs.h>
#endif

//Inkscape includes
#include "prefs-utils.h"
#include <extension/input.h>
#include <extension/output.h>
#include <extension/db.h>
#include "inkscape.h"
#include "svg-view-widget.h"
#include "gc-core.h"

//For export dialog
#include "ui/widget/scalar-unit.h"


namespace Inkscape
{
namespace UI 
{   
namespace Dialog
{   
    

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

/**
 * This class provides an implementation-independent API for
 * file "ExportToOCALPassword" dialogs.
 */
class FileExportToOCALPasswordDialog
{
public:

    /**
     * Constructor.  Do not call directly .   Use the factory.
     * @param title the title of the dialog
    */
    FileExportToOCALPasswordDialog ()
        {};

    /**
     * Factory.
     * @param title the title of the dialog
     */
    static FileExportToOCALPasswordDialog *create(Gtk::Window& parentWindow, 
                                     const Glib::ustring &title);
                                     

    /**
     * Destructor.
     * Perform any necessary cleanups.
     */
    virtual ~FileExportToOCALPasswordDialog() {};


    /**
     * Show 2 entry to input username and password.
     */
    virtual bool show() =0;

    virtual Glib::ustring getUsername () =0;
    virtual Glib::ustring getPassword () =0;

    /**
     * Change the window title.
     */
    virtual void change_title(const Glib::ustring& title) =0;


}; //FileExportToOCALPassword



/**
 * This class provides an implementation-independent API for
 * file "ImportFromOCAL" dialogs. 
 */
class FileImportFromOCALDialog
{
public:


    /**
     * Constructor ..  do not call directly
     * @param path the directory where to start searching
     * @param fileTypes one of FileDialogTypes
     * @param title the title of the dialog
     */
    FileImportFromOCALDialog()
        {};

    /**
     * Factory.
     * @param path the directory where to start searching
     * @param fileTypes one of FileDialogTypes
     * @param title the title of the dialog
     */
    static FileImportFromOCALDialog *create(Gtk::Window& parentWindow, 
                                  const Glib::ustring &path,
                                  FileDialogType fileTypes,
                                  const Glib::ustring &title);


    /**
     * Destructor.
     * Perform any necessary cleanups.
     */
    virtual ~FileImportFromOCALDialog() {};

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

}; //FileImportFromOCALDialog


/*#########################################################################
### F I L E     D I A L O G    O C A L    B A S E    C L A S S
#########################################################################*/

/**
 * This class is the base implementation for export to OCAL.
 */
class FileDialogOCALBase : public Gtk::Dialog
{
public:

    /**
     *
     */
    FileDialogOCALBase(const Glib::ustring &title) : Gtk::Dialog(title,true)
    {}
    /*
     *
     */
    virtual ~FileDialogOCALBase()
    {}

protected:
    void cleanup( bool showConfirmed );

    //Glib::ustring preferenceBase;
    /**
     * What type of 'open' are we? (open, import, place, etc)
     */
    FileDialogType dialogType;
};




//########################################################################
//# F I L E    E X P O R T   T O   O C A L
//########################################################################



/**
 * Our implementation of the FileExportToOCALDialog interface.
 */
class FileExportToOCALDialogImpl : public FileExportToOCALDialog, public FileDialogOCALBase
{

public:
  FileExportToOCALDialogImpl(Gtk::Window& parentWindow, 
                             FileDialogType fileTypes,
			     const Glib::ustring &title,
			     const Glib::ustring &default_key);
    
    virtual ~FileExportToOCALDialogImpl();

    bool show();

    Inkscape::Extension::Extension *getSelectionType();
    virtual void setSelectionType( Inkscape::Extension::Extension * key );

    Glib::ustring getFilename();

    Glib::ustring myFilename;

    void change_title(const Glib::ustring& title);
    void updateNameAndExtension();

private:

    /**
     * Fix to allow the user to type the file name
     */
    Gtk::Entry *fileNameEntry;
    
    
    /**
     * Allow the specification of the output file type
     */
    Gtk::ComboBoxText fileTypeComboBox;


    /**
     *  Data mirror of the combo box
     */
    std::vector<FileType> fileTypes;

    // Child widgets
    Gtk::HBox childBox;
    Gtk::VBox checksBox;
    Gtk::HBox fileBox;

    Gtk::CheckButton fileTypeCheckbox;

    /**
     * Callback for user choose a fileType
     */
    void fileTypeChangedCallback();

    /**
     *  Create a filter menu for this type of dialog
     */
    void createFileTypeMenu();


    /**
     * The extension to use to write this file
     */
    Inkscape::Extension::Extension *extension;

    /**
     * Callback for user input into fileNameEntry
     */
    void fileNameEntryChangedCallback();

    /**
     * List of known file extensions.
     */
    std::set<Glib::ustring> knownExtensions;
};


//########################################################################
//# F I L E    E X P O R T   T O   O C A L   P A S S W O R D
//########################################################################


/**
 * Our implementation of the FileExportToOCALPasswordDialog interface.
 */
class FileExportToOCALPasswordDialogImpl : public FileExportToOCALPasswordDialog, public FileDialogOCALBase
{

public:
  FileExportToOCALPasswordDialogImpl(Gtk::Window& parentWindow, 
                                const Glib::ustring &title);
    
    virtual ~FileExportToOCALPasswordDialogImpl();

    bool show();

    Glib::ustring getUsername();
    Glib::ustring getPassword();

    Glib::ustring myUsername;
    Glib::ustring myPassword;

    void change_title(const Glib::ustring& title);

private:

    /**
     * Fix to allow the user to type the file name
     */
    Gtk::Entry *usernameEntry;
    Gtk::Entry *passwordEntry;
    
    // Child widgets
    Gtk::VBox entriesBox;
    Gtk::HBox userBox;
    Gtk::HBox passBox;
    
};




//#########################################################################
//### F I L E   I M P O R T   F R O M   O C A L
//#########################################################################

/**
 * Our implementation class for filesListView
 */
class FileListViewText : public Gtk::ListViewText
{
public:
    FileListViewText(guint columns_count, SVGPreview& filesPreview):ListViewText(columns_count)
    {
        myPreview = &filesPreview;
    }
    Glib::ustring getFilename();
protected:
    void on_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
private:
    Glib::ustring myFilename;
    SVGPreview *myPreview;
};

/**
 * Our implementation class for the FileImportFromOCALDialog interface..
 */
class FileImportFromOCALDialogImplGtk : public FileImportFromOCALDialog, public FileDialogOCALBase
{
public:

    FileImportFromOCALDialogImplGtk(Gtk::Window& parentWindow,
    		       const Glib::ustring &dir,
                       FileDialogType fileTypes,
                       const Glib::ustring &title);

    virtual ~FileImportFromOCALDialogImplGtk();

    bool show();

    Inkscape::Extension::Extension *getSelectionType();

    Glib::ustring getFilename();

private:

    /**
     * Allow the user to type the tag to be searched
     */
    Gtk::Entry *searchTagEntry;
    FileListViewText *filesList;
    SVGPreview *filesPreview;
    Gtk::Label *notFoundLabel;

    // Child widgets
    Gtk::HBox tagBox;
    Gtk::HBox filesBox;
    Gtk::HBox messageBox;
    Gtk::ScrolledWindow listScrolledWindow;
    Glib::RefPtr<Gtk::TreeSelection> selection;

    /**
     * Callback for user input into searchTagEntry
     */
    void searchTagEntryChangedCallback();


    /**
     * Prints the names of the all the xml elements 
     * that are siblings or children of a given xml node
     */
    void print_xml_element_names(xmlNode * a_node);

    /**
     * The extension to use to write this file
     */
    Inkscape::Extension::Extension *extension;
};




} //namespace Dialog
} //namespace UI
} //namespace Inkscape


#endif /* __OCAL_DIALOG_H__ */

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
