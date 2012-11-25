/** @file
 * @brief Implementation of the file dialog interfaces defined in filedialogimpl.h
 */
/* Authors:
 *   Bob Jamison
 *   Johan Engelen <johan@shouraizou.nl>
 *   Joel Holdsworth
 *   Bruno Dilly
 *   Other dudes from The Inkscape Organization
 *
 * Copyright (C) 2004-2008 Authors
 * Copyright (C) 2004-2007 The Inkscape Organization
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __FILE_DIALOGIMPL_H__
#define __FILE_DIALOGIMPL_H__

//Gtk includes
#include <gtkmm/filechooserdialog.h>
#include <glib/gstdio.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/checkbutton.h>

//General includes
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#include "filedialog.h"


namespace Gtk {
class Expander;
}

namespace Inkscape
{

class URI;

namespace UI
{
namespace Dialog
{

/*#########################################################################
### Utility
#########################################################################*/
void
fileDialogExtensionToPattern(Glib::ustring &pattern,
                      Glib::ustring &extension);

void
findEntryWidgets(Gtk::Container *parent,
                 std::vector<Gtk::Entry *> &result);

void
findExpanderWidgets(Gtk::Container *parent,
                    std::vector<Gtk::Expander *> &result);

/*#########################################################################
### SVG Preview Widget
#########################################################################*/

class FileType
{
    public:
    FileType(): name(), pattern(),extension(0) {}
    ~FileType() {}
    Glib::ustring name;
    Glib::ustring pattern;
    Inkscape::Extension::Extension *extension;
};

/**
 * Simple class for displaying an SVG file in the "preview widget."
 * Currently, this is just a wrapper of the sp_svg_view Gtk widget.
 * Hopefully we will eventually replace with a pure Gtkmm widget.
 */
class SVGPreview : public Gtk::VBox
{
public:

    SVGPreview();

    ~SVGPreview();

    bool setDocument(SPDocument *doc);

    bool setFileName(Glib::ustring &fileName);

    bool setFromMem(char const *xmlBuffer);

    bool set(Glib::ustring &fileName, int dialogType);

    bool setURI(URI &uri);

    /**
     * Show image embedded in SVG
     */
    void showImage(Glib::ustring &fileName);

    /**
     * Show the "No preview" image
     */
    void showNoPreview();

    /**
     * Show the "Too large" image
     */
    void showTooLarge(long fileLength);

private:
    /**
     * The svg document we are currently showing
     */
    SPDocument *document;

    /**
     * The sp_svg_view widget
     */
    Gtk::Widget *viewerGtk;

    /**
     * are we currently showing the "no preview" image?
     */
    bool showingNoPreview;

};

/*#########################################################################
### F I L E     D I A L O G    B A S E    C L A S S
#########################################################################*/

/**
 * This class is the base implementation for the others.  This
 * reduces redundancies and bugs.
 */
class FileDialogBaseGtk : public Gtk::FileChooserDialog
{
public:

    /**
     *
     */
    FileDialogBaseGtk(Gtk::Window& parentWindow, const Glib::ustring &title,
    		Gtk::FileChooserAction dialogType, FileDialogType type, gchar const* preferenceBase) :
        Gtk::FileChooserDialog(parentWindow, title, dialogType),
        preferenceBase(preferenceBase ? preferenceBase : "unknown"),
        _dialogType(type)
    {
        internalSetup();
    }

    /**
     *
     */
    FileDialogBaseGtk(Gtk::Window& parentWindow, const char *title,
                   Gtk::FileChooserAction dialogType, FileDialogType type, gchar const* preferenceBase) :
        Gtk::FileChooserDialog(parentWindow, title, dialogType),
        preferenceBase(preferenceBase ? preferenceBase : "unknown"),
        _dialogType(type)
    {
        internalSetup();
    }

    /**
     *
     */
    virtual ~FileDialogBaseGtk()
        {}

protected:
    void cleanup( bool showConfirmed );

    Glib::ustring const preferenceBase;
    /**
     * What type of 'open' are we? (open, import, place, etc)
     */
    FileDialogType _dialogType;

    /**
     * Our svg preview widget
     */
    SVGPreview svgPreview;

    /**
	 * Child widgets
	 */
    Gtk::CheckButton previewCheckbox;

private:
    void internalSetup();

    /**
     * Callback for user changing preview checkbox
     */
    void _previewEnabledCB();

    /**
     * Callback for seeing if the preview needs to be drawn
     */
    void _updatePreviewCallback();
};




/*#########################################################################
### F I L E    O P E N
#########################################################################*/

/**
 * Our implementation class for the FileOpenDialog interface..
 */
class FileOpenDialogImplGtk : public FileOpenDialog, public FileDialogBaseGtk
{
public:

    FileOpenDialogImplGtk(Gtk::Window& parentWindow,
    		       const Glib::ustring &dir,
                       FileDialogType fileTypes,
                       const Glib::ustring &title);

    virtual ~FileOpenDialogImplGtk();

    bool show();

    Inkscape::Extension::Extension *getSelectionType();

    Glib::ustring getFilename();

    std::vector<Glib::ustring> getFilenames();

	Glib::ustring getCurrentDirectory();

    /// Add a custom file filter menu item
    /// @param name - Name of the filter (such as "Javscript")
    /// @param pattern - File filtering patter (such as "*.js")
    /// Use the FileDialogType::CUSTOM_TYPE in constructor to not include other file types
    void addFilterMenu(Glib::ustring name, Glib::ustring pattern);

private:

    /**
     *  Create a filter menu for this type of dialog
     */
    void createFilterMenu();


    /**
     * Filter name->extension lookup
     */
    std::map<Glib::ustring, Inkscape::Extension::Extension *> extensionMap;

    /**
     * The extension to use to write this file
     */
    Inkscape::Extension::Extension *extension;

};



//########################################################################
//# F I L E    S A V E
//########################################################################

/**
 * Our implementation of the FileSaveDialog interface.
 */
class FileSaveDialogImplGtk : public FileSaveDialog, public FileDialogBaseGtk
{

public:
    FileSaveDialogImplGtk(Gtk::Window &parentWindow,
                          const Glib::ustring &dir,
                          FileDialogType fileTypes,
                          const Glib::ustring &title,
                          const Glib::ustring &default_key,
                          const gchar* docTitle,
                          const Inkscape::Extension::FileSaveMethod save_method);

    virtual ~FileSaveDialogImplGtk();

    bool show();

    Inkscape::Extension::Extension *getSelectionType();
    virtual void setSelectionType( Inkscape::Extension::Extension * key );

	Glib::ustring getCurrentDirectory();
	void addFileType(Glib::ustring name, Glib::ustring pattern);

private:
    //void change_title(const Glib::ustring& title);
    void change_path(const Glib::ustring& path);
    void updateNameAndExtension();

    /**
     * The file save method (essentially whether the dialog was invoked by "Save as ..." or "Save a
     * copy ..."), which is used to determine file extensions and save paths.
     */
    Inkscape::Extension::FileSaveMethod save_method;

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

    //# Child widgets
    Gtk::HBox childBox;
    Gtk::VBox checksBox;

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
};




#ifdef NEW_EXPORT_DIALOG

//########################################################################
//# F I L E     E X P O R T
//########################################################################

/**
 * Our implementation of the FileExportDialog interface.
 */
class FileExportDialogImpl : public FileExportDialog, public FileDialogBaseGtk
{

public:
    FileExportDialogImpl(Gtk::Window& parentWindow,
            const Glib::ustring &dir,
            FileDialogType fileTypes,
            const Glib::ustring &title,
            const Glib::ustring &default_key);

    virtual ~FileExportDialogImpl();

    bool show();

    Inkscape::Extension::Extension *getSelectionType();

    Glib::ustring getFilename();


    /**
     * Return the scope of the export.  One of the enumerated types
     * in ScopeType
     */
    ScopeType getScope()
        {
        if (pageButton.get_active())
            return SCOPE_PAGE;
        else if (selectionButton.get_active())
            return SCOPE_SELECTION;
        else if (customButton.get_active())
            return SCOPE_CUSTOM;
        else
            return SCOPE_DOCUMENT;

        }

    /**
     * Return left side of the exported region
     */
    double getSourceX()
        { return sourceX0Spinner.getValue(); }

    /**
     * Return the top of the exported region
     */
    double getSourceY()
        { return sourceY1Spinner.getValue(); }

    /**
     * Return the width of the exported region
     */
    double getSourceWidth()
        { return sourceWidthSpinner.getValue(); }

    /**
     * Return the height of the exported region
     */
    double getSourceHeight()
        { return sourceHeightSpinner.getValue(); }

    /**
     * Return the units of the coordinates of exported region
     */
    Glib::ustring getSourceUnits()
        { return sourceUnitsSpinner.getUnitAbbr(); }

    /**
     * Return the width of the destination document
     */
    double getDestinationWidth()
        { return destWidthSpinner.getValue(); }

    /**
     * Return the height of the destination document
     */
    double getDestinationHeight()
        { return destHeightSpinner.getValue(); }

    /**
     * Return the height of the exported region
     */
    Glib::ustring getDestinationUnits()
        { return destUnitsSpinner.getUnitAbbr(); }

    /**
     * Return the destination DPI image resulution, if bitmap
     */
    double getDestinationDPI()
        { return destDPISpinner.getValue(); }

    /**
     * Return whether we should use Cairo for rendering
     */
    bool getUseCairo()
        { return cairoButton.get_active(); }

    /**
     * Return whether we should use antialiasing
     */
    bool getUseAntialias()
        { return antiAliasButton.get_active(); }

    /**
     * Return the background color for exporting
     */
    unsigned long getBackground()
        { return backgroundButton.get_color().get_pixel(); }

private:

    /**
     * Fix to allow the user to type the file name
     */
    Gtk::Entry *fileNameEntry;

    //##########################################
    //# EXTRA WIDGET -- SOURCE SIDE
    //##########################################

    Gtk::Frame            sourceFrame;
    Gtk::VBox             sourceBox;

    Gtk::HBox             scopeBox;
    Gtk::RadioButtonGroup scopeGroup;
    Gtk::RadioButton      documentButton;
    Gtk::RadioButton      pageButton;
    Gtk::RadioButton      selectionButton;
    Gtk::RadioButton      customButton;

    Gtk::Table                      sourceTable;
    Inkscape::UI::Widget::Scalar    sourceX0Spinner;
    Inkscape::UI::Widget::Scalar    sourceY0Spinner;
    Inkscape::UI::Widget::Scalar    sourceX1Spinner;
    Inkscape::UI::Widget::Scalar    sourceY1Spinner;
    Inkscape::UI::Widget::Scalar    sourceWidthSpinner;
    Inkscape::UI::Widget::Scalar    sourceHeightSpinner;
    Inkscape::UI::Widget::UnitMenu  sourceUnitsSpinner;


    //##########################################
    //# EXTRA WIDGET -- DESTINATION SIDE
    //##########################################

    Gtk::Frame       destFrame;
    Gtk::VBox        destBox;

    Gtk::Table                      destTable;
    Inkscape::UI::Widget::Scalar    destWidthSpinner;
    Inkscape::UI::Widget::Scalar    destHeightSpinner;
    Inkscape::UI::Widget::Scalar    destDPISpinner;
    Inkscape::UI::Widget::UnitMenu  destUnitsSpinner;

    Gtk::HBox        otherOptionBox;
    Gtk::CheckButton cairoButton;
    Gtk::CheckButton antiAliasButton;
    Gtk::ColorButton backgroundButton;


    /**
     * 'Extra' widget that holds two boxes above
     */
    Gtk::HBox exportOptionsBox;


    //# Child widgets
    Gtk::CheckButton fileTypeCheckbox;

    /**
     * Allow the specification of the output file type
     */
    Gtk::ComboBoxText fileTypeComboBox;


    /**
     *  Data mirror of the combo box
     */
    std::vector<FileType> fileTypes;



    /**
     * Callback for user input into fileNameEntry
     */
    void fileTypeChangedCallback();

    /**
     *  Create a filter menu for this type of dialog
     */
    void createFileTypeMenu();


    bool append_extension;

    /**
     * The extension to use to write this file
     */
    Inkscape::Extension::Extension *extension;

    /**
     * Callback for user input into fileNameEntry
     */
    void fileNameEntryChangedCallback();

    /**
     * Filename that was given
     */
    Glib::ustring myFilename;
};

#endif // NEW_EXPORT_DIALOG

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

#endif /*__FILE_DIALOGIMPL_H__*/

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
