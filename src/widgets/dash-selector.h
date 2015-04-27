#ifndef SEEN_SP_DASH_SELECTOR_NEW_H
#define SEEN_SP_DASH_SELECTOR_NEW_H

/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Maximilian Albert <maximilian.albert> (gtkmm-ification)
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm/box.h>
#include <gtkmm/combobox.h>
#include <gtkmm/liststore.h>

#include <sigc++/signal.h>


/**
 * Class that wraps a combobox and spinbutton for selecting dash patterns.
 */
class SPDashSelector : public Gtk::HBox {
public:
    SPDashSelector();
    ~SPDashSelector();

    /**
     * Get and set methods for dashes
     */
    void set_dash(int ndash, double *dash, double offset);
    void get_dash(int *ndash, double **dash, double *offset);

    sigc::signal<void> changed_signal;

private:

    /**
     * Initialize dashes list from preferences
     */
    static void init_dashes();

    /**
     * Fill a pixbuf with the dash pattern using standard cairo drawing
     */
    GdkPixbuf* sp_dash_to_pixbuf(double *pattern);

    /**
     * Fill a pixbuf with text standard cairo drawing
     */
    GdkPixbuf* sp_text_to_pixbuf(char *text);

    /**
     * Callback for combobox image renderer
     */
    void prepareImageRenderer( Gtk::TreeModel::const_iterator const &row );

    /**
     * Callback for offset adjustment changing
     */
    void offset_value_changed();

    /**
     * Callback for combobox selection changing
     */
    void on_selection();

    /**
     * Combobox columns
     */
    class DashColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        Gtk::TreeModelColumn<double *> dash;
        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > pixbuf;

        DashColumns() {
            add(dash); add(pixbuf);
        }
    };
    DashColumns dash_columns;
    Glib::RefPtr<Gtk::ListStore> dash_store;
    Gtk::ComboBox       dash_combo;
    Gtk::CellRendererPixbuf image_renderer;

#if WITH_GTKMM_3_0
    Glib::RefPtr<Gtk::Adjustment> offset;
#else
    Gtk::Adjustment     *offset;
#endif

    static gchar const *const _prefs_path;
    int preview_width;
    int preview_height;
    int preview_lineheight;

};

#endif // SEEN_SP_DASH_SELECTOR_NEW_H

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
