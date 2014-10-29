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

#include <gtkmm/progressbar.h>

#include "ui/dialog/desktop-tracker.h"
#include "ui/widget/panel.h"
#include "ui/widget/button.h"

namespace Gtk {
class Dialog;
}

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

    static Export &getInstance() {
        return *new Export();
    }

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
#if WITH_GTKMM_3_0
    void setValue(Glib::RefPtr<Gtk::Adjustment>& adj, double val);
    void setValuePx(Glib::RefPtr<Gtk::Adjustment>& adj, double val);
    float getValue(Glib::RefPtr<Gtk::Adjustment>& adj);
    float getValuePx(Glib::RefPtr<Gtk::Adjustment>& adj);
#else
    void setValue (Gtk::Adjustment *adj, double val);
    void setValuePx (Gtk::Adjustment *adj, double val);
    float getValue (Gtk::Adjustment *adj);
    float getValuePx (Gtk::Adjustment *adj);
#endif

    /**
     * Helper function to create, style and pack spinbuttons for the export dialog.
     *
     * Creates a new spin button for the export dialog.
     * @param  key  The name of the spin button
     * @param  val  A default value for the spin button
     * @param  min  Minimum value for the spin button
     * @param  max  Maximum value for the spin button
     * @param  step The step size for the spin button
     * @param  page Size of the page increment
     * @param  t    Table to put the spin button in
     * @param  x    X location in the table \c t to start with
     * @param  y    Y location in the table \c t to start with
     * @param  ll   Text to put on the left side of the spin button (optional)
     * @param  lr   Text to put on the right side of the spin button (optional)
     * @param  digits  Number of digits to display after the decimal
     * @param  sensitive  Whether the spin button is sensitive or not
     * @param  cb   Callback for when this spin button is changed (optional)
     *
     * No unit_selector is stored in the created spinbutton, relies on external unit management
     */
#if WITH_GTKMM_3_0
    Glib::RefPtr<Gtk::Adjustment> createSpinbutton( gchar const *key, float val, float min, float max,
                                                    float step, float page,
                                                    Gtk::Grid *t, int x, int y,
                                                    const Glib::ustring& ll, const Glib::ustring& lr,
                                                    int digits, unsigned int sensitive,
                                                    void (Export::*cb)() );
#else
    Gtk::Adjustment * createSpinbutton( gchar const *key, float val, float min, float max,
                                        float step, float page,
                                        Gtk::Table *t, int x, int y,
                                        const Glib::ustring& ll, const Glib::ustring& lr,
                                        int digits, unsigned int sensitive,
                                        void (Export::*cb)() );
#endif

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
    void onAreaX0Change() {
        areaXChange(x0_adj);
    } ;
    void onAreaX1Change() {
        areaXChange(x1_adj);
    } ;
#if WITH_GTKMM_3_0
    void areaXChange(Glib::RefPtr<Gtk::Adjustment>& adj);
#else
    void areaXChange ( Gtk::Adjustment *adj);
#endif

    /**
     * Area Y value changed callback
     */
    void onAreaY0Change() {
        areaYChange(y0_adj);
    } ;
    void onAreaY1Change() {
        areaYChange(y1_adj);
    } ;
#if WITH_GTKMM_3_0
    void areaYChange(Glib::RefPtr<Gtk::Adjustment>& adj);
#else
    void areaYChange ( Gtk::Adjustment *adj);
#endif

    /**
     * Unit changed callback
     */
    void onUnitChanged();

    /**
     * Hide except selected callback
     */
    void onHideExceptSelected ();

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
    void onSelectionModified (guint flags);

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
     * Creates progress dialog for batch exporting.
     *
     * @param progress_text Text to be shown in the progress bar
     */
    Gtk::Dialog * create_progress_dialog (Glib::ustring progress_text);

    /**
     * Callback to be used in for loop to update the progress bar.
     *
     * @param value number between 0 and 1 indicating the fraction of progress (0.17 = 17 % progress)
     * @param dlg void pointer to the Gtk::Dialog progress dialog
     */
    static unsigned int onProgressCallback(float value, void *dlg);

    /**
     * Callback for pressing the cancel button.
     */
    void onProgressCancel ();

    /**
     * Callback invoked on closing the progress dialog.
     */
    bool onProgressDelete (GdkEventAny *event);

    /**
     * Handles state changes as exporting starts or stops.
     */
    void setExporting(bool exporting, Glib::ustring const &text = "");

    /*
     * Utility filename and path functions
     */
    void set_default_filename ();
    Glib::ustring create_filepath_from_id (Glib::ustring id, const Glib::ustring &file_entry_text);
    Glib::ustring filename_add_extension (Glib::ustring filename, Glib::ustring extension);
    Glib::ustring absolutize_path_from_document_location (SPDocument *doc, const Glib::ustring &filename);

    /*
     * Currently selected export area type
     */
    selection_type current_key;
    /*
     * Original name for the export object
     */
    Glib::ustring original_name;
    Glib::ustring doc_export_name;
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

#if WITH_GTKMM_3_0
    /* Custom size widgets */
    Glib::RefPtr<Gtk::Adjustment> x0_adj;
    Glib::RefPtr<Gtk::Adjustment> x1_adj;
    Glib::RefPtr<Gtk::Adjustment> y0_adj;
    Glib::RefPtr<Gtk::Adjustment> y1_adj;
    Glib::RefPtr<Gtk::Adjustment> width_adj;
    Glib::RefPtr<Gtk::Adjustment> height_adj;

    /* Bitmap size widgets */
    Glib::RefPtr<Gtk::Adjustment> bmwidth_adj;
    Glib::RefPtr<Gtk::Adjustment> bmheight_adj;
    Glib::RefPtr<Gtk::Adjustment> xdpi_adj;
    Glib::RefPtr<Gtk::Adjustment> ydpi_adj;
#else
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
#endif

    Gtk::VBox size_box;
    Gtk::Label* bm_label;

    Gtk::VBox file_box;
    Gtk::Label *flabel;
    Gtk::Entry filename_entry;

    /* Unit selector widgets */
    Gtk::HBox unitbox;
    Inkscape::UI::Widget::UnitMenu unit_selector;
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

    Inkscape::UI::Widget::CheckButton closeWhenDone;

    /* Export Button widgets */
    Gtk::HBox button_box;
    Gtk::Button export_button;
    Gtk::Label export_label;
    Gtk::Image export_image;

    Gtk::ProgressBar _prog;

    Gtk::Dialog *prog_dlg;
    bool interrupted; // indicates whether export needs to be interrupted (read: user pressed cancel in the progress dialog)

    Inkscape::Preferences *prefs;
    SPDesktop *desktop;
    DesktopTracker deskTrack;
    sigc::connection desktopChangeConn;
    sigc::connection selectChangedConn;
    sigc::connection subselChangedConn;
    sigc::connection selectModifiedConn;
    sigc::connection unitChangedConn;

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
