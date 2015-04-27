/** @file
 * @brief Implementation of native file dialogs for Win32
 */
/* Authors:
 *   Joel Holdsworth
 *   The Inkscape Organization
 *
 * Copyright (C) 2004-2008 The Inkscape Organization
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <glibmm.h>

#ifdef WIN32

#include "filedialogimpl-gtkmm.h"
 
#include "inkgc/gc-core.h"
 // define WINVER high enough so we get the correct OPENFILENAMEW size
#ifndef WINVER
#define WINVER 0x0500 
#endif
#include <windows.h>


namespace Inkscape
{
namespace UI
{
namespace Dialog
{

/*#########################################################################
### F I L E     D I A L O G    B A S E    C L A S S
#########################################################################*/

/// This class is the base implementation of a MS Windows
/// file dialog.
class FileDialogBaseWin32
{
protected:
    /// Abstract Constructor
    /// @param parent The parent window for the dialog
    /// @param dir The directory to begin browing from
    /// @param title The title caption for the dialog in UTF-8
    /// @param type The dialog type
    /// @param preferenceBase The preferences key
    FileDialogBaseWin32(Gtk::Window &parent, const Glib::ustring &dir,
        const char *title, FileDialogType type,
        gchar const *preferenceBase);

    /// Destructor
    ~FileDialogBaseWin32();

public:

    /// Gets the currently selected extension. Valid after an [OK]
    /// @return Returns a pointer to the selected extension, or NULL
    /// if the selected filter requires an automatic type detection
    Inkscape::Extension::Extension* getSelectionType();

    /// Get the path of the current directory
    Glib::ustring getCurrentDirectory();


protected:
    /// The dialog type
    FileDialogType dialogType;

	/// A pointer to the GTK main-loop context object. This
    /// is used to keep the rest of the inkscape UI running
    /// while the file dialog is displayed
    GMainLoop *_main_loop;

    /// The result of the call to GetOpenFileName. If true
    /// the user clicked OK, if false the user clicked cancel
    bool _result;

    /// The parent window
    Gtk::Window &parent;

    /// The windows handle of the parent window
    HWND _ownerHwnd;

    /// The path of the directory that is currently being
    /// browsed
    Glib::ustring _current_directory;

    /// The title of the dialog in UTF-16
    wchar_t *_title;

    /// The path of the currently selected file in UTF-16
    wchar_t _path_string[_MAX_PATH];

    /// The filter string for GetOpenFileName in UTF-16
    wchar_t *_filter;

    /// The index of the currently selected filter.
	/// This value must be greater than or equal to 1,
	/// and less than or equal to _filter_count.
    unsigned int _filter_index;

	/// The number of filters registered
	unsigned int _filter_count;

    /// An array of the extensions associated with the
    /// file types of each filter. So the Nth entry of
    /// this array corresponds to the extension of the Nth
    /// filter in the list. NULL if no specific extension is
    /// specified/
    Inkscape::Extension::Extension **_extension_map;

	/// The currently selected extension. Valid after an [OK]
    Inkscape::Extension::Extension *_extension;
};


/*#########################################################################
### F I L E    O P E N
#########################################################################*/

/// An Inkscape compatible wrapper around MS Windows GetOpenFileName API
class FileOpenDialogImplWin32 : public FileOpenDialog, public FileDialogBaseWin32
{
public:
    /// Constructor
    /// @param parent The parent window for the dialog
    /// @param dir The directory to begin browing from
    /// @param title The title caption for the dialog in UTF-8
    /// @param type The dialog type
    FileOpenDialogImplWin32(Gtk::Window &parent,
                            const Glib::ustring &dir,
                            FileDialogType fileTypes,
                            const char *title);

    /// Destructor
    virtual ~FileOpenDialogImplWin32();

    /// Shows the file dialog, and blocks until a file
    /// has been selected.
    /// @return Returns true if the user selected a
    /// file, or false if the user pressed cancel.
    bool show();

    /// Gets a list of the selected file names
    /// @return Returns an STL vector filled with the
    /// GTK names of the selected files
    std::vector<Glib::ustring> getFilenames();

    /// Get the path of the current directory
    virtual Glib::ustring getCurrentDirectory()
        { return FileDialogBaseWin32::getCurrentDirectory(); }

    /// Gets the currently selected extension. Valid after an [OK]
    /// @return Returns a pointer to the selected extension, or NULL
    /// if the selected filter requires an automatic type detection
    virtual Inkscape::Extension::Extension* getSelectionType()
        { return FileDialogBaseWin32::getSelectionType(); }


    /// Add a custom file filter menu item
    /// @param name - Name of the filter (such as "Javscript")
    /// @param pattern - File filtering patter (such as "*.js")
    /// Use the FileDialogType::CUSTOM_TYPE in constructor to not include other file types
    virtual void addFilterMenu(Glib::ustring name, Glib::ustring pattern);

private:

    /// Create filter menu for this type of dialog
    void createFilterMenu();


    /// The handle of the preview pane window
    HWND _preview_wnd;

    /// The handle of the file dialog window
    HWND _file_dialog_wnd;

    /// A pointer to the standard window proc of the
    /// unhooked file dialog
    WNDPROC _base_window_proc;

    /// The handle of the bitmap of the "show preview"
    /// toggle button
    HBITMAP _show_preview_button_bitmap;

    /// The handle of the toolbar's window
    HWND _toolbar_wnd;

    /// This flag is set true when the preview should be
    /// shown, or false when it should be hidden
    static bool _show_preview;


    /// The current width of the preview pane in pixels
    int _preview_width;

    /// The current height of the preview pane in pixels
    int _preview_height;

    /// The handle of the windows to display within the
    /// preview pane, or NULL if no image should be displayed
    HBITMAP _preview_bitmap;

    /// The windows shell icon for the selected file
    HICON _preview_file_icon;

    /// The size of the preview file in kilobytes
    unsigned long _preview_file_size;


    /// The width of the document to be shown in the preview panel
    double _preview_document_width;

    /// The width of the document to be shown in the preview panel
    double _preview_document_height;

    /// The width of the rendered preview image in pixels
    int _preview_image_width;

    /// The height of the rendered preview image in pixels
    int _preview_image_height;

    /// A GDK Pixbuf of the rendered preview to be displayed
    Glib::RefPtr<Gdk::Pixbuf> _preview_bitmap_image;

    /// This flag is set true if a file has been selected
    bool _file_selected;

	/// This flag is set true when the GetOpenFileName call
    /// has returned
    bool _finished;

    /// This mutex is used to ensure that the worker thread
    /// that calls GetOpenFileName cannot collide with the
    /// main Inkscape thread
#if GLIB_CHECK_VERSION(2,32,0)
    Glib::Threads::Mutex *_mutex;
#else
    Glib::Mutex *_mutex;
#endif


    /// The controller function for the thread which calls
    /// GetOpenFileName
    void GetOpenFileName_thread();

    /// Registers the Windows Class of the preview panel window
    static void register_preview_wnd_class();

    /// A message proc which is called by the standard dialog
    /// proc
    static UINT_PTR CALLBACK GetOpenFileName_hookproc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);

    /// A message proc which wraps the standard dialog proc,
    /// but intercepts some calls
    static LRESULT CALLBACK file_dialog_subclass_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    /// The message proc for the preview panel window
    static LRESULT CALLBACK preview_wnd_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    /// Lays out the controls in the file dialog given it's
    /// current size
    /// GetOpenFileName thread only.
    void layout_dialog();

    /// Enables or disables the file preview.
    /// GetOpenFileName thread only.
    void enable_preview(bool enable);

    /// This function is called in the App thread when a file had
    /// been selected
    void file_selected();

    /// Loads and renders the unshrunk preview image.
    /// Main app thread only.
    void load_preview();

    /// Frees all the allocated objects associated with the file
    /// currently being previewed
    /// Main app thread only.
    void free_preview();

    /// Loads preview for an SVG or SVGZ file.
    /// Main app thread only.
    /// @return Returns true if the SVG loaded successfully
    bool set_svg_preview();

    /// A callback to allow this class to dispose of the
    /// memory block of the rendered SVG bitmap
    /// @buffer buffer The buffer to free
    static void destroy_svg_rendering(const guint8 *buffer);

    /// Loads the preview for a raster image
    /// Main app thread only.
    /// @return Returns true if the image loaded successfully
    bool set_image_preview();

    /// Loads the preview for a meta file
    /// Main app thread only.
    /// @return Returns true if the image loaded successfully
    bool set_emf_preview();

    /// This flag is set true when a meta file is previewed
    bool _preview_emf_image;

    /// Renders the unshrunk preview image to a windows HTBITMAP
    /// which can be painted in the preview pain.
    /// Main app thread only.
    void render_preview();

    /// Formats the caption in UTF-16 for the preview image
    /// @param caption The buffer to format the caption string into
    /// @param caption_size The number of wchar_ts in the caption buffer
    /// @return Returns the number of characters in caption string
    int format_caption(wchar_t *caption, int caption_size);
};


/*#########################################################################
### F I L E    S A V E
#########################################################################*/

/// An Inkscape compatible wrapper around MS Windows GetSaveFileName API
class FileSaveDialogImplWin32 : public FileSaveDialog, public FileDialogBaseWin32
{

public:
    FileSaveDialogImplWin32(Gtk::Window &parent,
                            const Glib::ustring &dir,
                            FileDialogType fileTypes,
                            const char *title,
                            const Glib::ustring &default_key,
                            const char *docTitle,
                            const Inkscape::Extension::FileSaveMethod save_method);

    /// Destructor
    virtual ~FileSaveDialogImplWin32();

    /// Shows the file dialog, and blocks until a file
    /// has been selected.
    /// @return Returns true if the user selected a
    /// file, or false if the user pressed cancel.
    bool show();

    /// Get the path of the current directory
    virtual Glib::ustring getCurrentDirectory()
        { return FileDialogBaseWin32::getCurrentDirectory(); }

    /// Gets the currently selected extension. Valid after an [OK]
    /// @return Returns a pointer to the selected extension, or NULL
    /// if the selected filter requires an automatic type detection
    virtual Inkscape::Extension::Extension* getSelectionType()
        { return FileDialogBaseWin32::getSelectionType(); }

    virtual void setSelectionType( Inkscape::Extension::Extension *key );

    virtual void addFileType(Glib::ustring name, Glib::ustring pattern);

private:
	/// A handle to the title label and edit box
    HWND _title_label;
    HWND _title_edit;

    /// Create a filter menu for this type of dialog
    void createFilterMenu();

    /// The controller function for the thread which calls
    /// GetSaveFileName
    void GetSaveFileName_thread();

    /// A message proc which is called by the standard dialog
    /// proc
    static UINT_PTR CALLBACK GetSaveFileName_hookproc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);

};


}
}
}

#endif

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
