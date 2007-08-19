/**
 * Implementation of the OCAL import/export dialogs
 *
 * Authors:
 *   Joel Holdsworth
 *   Bruno Dilly
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2007 Bruno Dilly
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "filedialogimpl-gtkmm.h"

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
class FileExportToOCALDialog : public FileDialogOCALBase
{

public:
    FileExportToOCALDialog(Gtk::Window& parentWindow, 
			     const Glib::ustring &title,
			     const Glib::ustring &default_key);
    
    /**
     * Destructor.
     * Perform any necessary cleanups.
     */
    virtual ~FileExportToOCALDialog();

    bool show();

    /**
     * Return the 'key' (filetype) of the selection, if any
     * @return a pointer to a string if successful (which must
     * be later freed with g_free(), else NULL.
     */
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
     * Callback for user input into fileNameEntry
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
 * Our implementation class for the FileImportFromOCALDialog
 */
class FileImportFromOCALDialog : public FileDialogOCALBase
{
public:

    FileImportFromOCALDialog(Gtk::Window& parentWindow,
    		       const Glib::ustring &dir,
                       const Glib::ustring &title);

    /**
     * Destructor.
     * Perform any necessary cleanups.
     */
    virtual ~FileImportFromOCALDialog();

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

}
}
}

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
