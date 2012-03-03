/** @file
 * @brief export to bitmap dialog
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *
 * Copyright (C) 1999-2007 Authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SP_EXPORT_H
#define SP_EXPORT_H

#include <gtk/gtk.h>
#include <glibmm/i18n.h>
#include <gtkmm/expander.h>
#include <gtkmm/frame.h>
#include <gtkmm/textview.h>
#include <gtkmm/comboboxtext.h>

#include "desktop.h"
#include "ui/dialog/desktop-tracker.h"
#include "ui/widget/panel.h"
#include "ui/widget/button.h"
#include "ui/widget/entry.h"
#include "widgets/sp-attribute-widget.h"

namespace Inkscape {
namespace UI {
namespace Dialog {


/** What type of button is being pressed */
enum selection_type {
    SELECTION_PAGE = 0,  /**< Export the whole page */
    SELECTION_DRAWING,   /**< Export everything drawn on the page */
    SELECTION_SELECTION, /**< Export everything that is selected */
    SELECTION_CUSTOM,    /**< Allows the user to set the region exported */
    SELECTION_NUMBER_OF  /**< A counter for the number of these guys */
};

/** A list of strings that is used both in the preferences, and in the
    data fields to describe the various values of \c selection_type. */
static const char * selection_names[SELECTION_NUMBER_OF] = {
    "page", "drawing", "selection", "custom"};

/** The names on the buttons for the various selection types. */
static const char * selection_labels[SELECTION_NUMBER_OF] = {
    N_("_Page"), N_("_Drawing"), N_("_Selection"), N_("_Custom")};


/**
 * A dialog widget to export to various image formats such as bitmap and png.
 *
 * Creates a dialog window for exporting an image to a bitmap if one doesn't already exist and
 * shows it to the user. If the dialog has already been created, it simply shows the window.
 *
 */
class Export : public Widget::Panel {
public:
    Export ();
    ~Export ();

    static Export &getInstance() { return *new Export(); }

private:

    /**
     * A function to set the xdpi.
     *
     * This function grabs all of the x values and then figures out the
     * new bitmap size based on the changing dpi value.  The dpi value is
     * gotten from the xdpi setting as these can not currently be independent.
     *
     */
    void setImageX();
    /**
     * A function to set the ydpi.
     *
     * This function grabs all of the y values and then figures out the
     * new bitmap size based on the changing dpi value.  The dpi value is
     * gotten from the xdpi setting as these can not currently be independent.
     */
    void setImageY();
    bool bbox_equal(Geom::Rect const &one, Geom::Rect const &two);
    void updateCheckbuttons ();
    inline void findDefaultSelection();
    void detectSize();
    void setArea ( double x0, double y0, double x1, double y1);
    /*
     * Getter/setter style functions for the spinbuttons
     */
    void setValue (Gtk::Adjustment *adj, double val);
    void setValuePx (Gtk::Adjustment *adj, double val);
    float getValue    ( Gtk::Adjustment *adj );
    float getValuePx ( Gtk::Adjustment *adj );
    /*
     * Helper function to create, style and pack spinbuttons
     */
    Gtk::Adjustment * createSpinbutton( gchar const *key, float val, float min, float max,
                                          float step, float page, GtkWidget *us,
                                          GtkWidget *t, int x, int y,
                                          const gchar *ll, const gchar *lr,
                                          int digits, unsigned int sensitive,
                                          void (Export::*cb)() );
    /**
     * One of the area select radio buttons was pressed
     */
    void onAreaToggled();

    /**
     * Export button callback
     */
    void onExport ();

    /**
     * File Browse button callback
     */
    void onBrowse ();

    /**
     * Area X value changed callback
     */
    void onAreaX0Change() {areaXChange(x0_adj);} ;
    void onAreaX1Change() {areaXChange(x1_adj);} ;
    void areaXChange ( Gtk::Adjustment *adj);

    /**
     * Area Y value changed callback
     */
    void onAreaY0Change() {areaYChange(y0_adj);} ;
    void onAreaY1Change() {areaYChange(y1_adj);} ;
    void areaYChange ( Gtk::Adjustment *adj);

    /**
     * Area width value changed callback
     */
    void onAreaWidthChange   ();

    /**
     * Area height value changed callback
     */
    void onAreaHeightChange  ();

    /**
     * Bitmap width value changed callback
     */
    void onBitmapWidthChange ();

    /**
     * Bitmap height value changed callback
     */
    void onBitmapHeightChange ();

    /**
     * Export xdpi value changed callback
     */
    void onExportXdpiChange ();

    /**
     * Batch export callback
     */
    void onBatchClicked ();

    /**
     * Inkscape selection change callback
     */
    void onSelectionChanged ();
    void onSelectioModified (guint flags);

    /**
     * Filename modified callback
     */
    void onFilenameModified ();

    /**
     * Can be invoked for setting the desktop. Currently not used.
     */
    void setDesktop(SPDesktop *desktop);

    /**
     * Is invoked by the desktop tracker when the desktop changes.
     */
    void setTargetDesktop(SPDesktop *desktop);

    /**
     * Progress dialog callbacks
     */
    GtkWidget * create_progress_dialog (gchar *progress_text);
    static unsigned int onProgressCallback (float value, void *data);
    static void onProgressCancel ( GtkWidget *widget, GObject *base );
    static gint onProgressDelete ( GtkWidget *widget, GdkEvent *event, GObject *base );

    /*
     * Utlitiy filename and path functions
     */
    void set_default_filename ();
    gchar* create_filepath_from_id (const gchar *id, const gchar *file_entry_text);
    gchar *filename_add_extension (const gchar *filename, const gchar *extension);
    gchar *absolutize_path_from_document_location (SPDocument *doc, const gchar *filename);

    /*
     * Currently selected export area type
     */
    selection_type current_key;
    /*
     * Original name for the export object
     */
    gchar * original_name;
    gchar * doc_export_name;
    /*
     * Was the Original name modified
     */
    bool filename_modified;
    bool was_empty;
    /*
     * Flag to stop simultaneous updates
     */
    bool update;

    /* Area selection radio buttons */
    Gtk::HBox togglebox;
    Gtk::RadioButton *selectiontype_buttons[SELECTION_NUMBER_OF];

    Gtk::VBox area_box;
    Gtk::VBox singleexport_box;

    /* Custom size widgets */
    Gtk::Adjustment *x0_adj;
    Gtk::Adjustment *x1_adj;
    Gtk::Adjustment *y0_adj;
    Gtk::Adjustment *y1_adj;
    Gtk::Adjustment *width_adj;
    Gtk::Adjustment *height_adj;

    /* Bitmap size widgets */
    Gtk::Adjustment *bmwidth_adj;
    Gtk::Adjustment *bmheight_adj;
    Gtk::Adjustment *xdpi_adj;
    Gtk::Adjustment *ydpi_adj;

    Gtk::VBox size_box;
    Gtk::Label* bm_label;

    Gtk::VBox file_box;
    Gtk::Label *flabel;
    Gtk::Entry filename_entry;

    /* Unit selector widgets */
    Gtk::HBox unitbox;
    Gtk::Widget* unit_selector;
    Gtk::Label units_label;

    /* Filename widgets  */
    Gtk::HBox filename_box;
    Gtk::Button browse_button;
    Gtk::Label browse_label;
    Gtk::Image browse_image;

    Gtk::HBox batch_box;
    Inkscape::UI::Widget::CheckButton    batch_export;

    Gtk::HBox hide_box;
    Inkscape::UI::Widget::CheckButton    hide_export;

    /* Export Button widgets */
    Gtk::HButtonBox button_box;
    Gtk::Button export_button;
    Gtk::Label export_label;
    Gtk::Image export_image;

    Inkscape::Preferences *prefs;
    SPDesktop *desktop;
    DesktopTracker deskTrack;
    sigc::connection desktopChangeConn;
    sigc::connection selectChangedConn;
    sigc::connection subselChangedConn;
    sigc::connection selectModifiedConn;

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
