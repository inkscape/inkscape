/** @file
 * @brief Open Clip Art Library integration dialogs
 */
/* Authors:
 *   Bruno Dilly <bruno.dilly@gmail.com>
 *   Inkscape Guys
 *   Andrew Higginson
 *
 * Copyright (C) 2007 Bruno Dilly <bruno.dilly@gmail.com>
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
 
#ifndef __OCAL_DIALOG_H__
#define __OCAL_DIALOG_H__

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

//Gtk includes
#include <gtkmm/box.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/listviewtext.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/window.h>

#include <cairomm/refptr.h>

#if GTK_CHECK_VERSION(3,0,0)
# include <gtkmm/searchentry.h>
#endif

#include <giomm/file.h>

//Inkscape includes
#include "ui/dialog/filedialog.h"
#include "ui/dialog-events.h"

struct _xmlNode;
typedef _xmlNode xmlNode;

namespace Gtk {
class Notebook;
class Spinner;
}

namespace Inkscape
{
namespace UI 
{   
namespace Dialog
{   

class SVGPreview;
    
namespace OCAL
{
/*#########################################################################
### F I L E     D I A L O G    O C A L    B A S E    C L A S S
#########################################################################*/

/**
 * This class is the base implementation for export to OCAL.
 */
class FileDialogBase : public Gtk::Window
{
public:

    /**
     * Constructor
     */
    FileDialogBase(const Glib::ustring &title, Gtk::Window& /*parent*/) : Gtk::Window(Gtk::WINDOW_TOPLEVEL)
    {
        set_title(title);
        sp_transientize(GTK_WIDGET(gobj()));
        
        // Allow shrinking of window so labels wrap correctly
        set_resizable(true);

        Gdk::Geometry geom;
        geom.min_width = 480;
        geom.min_height = 320;

        set_geometry_hints(*this, geom, Gdk::HINT_MIN_SIZE);
    }

    /*
     * Destructor
     */
    virtual ~FileDialogBase()
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
 * Our implementation of the ExportDialog interface.
 */
/*
class ExportDialog : public FileDialogBase
{

public:
*/
    /**
     * Constructor
     * @param fileTypes one of FileDialogTypes
     * @param title the title of the dialog
     * @param key a list of file types from which the user can select
     */
/*
    ExportDialog(Gtk::Window& parentWindow, 
                             FileDialogType fileTypes,
                 const Glib::ustring &title);
*/
    /**
     * Destructor.
     * Perform any necessary cleanups.
     */
/*
    ~ExportDialog();
*/
    /**
     * Show an SaveAs file selector.
     * @return the selected path if user selected one, else NULL
     */
/*
    bool show();

    Glib::ustring getFilename();

    Glib::ustring myFilename;
*/
    /**
     * Change the window title.
     */
/*
    void change_title(const Glib::ustring& title);
    
private:
*/
    /**
     * Fix to allow the user to type the file name
     */
/*
    Gtk::Entry *fileNameEntry;
*/
    /**
     *  Data mirror of the combo box
     */
/*
    std::vector<FileType> fileTypes;

    // Child widgets
    Gtk::HBox childBox;
    Gtk::VBox checksBox;
    Gtk::HBox fileBox;
*/
    /**
     * The extension to use to write this file
     */
/*
    Inkscape::Extension::Extension *extension;
*/
    /**
     * Callback for user input into fileNameEntry
     */
/*
    void fileNameEntryChangedCallback();
*/
    /**
     * List of known file extensions.
     */
/*
    std::set<Glib::ustring> knownExtensions;

}; //ExportDialog
*/

//########################################################################
//# F I L E    E X P O R T   T O   O C A L   P A S S W O R D
//########################################################################


/**
 * Our implementation of the ExportPasswordDialog interface.
 */
/*
class ExportPasswordDialog : public FileDialogBase
{

public:
*/
    /**
     * Constructor
     * @param title the title of the dialog
     */
/*
    ExportPasswordDialog(Gtk::Window& parentWindow, 
                                const Glib::ustring &title);
*/
    /**
     * Destructor.
     * Perform any necessary cleanups.
     */
/*
    ~ExportPasswordDialog();
*/

    /**
     * Show 2 entry to input username and password.
     */
/*
    bool show();

    Glib::ustring getUsername();
    Glib::ustring getPassword();
*/
    /**
     * Change the window title.
     */
/*
    void change_title(const Glib::ustring& title);

    Glib::ustring myUsername;
    Glib::ustring myPassword;

private:
*/
    /**
     * Fix to allow the user to type the file name
     */
/*
    Gtk::Entry *usernameEntry;
    Gtk::Entry *passwordEntry;
    
    // Child widgets
    Gtk::VBox entriesBox;
    Gtk::HBox userBox;
    Gtk::HBox passBox;
    
}; //ExportPasswordDialog
*/


//#########################################################################
//### F I L E   I M P O R T   F R O M   O C A L
//#########################################################################

class WrapLabel : public Gtk::Label
{
public:
    WrapLabel();

private:
    void _on_size_allocate(Gtk::Allocation& allocation);
};

class LoadingBox : public Gtk::EventBox
{
public:
    LoadingBox();

    void start();
    void stop();

private:
    unsigned int spinner_step;
    sigc::connection timeout;
    bool draw_spinner;

#if !WITH_GTKMM_3_0
    bool _on_expose_event(GdkEventExpose* event);
#endif

    bool _on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
    bool on_timeout();
};

class PreviewWidget : public Gtk::VBox
{
public:
    PreviewWidget();

    void set_metadata(Glib::ustring description, Glib::ustring creator, Glib::ustring time);
    void show_box_loading();
    void hide_box_loading();
    void set_image(std::string path);
    void clear();

private:
    LoadingBox* box_loading;
    Gtk::Image* image;

    WrapLabel* label_title;
    WrapLabel* label_description;
    WrapLabel* label_time;
    
#if !WITH_GTKMM_3_0
    bool _on_expose_event(GdkEventExpose* event);
#endif

    bool _on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
};

/**
 * A Widget that contains an status icon and a message
 */
class StatusWidget : public Gtk::HBox
{
public:
    StatusWidget();

    void clear();
    void set_error(Glib::ustring text);
    void set_info(Glib::ustring text);
    void start_process(Glib::ustring text);
    void end_process();

    Gtk::Spinner* spinner;
    Gtk::Image* image;
    Gtk::Label* label;
};

#if !GTK_CHECK_VERSION(3,0,0)
/**
 * A Gtk::Entry with search & clear icons
 */
class SearchEntry : public Gtk::Entry
{
public:
    SearchEntry();
    
private:
    void _on_icon_pressed(Gtk::EntryIconPosition icon_position, const GdkEventButton* event);
    void _on_changed();
};
#endif

/**
 * A box which paints an overlay of the OCAL logo
 */
class LogoArea : public Gtk::EventBox
{
public:
    LogoArea();
private:
#if !WITH_GTKMM_3_0
    bool _on_expose_event(GdkEventExpose* event);
#endif
    bool _on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
    bool draw_logo;
    Cairo::RefPtr<Cairo::ImageSurface> logo_mask;
};

/**
 * A box filled with the Base colour from the user's GTK theme, and a border
 */
class BaseBox : public Gtk::EventBox
{
public:
    BaseBox();
private:
#if !WITH_GTKMM_3_0
    bool _on_expose_event(GdkEventExpose* event);
#endif
    bool _on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
};

enum {
    RESULTS_COLUMN_MARKUP,
    RESULTS_COLUMN_TITLE,
    RESULTS_COLUMN_DESCRIPTION,
    RESULTS_COLUMN_CREATOR,
    RESULTS_COLUMN_DATE,
    RESULTS_COLUMN_FILENAME,
    RESULTS_COLUMN_THUMBNAIL_FILENAME,
    RESULTS_COLUMN_URL,
    RESULTS_COLUMN_THUMBNAIL_URL,
    RESULTS_COLUMN_GUID,
    RESULTS_COLUMN_LENGTH,
};

enum {
    NOTEBOOK_PAGE_LOGO,
    NOTEBOOK_PAGE_RESULTS,
    NOTEBOOK_PAGE_NOT_FOUND,
};

enum ResourceType {
    TYPE_THUMBNAIL,
    TYPE_IMAGE,
};

/**
 * The TreeView which holds the search results
 */
class SearchResultList : public Gtk::ListViewText
{
public:
    SearchResultList(guint columns_count);
    void populate_from_xml(xmlNode* a_node);
};

/**
 * The Import Dialog
 */
class ImportDialog : public FileDialogBase
{
public:
    /**
     * Constructor
     * @param path the directory where to start searching
     * @param fileTypes one of FileDialogTypes
     * @param title the title of the dialog
     */
    ImportDialog(Gtk::Window& parent_window, FileDialogType file_types,
                const Glib::ustring &title);

    /**
     * Destructor.
     * Perform any necessary cleanups.
     */
    ~ImportDialog();

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
    Inkscape::Extension::Extension *get_selection_type();
    
    typedef sigc::signal<void, Glib::ustring> type_signal_response;
    type_signal_response signal_response();
    
protected:
  type_signal_response m_signal_response;

private:
    Glib::ustring filename_image;
    Glib::ustring filename_thumbnail;

#if GTK_CHECK_VERSION(3,0,0)
    Gtk::SearchEntry *entry_search;
#else
    SearchEntry *entry_search;
#endif

    LogoArea *drawingarea_logo;
    SearchResultList *list_results;
    PreviewWidget *preview_files;
    Gtk::Label *label_not_found;
    Gtk::Label *label_description;
    Gtk::Button *button_search;
    Gtk::Button *button_import;
    Gtk::Button *button_close;
    Gtk::Button *button_cancel;
    StatusWidget *widget_status;

    // Child widgets
    Gtk::Notebook *notebook_content;
    Gtk::HBox hbox_tags;
    Gtk::HBox hbox_files;
    Gtk::ScrolledWindow scrolledwindow_list;
    Glib::RefPtr<Gtk::TreeSelection> selection;

    Glib::RefPtr<Gio::Cancellable> cancellable_image;
    Glib::RefPtr<Gio::Cancellable> cancellable_thumbnail;
    bool downloading_thumbnail;
    bool cancelled_thumbnail;
    bool cancelled_image;

    void update_label_no_search_results();
    void update_preview(int row);
    void on_list_results_cursor_changed();
    
    void download_resource(ResourceType type, int row);
    void on_resource_downloaded(const Glib::RefPtr<Gio::AsyncResult>& result,
        Glib::RefPtr<Gio::File> file_remote, Glib::ustring path, ResourceType resource);
    void on_image_downloaded(Glib::ustring path, bool success);
    void on_thumbnail_downloaded(Glib::ustring path, bool success);
    void on_list_results_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
    void on_button_import_clicked();
    void on_button_close_clicked();
    void on_button_cancel_clicked();
    void on_button_search_clicked();
    void on_entry_search_activated();
    void on_list_results_selection_changed();
    void on_xml_file_read(const Glib::RefPtr<Gio::AsyncResult>& result,
        Glib::RefPtr<Gio::File> xml_file, Glib::ustring xml_uri);
    void create_temporary_dirs();
    std::string get_temporary_dir(ResourceType type);


    /**
     * The extension to use to write this file
     */
    Inkscape::Extension::Extension *extension;

}; //ImportDialog


} //namespace OCAL
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
