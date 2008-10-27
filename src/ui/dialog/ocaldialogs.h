/** @file
 * @brief Open Clip Art Library integration dialogs
 */
/* Authors:
 *   Bruno Dilly <bruno.dilly@gmail.com>
 *   Inkscape Guys
 *
 * Copyright (C) 2007 Bruno Dilly <bruno.dilly@gmail.com>
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
 
#ifndef __OCAL_DIALOG_H__
#define __OCAL_DIALOG_H__

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
     * Constructor
     */
    FileDialogOCALBase(const Glib::ustring &title, Gtk::Window& parent) : Gtk::Dialog(title, parent, true)
    {}

    /*
     * Destructor
     */
    virtual ~FileDialogOCALBase()
    {}

protected:
    void cleanup( bool showConfirmed );

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
class FileExportToOCALDialog : public FileDialogOCALBase
{

public:
    /**
     * Constructor
     * @param fileTypes one of FileDialogTypes
     * @param title the title of the dialog
     * @param key a list of file types from which the user can select
     */
    FileExportToOCALDialog(Gtk::Window& parentWindow, 
                             FileDialogType fileTypes,
			     const Glib::ustring &title);
    
    /**
     * Destructor.
     * Perform any necessary cleanups.
     */
    ~FileExportToOCALDialog();

    /**
     * Show an SaveAs file selector.
     * @return the selected path if user selected one, else NULL
     */
    bool show();

    Glib::ustring getFilename();

    Glib::ustring myFilename;

    /**
     * Change the window title.
     */
    void change_title(const Glib::ustring& title);
    
private:

    /**
     * Fix to allow the user to type the file name
     */
    Gtk::Entry *fileNameEntry;
    
    /**
     *  Data mirror of the combo box
     */
    std::vector<FileType> fileTypes;

    // Child widgets
    Gtk::HBox childBox;
    Gtk::VBox checksBox;
    Gtk::HBox fileBox;

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

}; //FileExportToOCAL


//########################################################################
//# F I L E    E X P O R T   T O   O C A L   P A S S W O R D
//########################################################################


/**
 * Our implementation of the FileExportToOCALPasswordDialog interface.
 */
class FileExportToOCALPasswordDialog : public FileDialogOCALBase
{

public:
    /**
     * Constructor
     * @param title the title of the dialog
     */
    FileExportToOCALPasswordDialog(Gtk::Window& parentWindow, 
                                const Glib::ustring &title);
    
    /**
     * Destructor.
     * Perform any necessary cleanups.
     */
    ~FileExportToOCALPasswordDialog();


    /**
     * Show 2 entry to input username and password.
     */
    bool show();

    Glib::ustring getUsername();
    Glib::ustring getPassword();

    /**
     * Change the window title.
     */
    void change_title(const Glib::ustring& title);

    Glib::ustring myUsername;
    Glib::ustring myPassword;

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
    
}; //FileExportToOCALPassword



//#########################################################################
//### F I L E   I M P O R T   F R O M   O C A L
//#########################################################################

/**
 * Our implementation class for filesListView
 */
class FileListViewText : public Gtk::ListViewText
{
public:
    FileListViewText(guint columns_count, SVGPreview& filesPreview, Gtk::Label& description, Gtk::Button& okButton)
                :ListViewText(columns_count)
    {
        myPreview = &filesPreview;
        myLabel = &description;
        myButton = &okButton;
    }
    Glib::ustring getFilename();
protected:
    void on_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
    void on_cursor_changed();
private:
    Glib::ustring myFilename;
    SVGPreview *myPreview;
    Gtk::Label *myLabel;
    Gtk::Button *myButton;
};

/**
 * Our implementation class for the FileImportFromOCALDialog interface..
 */
class FileImportFromOCALDialog : public FileDialogOCALBase
{
public:
    /**
     * Constructor
     * @param path the directory where to start searching
     * @param fileTypes one of FileDialogTypes
     * @param title the title of the dialog
     */
    FileImportFromOCALDialog(Gtk::Window& parentWindow,
    		       const Glib::ustring &dir,
                       FileDialogType fileTypes,
                       const Glib::ustring &title);

    /**
     * Destructor.
     * Perform any necessary cleanups.
     */
    ~FileImportFromOCALDialog();

    /**
     * Show an OpenFile file selector.
     * @return the selected path if user selected one, else NULL
     */
    bool show();

    /**
     * Return the 'key' (filetype) of the selection, if any
     * @return a pointer to a string if successful (which must
     * be later freed with g_free(), else NULL.
     */
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
    Gtk::Label *descriptionLabel;
    Gtk::Button *searchButton;
    Gtk::Button *okButton;

    // Child widgets
    Gtk::HBox tagBox;
    Gtk::HBox filesBox;
    Gtk::HBox messageBox;
    Gtk::HBox descriptionBox;
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

}; //FileImportFromOCALDialog


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
