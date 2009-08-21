/** @file
 * @brief  PNG export dialog
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

// This has to be included prior to anything that includes setjmp.h, it croaks otherwise
#include <png.h>

#include <gtk/gtk.h>
#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/label.h>
#include <gtkmm/widget.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/image.h>
#include <gtkmm/stockid.h>
#include <gtkmm/stock.h>
#ifdef WITH_GNOME_VFS
# include <libgnomevfs/gnome-vfs-init.h>  // gnome_vfs_initialized
#endif

#include <glibmm/i18n.h>
#include "helper/unit-menu.h"
#include "helper/units.h"
#include "unit-constants.h"
#include "helper/window.h"
#include "inkscape-private.h"
#include "document.h"
#include "desktop-handles.h"
#include "sp-item.h"
#include "selection.h"
#include "file.h"
#include "macros.h"
#include "sp-namedview.h"
#include "selection-chemistry.h"

#include "dialog-events.h"
#include "preferences.h"
#include "verbs.h"
#include "interface.h"

#include "extension/output.h"
#include "extension/db.h"

#include "io/sys.h"

#include "helper/png-write.h"

#ifdef WIN32
#include <windows.h>
#include <COMMDLG.h>
#include <gdk/gdkwin32.h>
#endif

#define SP_EXPORT_MIN_SIZE 1.0

#define DPI_BASE PX_PER_IN

#define EXPORT_COORD_PRECISION 3

#define MIN_ONSCREEN_DISTANCE 50

static void sp_export_area_toggled   ( GtkToggleButton *tb, GtkObject *base );
static void sp_export_export_clicked ( GtkButton *button, GtkObject *base );
static void sp_export_browse_clicked ( GtkButton *button, gpointer userdata );

static void sp_export_area_x_value_changed       ( GtkAdjustment *adj,
                                                   GtkObject *base);

static void sp_export_area_y_value_changed       ( GtkAdjustment *adj,
                                                   GtkObject *base);

static void sp_export_area_width_value_changed   ( GtkAdjustment *adj,
                                                   GtkObject *base);

static void sp_export_area_height_value_changed  ( GtkAdjustment *adj,
                                                   GtkObject *base);

static void sp_export_bitmap_width_value_changed ( GtkAdjustment *adj,
                                                   GtkObject *base);

static void sp_export_bitmap_height_value_changed ( GtkAdjustment *adj,
                                                   GtkObject *base);

static void sp_export_xdpi_value_changed         ( GtkAdjustment *adj,
                                                   GtkObject *base);

static void sp_export_selection_changed ( Inkscape::Application *inkscape,
                                          Inkscape::Selection *selection,
                                          GtkObject *base);
static void sp_export_selection_modified ( Inkscape::Application *inkscape,
                                           Inkscape::Selection *selection,
                                           guint flags,
                                           GtkObject *base );

static void sp_export_set_area (GtkObject *base, double x0, double y0, double x1, double y1);
static void sp_export_value_set (GtkObject *base, const gchar *key, double val);
static void sp_export_value_set_px (GtkObject *base, const gchar *key, double val);
static float sp_export_value_get    ( GtkObject *base, const gchar *key );
static float sp_export_value_get_px ( GtkObject *base, const gchar *key );

static void sp_export_filename_modified (GtkObject * object, gpointer data);
static inline void sp_export_find_default_selection(GtkWidget * dlg);
static void sp_export_detect_size(GtkObject * base);

static Glib::ustring const prefs_path = "/dialogs/export/";

// these all need to be reinitialized to their defaults during dialog_destroy
static GtkWidget *dlg = NULL;
static win_data wd;
static gint x = -1000, y = -1000, w = 0, h = 0; // impossible original values to make sure they are read from prefs
static gchar * original_name = NULL;
static gchar * doc_export_name = NULL;
static bool was_empty = TRUE;

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

static void
sp_export_dialog_destroy ( GtkObject */*object*/, gpointer /*data*/ )
{
    sp_signal_disconnect_by_data (INKSCAPE, dlg);

    wd.win = dlg = NULL;
    wd.stop = 0;
    x = -1000; y = -1000; w = 0; h = 0;
    g_free(original_name);
    original_name = NULL;
    g_free(doc_export_name);
    doc_export_name = NULL;
    was_empty = TRUE;

    return;
} // end of sp_export_dialog_destroy()

/// Called when dialog is closed or inkscape is shut down.
static bool
sp_export_dialog_delete ( GtkObject */*object*/, GdkEvent */*event*/, gpointer /*data*/ )
{

    gtk_window_get_position ((GtkWindow *) dlg, &x, &y);
    gtk_window_get_size ((GtkWindow *) dlg, &w, &h);

    if (x<0) x=0;
    if (y<0) y=0;

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setInt(prefs_path + "x", x);
    prefs->setInt(prefs_path + "y", y);
    prefs->setInt(prefs_path + "w", w);
    prefs->setInt(prefs_path + "h", h);

    return FALSE; // which means, go ahead and destroy it

} // end of sp_export_dialog_delete()

/**
    \brief  Creates a new spin button for the export dialog
    \param  key  The name of the spin button
    \param  val  A default value for the spin button
    \param  min  Minimum value for the spin button
    \param  max  Maximum value for the spin button
    \param  step The step size for the spin button
    \param  page Size of the page increment
    \param  us   Unit selector that effects this spin button
    \param  t    Table to put the spin button in
    \param  x    X location in the table \c t to start with
    \param  y    Y location in the table \c t to start with
    \param  ll   Text to put on the left side of the spin button (optional)
    \param  lr   Text to put on the right side of the spin button (optional)
    \param  digits  Number of digits to display after the decimal
    \param  sensitive  Whether the spin button is sensitive or not
    \param  cb   Callback for when this spin button is changed (optional)
    \param  dlg  Export dialog the spin button is being placed in

*/
static void
sp_export_spinbutton_new ( gchar const *key, float val, float min, float max,
                           float step, float page, GtkWidget *us,
                           GtkWidget *t, int x, int y,
                           const gchar *ll, const gchar *lr,
                           int digits, unsigned int sensitive,
                           GCallback cb, GtkWidget *dlg )
{
    GtkObject *adj = gtk_adjustment_new( val, min, max, step, page, 0 );
    gtk_object_set_data( adj, "key", const_cast<gchar *>(key) );
    gtk_object_set_data( GTK_OBJECT (dlg), (const gchar *)key, adj );

    if (us) {
        sp_unit_selector_add_adjustment ( SP_UNIT_SELECTOR (us),
                                          GTK_ADJUSTMENT (adj) );
    }

    int pos = 0;

    GtkWidget *l = NULL;

    if (ll) {

        l = gtk_label_new_with_mnemonic ((const gchar *)ll);
        gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
        gtk_table_attach ( GTK_TABLE (t), l, x + pos, x + pos + 1, y, y + 1,
                           (GtkAttachOptions)0, (GtkAttachOptions)0, 0, 0 );
        gtk_widget_set_sensitive (l, sensitive);
        pos += 1;

    }

    GtkWidget *sb = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 1.0, digits);
    gtk_table_attach ( GTK_TABLE (t), sb, x + pos, x + pos + 1, y, y + 1,
                       (GtkAttachOptions)0, (GtkAttachOptions)0, 0, 0 );
    gtk_widget_set_size_request (sb, 80, -1);
    gtk_widget_set_sensitive (sb, sensitive);
    pos += 1;

    if (ll) { gtk_label_set_mnemonic_widget (GTK_LABEL(l), sb); }

    if (lr) {

        l = gtk_label_new_with_mnemonic ((const gchar *)lr);
        gtk_misc_set_alignment (GTK_MISC (l), 0.0, 0.5);
        gtk_table_attach ( GTK_TABLE (t), l, x + pos, x + pos + 1, y, y + 1,
                           (GtkAttachOptions)0, (GtkAttachOptions)0, 0, 0 );
        gtk_widget_set_sensitive (l, sensitive);
        pos += 1;

        gtk_label_set_mnemonic_widget (GTK_LABEL(l), sb);
    }

    if (cb)
        gtk_signal_connect (adj, "value_changed", cb, dlg);

    return;
} // end of sp_export_spinbutton_new()


static Gtk::VBox *
sp_export_dialog_area_box (GtkWidget * dlg)
{
    Gtk::VBox* vb = new Gtk::VBox(false, 3);

    Gtk::Label* lbl = new Gtk::Label(_("<big><b>Export area</b></big>"), Gtk::ALIGN_LEFT);
    lbl->set_use_markup(true);
    vb->pack_start(*lbl);

    /* Units box */
    Gtk::HBox* unitbox = new Gtk::HBox(false, 0);
    /* gets added to the vbox later, but the unit selector is needed
       earlier than that */

    Gtk::Widget* us = Glib::wrap(sp_unit_selector_new (SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE));
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop)
        sp_unit_selector_set_unit (SP_UNIT_SELECTOR(us->gobj()), sp_desktop_namedview(desktop)->doc_units);
    unitbox->pack_end(*us, false, false, 0);
    Gtk::Label* l = new Gtk::Label(_("Units:"));
    unitbox->pack_end(*l, false, false, 3);
    gtk_object_set_data (GTK_OBJECT (dlg), "units", us->gobj());

    Gtk::HBox* togglebox = new Gtk::HBox(true, 0);

    Gtk::ToggleButton* b;
    for (int i = 0; i < SELECTION_NUMBER_OF; i++) {
        b = new Gtk::ToggleButton(_(selection_labels[i]), true);
        b->set_data("key", GINT_TO_POINTER(i));
        gtk_object_set_data (GTK_OBJECT (dlg), selection_names[i], b->gobj());
        togglebox->pack_start(*b, false, true, 0);
        gtk_signal_connect ( GTK_OBJECT (b->gobj()), "clicked",
                             GTK_SIGNAL_FUNC (sp_export_area_toggled), dlg );
    }

    g_signal_connect ( G_OBJECT (INKSCAPE), "change_selection",
                       G_CALLBACK (sp_export_selection_changed), dlg );
    g_signal_connect ( G_OBJECT (INKSCAPE), "modify_selection",
                       G_CALLBACK (sp_export_selection_modified), dlg );
    g_signal_connect ( G_OBJECT (INKSCAPE), "activate_desktop",
                       G_CALLBACK (sp_export_selection_changed), dlg );

    Gtk::Table* t = new Gtk::Table(2, 6, FALSE);
    t->set_row_spacings (4);
    t->set_col_spacings (4);

    sp_export_spinbutton_new ( "x0", 0.0, -1000000.0, 1000000.0, 0.1, 1.0, us->gobj(),
                               GTK_WIDGET(t->gobj()), 0, 0, _("_x0:"), NULL, EXPORT_COORD_PRECISION, 1,
                               G_CALLBACK ( sp_export_area_x_value_changed),
                               dlg );

    sp_export_spinbutton_new ( "x1", 0.0, -1000000.0, 1000000.0, 0.1, 1.0, us->gobj(),
                               GTK_WIDGET(t->gobj()), 2, 0, _("x_1:"), NULL, EXPORT_COORD_PRECISION, 1,
                               G_CALLBACK (sp_export_area_x_value_changed),
                               dlg );

    sp_export_spinbutton_new ( "width", 0.0, 0.0, PNG_UINT_31_MAX, 0.1, 1.0,
                               us->gobj(), GTK_WIDGET(t->gobj()), 4, 0, _("Wid_th:"), NULL, EXPORT_COORD_PRECISION, 1,
                               G_CALLBACK
                                   (sp_export_area_width_value_changed),
                               dlg );

    sp_export_spinbutton_new ( "y0", 0.0, -1000000.0, 1000000.0, 0.1, 1.0, us->gobj(),
                               GTK_WIDGET(t->gobj()), 0, 1, _("_y0:"), NULL, EXPORT_COORD_PRECISION, 1,
                               G_CALLBACK (sp_export_area_y_value_changed),
                               dlg );

    sp_export_spinbutton_new ( "y1", 0.0, -1000000.0, 1000000.0, 0.1, 1.0, us->gobj(),
                               GTK_WIDGET(t->gobj()), 2, 1, _("y_1:"), NULL, EXPORT_COORD_PRECISION, 1,
                               G_CALLBACK (sp_export_area_y_value_changed),
                               dlg );

    sp_export_spinbutton_new ( "height", 0.0, 0.0, PNG_UINT_31_MAX, 0.1, 1.0,
                               us->gobj(), GTK_WIDGET(t->gobj()), 4, 1, _("Hei_ght:"), NULL, EXPORT_COORD_PRECISION, 1,
                               G_CALLBACK (sp_export_area_height_value_changed),
                               dlg );

    vb->pack_start(*togglebox, false, false, 3);
    vb->pack_start(*t, false, false, 0);
    vb->pack_start(*unitbox, false, false, 0);

    return vb;
} // end of sp_export_dialog_area_box


gchar* create_filepath_from_id (const gchar *id, const gchar *file_entry_text) {

    if (id == NULL) /* This should never happen */
        id = "bitmap";

    gchar * directory = NULL;

    if (directory == NULL && file_entry_text != NULL && file_entry_text[0] != '\0') {
        // std::cout << "Directory from dialog" << std::endl;
        directory = g_dirname(file_entry_text);
    }

    if (directory == NULL) {
        /* Grab document directory */
        if (SP_DOCUMENT_URI(SP_ACTIVE_DOCUMENT)) {
            // std::cout << "Directory from document" << std::endl;
            directory = g_dirname(SP_DOCUMENT_URI(SP_ACTIVE_DOCUMENT));
        }
    }

    if (directory == NULL) {
        // std::cout << "Home Directory" << std::endl;
        directory = homedir_path(NULL);
    }

    gchar * id_ext = g_strconcat(id, ".png", NULL);
    gchar *filename = g_build_filename(directory, id_ext, NULL);
    g_free(directory);
    g_free(id_ext);
    return filename;
}

static void
batch_export_clicked (GtkWidget *widget, GtkObject *base)
{
    Gtk::Widget *vb_singleexport = (Gtk::Widget *)gtk_object_get_data(base, "vb_singleexport");
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(widget))) {
        vb_singleexport->set_sensitive(false);
    } else {
        vb_singleexport->set_sensitive(true);
    }
}

void
sp_export_dialog (void)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (!dlg) {

        gchar title[500];
        sp_ui_dialog_title_string (Inkscape::Verb::get(SP_VERB_FILE_EXPORT), title);

        dlg = sp_window_new (title, TRUE);

        if (x == -1000 || y == -1000) {
            x = prefs->getInt(prefs_path + "x", 0);
            y = prefs->getInt(prefs_path + "y", 0);
        }

        if (w ==0 || h == 0) {
            w = prefs->getInt(prefs_path + "w", 0);
            h = prefs->getInt(prefs_path + "h", 0);
        }

//        if (x<0) x=0;
//        if (y<0) y=0;

        if (w && h) gtk_window_resize ((GtkWindow *) dlg, w, h);
        if (x >= 0 && y >= 0 && (x < (gdk_screen_width()-MIN_ONSCREEN_DISTANCE)) && (y < (gdk_screen_height()-MIN_ONSCREEN_DISTANCE)))
            gtk_window_move ((GtkWindow *) dlg, x, y);
        else
            gtk_window_set_position(GTK_WINDOW(dlg), GTK_WIN_POS_CENTER);
        sp_transientize (dlg);
        wd.win = dlg;
        wd.stop = 0;

        g_signal_connect   ( G_OBJECT (INKSCAPE), "activate_desktop",
                             G_CALLBACK (sp_transientize_callback), &wd);

        gtk_signal_connect ( GTK_OBJECT (dlg), "event",
                             GTK_SIGNAL_FUNC (sp_dialog_event_handler), dlg);

        gtk_signal_connect ( GTK_OBJECT (dlg), "destroy",
                             G_CALLBACK (sp_export_dialog_destroy), dlg);

        gtk_signal_connect ( GTK_OBJECT (dlg), "delete_event",
                             G_CALLBACK (sp_export_dialog_delete), dlg);

        g_signal_connect   ( G_OBJECT (INKSCAPE), "shut_down",
                             G_CALLBACK (sp_export_dialog_delete), dlg);

        g_signal_connect   ( G_OBJECT (INKSCAPE), "dialogs_hide",
                             G_CALLBACK (sp_dialog_hide), dlg);

        g_signal_connect   ( G_OBJECT (INKSCAPE), "dialogs_unhide",
                             G_CALLBACK (sp_dialog_unhide), dlg);

        GtkTooltips *tt = gtk_tooltips_new();

        Gtk::VBox *vb = new Gtk::VBox(false, 3);
        vb->set_border_width(3);
        gtk_container_add (GTK_CONTAINER (dlg), GTK_WIDGET(vb->gobj()));

        Gtk::VBox *vb_singleexport = new Gtk::VBox(false, 0);
        vb_singleexport->set_border_width(0);
        vb->pack_start(*vb_singleexport);
        gtk_object_set_data(GTK_OBJECT(dlg), "vb_singleexport", vb_singleexport);

        /* Export area frame */
        {
            Gtk::VBox *area_box = sp_export_dialog_area_box(dlg);
            area_box->set_border_width(3);
            vb_singleexport->pack_start(*area_box, false, false, 0);
        }

        /* Bitmap size frame */
        {
            Gtk::VBox *size_box = new Gtk::VBox(false, 3);
            size_box->set_border_width(3);

            Gtk::Label* lbl = new Gtk::Label(_("<big><b>Bitmap size</b></big>"), Gtk::ALIGN_LEFT);
            lbl->set_use_markup(true);
            size_box->pack_start(*lbl, false, false, 0);
            const int rows = 2;
            const int cols = 5;
            const bool homogeneous = false;
            Gtk::Table *t = new Gtk::Table(rows, cols, homogeneous);
            t->set_row_spacings (4);
            t->set_col_spacings (4);
            size_box->pack_start(*t);

            sp_export_spinbutton_new ( "bmwidth", 16.0, 1.0, 1000000.0, 1.0, 10.0,
                                       NULL, GTK_WIDGET(t->gobj()), 0, 0,
                                       _("_Width:"), _("pixels at"), 0, 1,
                                       G_CALLBACK
                                       (sp_export_bitmap_width_value_changed),
                                       dlg );

            sp_export_spinbutton_new ( "xdpi",
                                       prefs->getDouble("/dialogs/export/defaultxdpi/value", DPI_BASE),
                                       0.01, 100000.0, 0.1, 1.0, NULL, GTK_WIDGET(t->gobj()), 3, 0,
                                       NULL, _("dp_i"), 2, 1,
                                       G_CALLBACK (sp_export_xdpi_value_changed),
                                       dlg );

            sp_export_spinbutton_new ( "bmheight", 16.0, 1.0, 1000000.0, 1.0, 10.0,
                                       NULL, GTK_WIDGET(t->gobj()), 0, 1,
                                       _("_Height:"), _("pixels at"), 0, 1,
                                       G_CALLBACK
                                       (sp_export_bitmap_height_value_changed),
                                       dlg );

            /** \todo
             * Needs fixing: there's no way to set ydpi currently, so we use
             *       the defaultxdpi value here, too...
             */
            sp_export_spinbutton_new ( "ydpi", prefs->getDouble("/dialogs/export/defaultxdpi/value", DPI_BASE),
                                       0.01, 100000.0, 0.1, 1.0, NULL, GTK_WIDGET(t->gobj()), 3, 1,
                                       NULL, _("dpi"), 2, 0, NULL, dlg );

            vb_singleexport->pack_start(*size_box);
        }

        /* File entry */
        {
            Gtk::VBox* file_box = new Gtk::VBox(false, 3);
            file_box->set_border_width(3);

            // true = has mnemonic
            Gtk::Label *flabel = new Gtk::Label(_("<big><b>_Filename</b></big>"), Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER, true);
            flabel->set_use_markup(true);
            file_box->pack_start(*flabel, false, false, 0);

            Gtk::Entry *fe = new Gtk::Entry();

            /*
             * set the default filename to be that of the current path + document
             * with .png extension
             *
             * One thing to notice here is that this filename may get
             * overwritten, but it won't happen here.  The filename gets
             * written into the text field, but then the button to select
             * the area gets set.  In that code the filename can be changed
             * if there are some with presidence in the document.  So, while
             * this code sets the name first, it may not be the one users
             * really see.
             */
            if (SP_ACTIVE_DOCUMENT && SP_DOCUMENT_URI (SP_ACTIVE_DOCUMENT))
            {
                gchar *name;
                SPDocument * doc = SP_ACTIVE_DOCUMENT;
                const gchar *uri = SP_DOCUMENT_URI (doc);
                const gchar *text_extension = get_file_save_extension (Inkscape::Extension::FILE_SAVE_METHOD_SAVE_AS).c_str();
                Inkscape::Extension::Output * oextension = NULL;

                if (text_extension != NULL) {
                    oextension = dynamic_cast<Inkscape::Extension::Output *>(Inkscape::Extension::db.get(text_extension));
                }

                if (oextension != NULL) {
                    gchar * old_extension = oextension->get_extension();
                    if (g_str_has_suffix(uri, old_extension)) {
                        gchar * uri_copy;
                        gchar * extension_point;
                        gchar * final_name;

                        uri_copy = g_strdup(uri);
                        extension_point = g_strrstr(uri_copy, old_extension);
                        extension_point[0] = '\0';

                        final_name = g_strconcat(uri_copy, ".png", NULL);
                        fe->set_text(final_name);

                        g_free(final_name);
                        g_free(uri_copy);
                    }
                } else {
                    name = g_strconcat(uri, ".png", NULL);
                    fe->set_text(name);
                    g_free(name);
                }

                doc_export_name = g_strdup(fe->get_text().c_str());
            }
            g_signal_connect ( G_OBJECT (fe->gobj()), "changed",
                               G_CALLBACK (sp_export_filename_modified), dlg);

            Gtk::HBox *hb = new Gtk::HBox(FALSE, 5);

            {
                // true = has mnemonic
                Gtk::Button *b = new Gtk::Button();

                Gtk::HBox* pixlabel = new Gtk::HBox(false, 3);
                Gtk::Image *im = new Gtk::Image(Gtk::StockID(Gtk::Stock::INDEX),
                        Gtk::ICON_SIZE_BUTTON);
                pixlabel->pack_start(*im);

                Gtk::Label *l = new Gtk::Label();
                l->set_markup_with_mnemonic(_("_Browse..."));
                pixlabel->pack_start(*l);

                b->add(*pixlabel);

                hb->pack_end (*b, false, false, 4);
                g_signal_connect ( G_OBJECT (b->gobj()), "clicked",
                                   G_CALLBACK (sp_export_browse_clicked), NULL );
            }

            hb->pack_start (*fe, true, true, 0);
            file_box->add(*hb);
            gtk_object_set_data (GTK_OBJECT (dlg), "filename", fe->gobj());
            gtk_object_set_data (GTK_OBJECT (dlg), "filename-modified", (gpointer)FALSE);
            original_name = g_strdup(fe->get_text().c_str());
            // pressing enter in the filename field is the same as clicking export:
            g_signal_connect ( G_OBJECT (fe->gobj()), "activate",
                               G_CALLBACK (sp_export_export_clicked), dlg );
            // focus is in the filename initially:
            fe->grab_focus();

            // mnemonic in frame label moves focus to filename:
            flabel->set_mnemonic_widget(*fe);

            vb_singleexport->pack_start(*file_box);
        }

        {
            Gtk::HBox* batch_box = new Gtk::HBox(FALSE, 5);
            GtkWidget *be = gtk_check_button_new_with_label(_("Batch export all selected objects"));
            gtk_widget_set_sensitive(GTK_WIDGET(be), TRUE);
            gtk_object_set_data(GTK_OBJECT(dlg), "batch_checkbox", be);
            batch_box->pack_start(*Glib::wrap(be), false, false);
            gtk_tooltips_set_tip(tt, be, _("Export each selected object into its own PNG file, using export hints if any (caution, overwrites without asking!)"), NULL);
            batch_box->show_all();
            g_signal_connect(G_OBJECT(be), "toggled", GTK_SIGNAL_FUNC(batch_export_clicked), dlg);
            vb->pack_start(*batch_box);
        }

        {
            Gtk::HBox* hide_box = new Gtk::HBox(FALSE, 5);
            GtkWidget *he = gtk_check_button_new_with_label(_("Hide all except selected"));
            gtk_widget_set_sensitive(GTK_WIDGET(he), TRUE);
            gtk_object_set_data(GTK_OBJECT(dlg), "hide_checkbox", he);
            hide_box->pack_start(*Glib::wrap(he), false, false);
            gtk_tooltips_set_tip(tt, he, _("In the exported image, hide all objects except those that are selected"), NULL);
            hide_box->show_all();
            vb->pack_start(*hide_box);
        }

        /* Buttons */
        Gtk::HButtonBox* bb = new Gtk::HButtonBox(Gtk::BUTTONBOX_END);
        bb->set_border_width(3);

        {
            Gtk::Button *b = new Gtk::Button();
            Gtk::HBox* image_label = new Gtk::HBox(false, 3);
            Gtk::Image *im = new Gtk::Image(Gtk::StockID(Gtk::Stock::APPLY),
                    Gtk::ICON_SIZE_BUTTON);
            image_label->pack_start(*im);

            Gtk::Label *l = new Gtk::Label();
            l->set_markup_with_mnemonic(_("_Export"));
            image_label->pack_start(*l);

            b->add(*image_label);
            gtk_tooltips_set_tip (tt, GTK_WIDGET(b->gobj()), _("Export the bitmap file with these settings"), NULL);
            gtk_signal_connect ( GTK_OBJECT (b->gobj()), "clicked",
                                 GTK_SIGNAL_FUNC (sp_export_export_clicked), dlg );
            bb->pack_end(*b, false, false, 0);
        }

        vb->pack_end(*bb, false, false, 0);
        vb->show_all();

    } // end of if (!dlg)

    sp_export_find_default_selection(dlg);

    gtk_window_present ((GtkWindow *) dlg);

    return;
} // end of sp_export_dialog()

static void
sp_export_update_checkbuttons (GtkObject *base)
{
    gint num = g_slist_length((GSList *) sp_desktop_selection(SP_ACTIVE_DESKTOP)->itemList());
    GtkWidget *be = (GtkWidget *)gtk_object_get_data(base, "batch_checkbox");
    GtkWidget *he = (GtkWidget *)gtk_object_get_data(base, "hide_checkbox");
    if (num >= 2) {
        gtk_widget_set_sensitive (be, true);
        gtk_button_set_label (GTK_BUTTON(be), g_strdup_printf (ngettext("Batch export %d selected object","Batch export %d selected objects",num), num));
    } else {
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(be), FALSE);
        gtk_widget_set_sensitive (be, FALSE);
    }
    if (num > 0) {
        gtk_widget_set_sensitive (he, true);
    } else {
        gtk_widget_set_sensitive (he, false);
    }
}

static inline void
sp_export_find_default_selection(GtkWidget * dlg)
{
    selection_type key = SELECTION_NUMBER_OF;

    if ((sp_desktop_selection(SP_ACTIVE_DESKTOP))->isEmpty() == false) {
        key = SELECTION_SELECTION;
    }

    /* Try using the preferences */
    if (key == SELECTION_NUMBER_OF) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        int i = SELECTION_NUMBER_OF;

        Glib::ustring what = prefs->getString("/dialogs/export/exportarea/value");

        if (!what.empty()) {
            for (i = 0; i < SELECTION_NUMBER_OF; i++) {
                if (what == selection_names[i]) {
                    break;
                }
            }
        }

        key = (selection_type)i;
    }

    if (key == SELECTION_NUMBER_OF) {
        key = SELECTION_SELECTION;
    }

    GtkWidget *button = (GtkWidget *)g_object_get_data(G_OBJECT(dlg),
                                                       selection_names[key]);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);

    sp_export_update_checkbuttons (GTK_OBJECT(dlg));
}


/**
 * \brief  If selection changed or a different document activated, we must
 * recalculate any chosen areas
 *
 */
static void
sp_export_selection_changed ( Inkscape::Application *inkscape,
                              Inkscape::Selection *selection,
                              GtkObject *base )
{
    selection_type current_key;
    current_key = (selection_type)(GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(base), "selection-type")));

    if ((current_key == SELECTION_DRAWING || current_key == SELECTION_PAGE) &&
            (sp_desktop_selection(SP_ACTIVE_DESKTOP))->isEmpty() == false &&
            was_empty) {
        gtk_toggle_button_set_active
            ( GTK_TOGGLE_BUTTON ( gtk_object_get_data (base, selection_names[SELECTION_SELECTION])),
              TRUE );
    }
    was_empty = (sp_desktop_selection(SP_ACTIVE_DESKTOP))->isEmpty();

    current_key = (selection_type)(GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(base), "selection-type")));

    if (inkscape &&
            SP_IS_INKSCAPE (inkscape) &&
            selection &&
            SELECTION_CUSTOM != current_key) {
        GtkToggleButton * button;
        button = (GtkToggleButton *)gtk_object_get_data(base, selection_names[current_key]);
        sp_export_area_toggled(button, base);
    }

    sp_export_update_checkbuttons (base);
}

static void
sp_export_selection_modified ( Inkscape::Application */*inkscape*/,
                               Inkscape::Selection */*selection*/,
                               guint /*flags*/,
                               GtkObject *base )
{
    selection_type current_key;
    current_key = (selection_type)(GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(base), "selection-type")));

    switch (current_key) {
        case SELECTION_DRAWING:
            if ( SP_ACTIVE_DESKTOP ) {
                SPDocument *doc;
                doc = sp_desktop_document (SP_ACTIVE_DESKTOP);
                Geom::OptRect bbox = sp_item_bbox_desktop (SP_ITEM (SP_DOCUMENT_ROOT (doc)));
                if (bbox) {
                    sp_export_set_area (base, bbox->min()[Geom::X],
                                              bbox->min()[Geom::Y],
                                              bbox->max()[Geom::X],
                                              bbox->max()[Geom::Y]);
                }
            }
            break;
        case SELECTION_SELECTION:
            if ((sp_desktop_selection(SP_ACTIVE_DESKTOP))->isEmpty() == false) {
                NRRect bbox;
                (sp_desktop_selection (SP_ACTIVE_DESKTOP))->bounds(&bbox);
                sp_export_set_area (base, bbox.x0, bbox.y0, bbox.x1, bbox.y1);
            }
            break;
        default:
            /* Do nothing for page or for custom */
            break;
    }

    return;
}

/// Called when one of the selection buttons was toggled.
static void
sp_export_area_toggled (GtkToggleButton *tb, GtkObject *base)
{
    if (gtk_object_get_data (base, "update"))
        return;

    selection_type key, old_key;
    key = (selection_type)(GPOINTER_TO_INT(gtk_object_get_data (GTK_OBJECT (tb), "key")));
    old_key = (selection_type)(GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(base), "selection-type")));

    /* Ignore all "turned off" events unless we're the only active button */
    if (!gtk_toggle_button_get_active (tb) ) {

        /* Don't let the current selection be deactived - but rerun the
           activate to allow the user to renew the values */
        if (key == old_key) {
            gtk_toggle_button_set_active ( tb, TRUE );
        }

        return;
    }

    /* Turn off the currently active button unless it's us */
    gtk_object_set_data(GTK_OBJECT(base), "selection-type", (gpointer)key);

    if (old_key != key) {
        gtk_toggle_button_set_active
            ( GTK_TOGGLE_BUTTON ( gtk_object_get_data (base, selection_names[old_key])),
              FALSE );
    }

    if ( SP_ACTIVE_DESKTOP )
    {
        SPDocument *doc;
        Geom::OptRect bbox;
        doc = sp_desktop_document (SP_ACTIVE_DESKTOP);

        /* Notice how the switch is used to 'fall through' here to get
           various backups.  If you modify this without noticing you'll
           probabaly screw something up. */
        switch (key) {
            case SELECTION_SELECTION:
                if ((sp_desktop_selection(SP_ACTIVE_DESKTOP))->isEmpty() == false)
                {
                    bbox = sp_desktop_selection (SP_ACTIVE_DESKTOP)->bounds();
                    /* Only if there is a selection that we can set
                       do we break, otherwise we fall through to the
                       drawing */
                    // std::cout << "Using selection: SELECTION" << std::endl;
                    key = SELECTION_SELECTION;
                    break;
                }
            case SELECTION_DRAWING:
                /** \todo
                 * This returns wrong values if the document has a viewBox.
                 */
                bbox = sp_item_bbox_desktop (SP_ITEM (SP_DOCUMENT_ROOT (doc)));
                /* If the drawing is valid, then we'll use it and break
                   otherwise we drop through to the page settings */
                if (bbox) {
                    // std::cout << "Using selection: DRAWING" << std::endl;
                    key = SELECTION_DRAWING;
                    break;
                }
            case SELECTION_PAGE:
                bbox = Geom::Rect(Geom::Point(0.0, 0.0),
                                  Geom::Point(sp_document_width(doc), sp_document_height(doc)));

                // std::cout << "Using selection: PAGE" << std::endl;
                key = SELECTION_PAGE;
                break;
            case SELECTION_CUSTOM:
            default:
                break;
        } // switch

        // remember area setting
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setString("/dialogs/export/exportarea/value", selection_names[key]);

        if ( key != SELECTION_CUSTOM && bbox ) {
            sp_export_set_area (base, bbox->min()[Geom::X],
                                      bbox->min()[Geom::Y],
                                      bbox->max()[Geom::X],
                                      bbox->max()[Geom::Y]);
        }

    } // end of if ( SP_ACTIVE_DESKTOP )


    if (SP_ACTIVE_DESKTOP && !gtk_object_get_data(GTK_OBJECT(base), "filename-modified")) {
        GtkWidget * file_entry;
        const gchar * filename = NULL;
        float xdpi = 0.0, ydpi = 0.0;

        file_entry = (GtkWidget *)gtk_object_get_data (base, "filename");

        switch (key) {
            case SELECTION_PAGE:
            case SELECTION_DRAWING: {
                SPDocument * doc = SP_ACTIVE_DOCUMENT;
                sp_document_get_export_hints (doc, &filename, &xdpi, &ydpi);

                if (filename == NULL) {
                    if (doc_export_name != NULL) {
                        filename = g_strdup(doc_export_name);
                    } else {
                        filename = g_strdup("");
                    }
                }
                break;
            }
            case SELECTION_SELECTION:
                if ((sp_desktop_selection(SP_ACTIVE_DESKTOP))->isEmpty() == false) {

                    sp_selection_get_export_hints (sp_desktop_selection(SP_ACTIVE_DESKTOP), &filename, &xdpi, &ydpi);

                    /* If we still don't have a filename -- let's build
                       one that's nice */
                    if (filename == NULL) {
                        const gchar * id = NULL;
                        const GSList * reprlst = sp_desktop_selection(SP_ACTIVE_DESKTOP)->reprList();
                        for(; reprlst != NULL; reprlst = reprlst->next) {
                            Inkscape::XML::Node * repr = (Inkscape::XML::Node *)reprlst->data;
                            if (repr->attribute("id")) {
                                id = repr->attribute("id");
                                break;
                            }
                        }

                        filename = create_filepath_from_id (id, gtk_entry_get_text(GTK_ENTRY(file_entry)));
                    }
                }
                break;
            case SELECTION_CUSTOM:
            default:
                break;
        }

        if (filename != NULL) {
            g_free(original_name);
            original_name = g_strdup(filename);
            gtk_entry_set_text(GTK_ENTRY(file_entry), filename);
        }

        if (xdpi != 0.0) {
            sp_export_value_set(base, "xdpi", xdpi);
        }

        /* These can't be separate, and setting x sets y, so for
           now setting this is disabled.  Hopefully it won't be in
           the future */
        if (FALSE && ydpi != 0.0) {
            sp_export_value_set(base, "ydpi", ydpi);
        }
    }

    return;
} // end of sp_export_area_toggled()

/// Called when dialog is deleted
static gint
sp_export_progress_delete ( GtkWidget */*widget*/, GdkEvent */*event*/, GObject *base )
{
    g_object_set_data (base, "cancel", (gpointer) 1);
    return TRUE;
} // end of sp_export_progress_delete()

/// Called when progress is cancelled
static void
sp_export_progress_cancel ( GtkWidget */*widget*/, GObject *base )
{
    g_object_set_data (base, "cancel", (gpointer) 1);
} // end of sp_export_progress_cancel()

/// Called for every progress iteration
static unsigned int
sp_export_progress_callback (float value, void *data)
{
    GtkWidget *prg;
    int evtcount;

    if (g_object_get_data ((GObject *) data, "cancel"))
        return FALSE;

    prg = (GtkWidget *) g_object_get_data ((GObject *) data, "progress");
    gtk_progress_bar_set_fraction ((GtkProgressBar *) prg, value);

    evtcount = 0;
    while ((evtcount < 16) && gdk_events_pending ()) {
            gtk_main_iteration_do (FALSE);
            evtcount += 1;
    }

    gtk_main_iteration_do (FALSE);

    return TRUE;

} // end of sp_export_progress_callback()

GtkWidget *
create_progress_dialog (GtkObject *base, gchar *progress_text) {
    GtkWidget *dlg, *prg, *btn; /* progressbar-stuff */

    dlg = gtk_dialog_new ();
    gtk_window_set_title (GTK_WINDOW (dlg), _("Export in progress"));
    prg = gtk_progress_bar_new ();
    sp_transientize (dlg);
    gtk_window_set_resizable (GTK_WINDOW (dlg), FALSE);
    g_object_set_data ((GObject *) base, "progress", prg);

    gtk_progress_bar_set_text ((GtkProgressBar *) prg, progress_text);

    gtk_progress_bar_set_orientation ( (GtkProgressBar *) prg,
                                       GTK_PROGRESS_LEFT_TO_RIGHT);
    gtk_box_pack_start ((GtkBox *) ((GtkDialog *) dlg)->vbox,
                        prg, FALSE, FALSE, 4 );
    btn = gtk_dialog_add_button ( GTK_DIALOG (dlg),
                                  GTK_STOCK_CANCEL,
                                  GTK_RESPONSE_CANCEL );

    g_signal_connect ( (GObject *) dlg, "delete_event",
                       (GCallback) sp_export_progress_delete, base);
    g_signal_connect ( (GObject *) btn, "clicked",
                       (GCallback) sp_export_progress_cancel, base);
    gtk_window_set_modal ((GtkWindow *) dlg, TRUE);
    gtk_widget_show_all (dlg);

    return dlg;
}

// FIXME: Some lib function should be available to do this ...
static gchar *
filename_add_extension (const gchar *filename, const gchar *extension)
{
  const gchar *dot;

  dot = strrchr (filename, '.');
  if ( !dot )
    return g_strconcat (filename, ".", extension, NULL);
  {
    if (dot[1] == '\0')
      return g_strconcat (filename, extension, NULL);
    else
    {
      if (g_strcasecmp (dot + 1, extension) == 0)
        return g_strdup (filename);
      else
      {
        return g_strconcat (filename, ".", extension, NULL);
      }
    }
  }
}

gchar *absolutize_path_from_document_location (SPDocument *doc, const gchar *filename)
{
    gchar *path = 0;
    //Make relative paths go from the document location, if possible:
    if (!g_path_is_absolute(filename) && doc->uri) {
        gchar *dirname = g_path_get_dirname(doc->uri);
        if (dirname) {
            path = g_build_filename(dirname, filename, NULL);
            g_free(dirname);
        }
    }
    if (!path) {
        path = g_strdup(filename);
    }
    return path;
}

/// Called when export button is clicked
static void
sp_export_export_clicked (GtkButton */*button*/, GtkObject *base)
{
    if (!SP_ACTIVE_DESKTOP) return;

    SPNamedView *nv = sp_desktop_namedview(SP_ACTIVE_DESKTOP);
    SPDocument *doc = sp_desktop_document (SP_ACTIVE_DESKTOP);

    GtkWidget *be = (GtkWidget *)gtk_object_get_data(base, "batch_checkbox");
    GtkWidget *he = (GtkWidget *)gtk_object_get_data(base, "hide_checkbox");
    bool hide = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (he));
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (be))) {
        // Batch export of selected objects

        gint num = g_slist_length((GSList *) sp_desktop_selection(SP_ACTIVE_DESKTOP)->itemList());
        gint n = 0;

        if (num < 1)
            return;

        gchar *progress_text = g_strdup_printf (_("Exporting %d files"), num);
        GtkWidget *prog_dlg = create_progress_dialog (base, progress_text);
        g_free (progress_text);

        for (GSList *i = (GSList *) sp_desktop_selection(SP_ACTIVE_DESKTOP)->itemList();
             i != NULL;
             i = i->next) {
            SPItem *item = (SPItem *) i->data;

            // retrieve export filename hint
            const gchar *filename = SP_OBJECT_REPR(item)->attribute("inkscape:export-filename");
            gchar *path = 0;
            if (!filename) {
                path = create_filepath_from_id (SP_OBJECT_ID(item), NULL);
            } else {
                path = absolutize_path_from_document_location(doc, filename);
            }

            // retrieve export dpi hints
            const gchar *dpi_hint = SP_OBJECT_REPR(item)->attribute("inkscape:export-xdpi"); // only xdpi, ydpi is always the same now
            gdouble dpi = 0.0;
            if (dpi_hint) {
                dpi = atof(dpi_hint);
            }
            if (dpi == 0.0) {
                dpi = DPI_BASE;
            }

            Geom::OptRect area;
            sp_item_invoke_bbox(item, area, sp_item_i2d_affine((SPItem *) item), TRUE);
            if (area) {
                gint width = (gint) (area->width() * dpi / PX_PER_IN + 0.5);
                gint height = (gint) (area->height() * dpi / PX_PER_IN + 0.5);

                if (width > 1 && height > 1) {
                    /* Do export */
                    if (!sp_export_png_file (doc, path,
                                             *area, width, height, dpi, dpi,
                                             nv->pagecolor,
                                             NULL, NULL, TRUE,  // overwrite without asking
                                             hide ? (GSList *) sp_desktop_selection(SP_ACTIVE_DESKTOP)->itemList() : NULL
                            )) {
                        gchar * error;
                        gchar * safeFile = Inkscape::IO::sanitizeString(path);
                        error = g_strdup_printf(_("Could not export to filename %s.\n"), safeFile);
                        sp_ui_error_dialog(error);
                        g_free(safeFile);
                        g_free(error);
                    }
                }
            }
            n++;
            g_free(path);
            sp_export_progress_callback((float)n/num, base);
        }

        gtk_widget_destroy (prog_dlg);
        g_object_set_data (G_OBJECT (base), "cancel", (gpointer) 0);

    } else {

    GtkWidget *fe = (GtkWidget *)gtk_object_get_data(base, "filename");
    gchar const *filename = gtk_entry_get_text(GTK_ENTRY(fe));

    float const x0 = sp_export_value_get_px(base, "x0");
    float const y0 = sp_export_value_get_px(base, "y0");
    float const x1 = sp_export_value_get_px(base, "x1");
    float const y1 = sp_export_value_get_px(base, "y1");
    float const xdpi = sp_export_value_get(base, "xdpi");
    float const ydpi = sp_export_value_get(base, "ydpi");
    unsigned long int const width = int(sp_export_value_get(base, "bmwidth") + 0.5);
    unsigned long int const height = int(sp_export_value_get(base, "bmheight") + 0.5);

    if (filename == NULL || *filename == '\0') {
        sp_ui_error_dialog(_("You have to enter a filename"));
        return;
    }

    if (!((x1 > x0) && (y1 > y0) && (width > 0) && (height > 0))) {
        sp_ui_error_dialog (_("The chosen area to be exported is invalid"));
        return;
    }

    // make sure that .png is the extension of the file:
    gchar * filename_ext = filename_add_extension(filename, "png");
    gtk_entry_set_text(GTK_ENTRY(fe), filename_ext);

    gchar *path = absolutize_path_from_document_location(doc, filename_ext);

    gchar *dirname = g_path_get_dirname(path);
    if ( dirname == NULL
         || !Inkscape::IO::file_test(dirname, (GFileTest)(G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR)) )
    {
        gchar *safeDir = Inkscape::IO::sanitizeString(dirname);
        gchar *error = g_strdup_printf(_("Directory %s does not exist or is not a directory.\n"),
                                       safeDir);
        sp_ui_error_dialog(error);
        g_free(safeDir);
        g_free(error);
        g_free(dirname);
        g_free(path);
        return;
    }
    g_free(dirname);

    gchar *fn = g_path_get_basename (path);
    gchar *progress_text = g_strdup_printf (_("Exporting %s (%lu x %lu)"), fn, width, height);
    g_free (fn);

    GtkWidget *prog_dlg = create_progress_dialog (base, progress_text);
    g_free (progress_text);

    /* Do export */
    if (!sp_export_png_file (sp_desktop_document (SP_ACTIVE_DESKTOP), path,
                             Geom::Rect(Geom::Point(x0, y0), Geom::Point(x1, y1)), width, height, xdpi, ydpi,
                             nv->pagecolor,
                             sp_export_progress_callback, base, FALSE,
                             hide ? (GSList *) sp_desktop_selection(SP_ACTIVE_DESKTOP)->itemList() : NULL
            )) {
        gchar * error;
        gchar * safeFile = Inkscape::IO::sanitizeString(path);
        error = g_strdup_printf(_("Could not export to filename %s.\n"), safeFile);
        sp_ui_error_dialog(error);
        g_free(safeFile);
        g_free(error);
    }

    /* Reset the filename so that it can be changed again by changing
       selections and all that */
    g_free(original_name);
    original_name = g_strdup(filename_ext);
    gtk_object_set_data (GTK_OBJECT (base), "filename-modified", (gpointer)FALSE);

    gtk_widget_destroy (prog_dlg);
    g_object_set_data (G_OBJECT (base), "cancel", (gpointer) 0);

    /* Setup the values in the document */
    switch ((selection_type)(GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(base), "selection-type")))) {
        case SELECTION_PAGE:
        case SELECTION_DRAWING: {
            SPDocument * doc = SP_ACTIVE_DOCUMENT;
            Inkscape::XML::Node * repr = sp_document_repr_root(doc);
            bool modified = false;
            const gchar * temp_string;

            bool saved = sp_document_get_undo_sensitive(doc);
            sp_document_set_undo_sensitive(doc, false);

            temp_string = repr->attribute("inkscape:export-filename");
            if (temp_string == NULL || strcmp(temp_string, filename_ext)) {
                repr->setAttribute("inkscape:export-filename", filename_ext);
                modified = true;
            }
            temp_string = repr->attribute("inkscape:export-xdpi");
            if (temp_string == NULL || xdpi != atof(temp_string)) {
                sp_repr_set_svg_double(repr, "inkscape:export-xdpi", xdpi);
                modified = true;
            }
            temp_string = repr->attribute("inkscape:export-ydpi");
            if (temp_string == NULL || xdpi != atof(temp_string)) {
                sp_repr_set_svg_double(repr, "inkscape:export-ydpi", ydpi);
                modified = true;
            }
            sp_document_set_undo_sensitive(doc, saved);

            if (modified) {
                doc->setModifiedSinceSave();
            }
            break;
        }
        case SELECTION_SELECTION: {
            const GSList * reprlst;
            SPDocument * doc = SP_ACTIVE_DOCUMENT;
            bool modified = false;

            bool saved = sp_document_get_undo_sensitive(doc);
            sp_document_set_undo_sensitive(doc, false);
            reprlst = sp_desktop_selection(SP_ACTIVE_DESKTOP)->reprList();

            for(; reprlst != NULL; reprlst = reprlst->next) {
                Inkscape::XML::Node * repr = (Inkscape::XML::Node *)reprlst->data;
                const gchar * temp_string;

                if (repr->attribute("id") == NULL ||
                        !(g_strrstr(filename_ext, repr->attribute("id")) != NULL &&
                          (!SP_DOCUMENT_URI(SP_ACTIVE_DOCUMENT) ||
                            strcmp(g_dirname(filename), g_dirname(SP_DOCUMENT_URI(SP_ACTIVE_DOCUMENT))) == 0))) {
                    temp_string = repr->attribute("inkscape:export-filename");
                    if (temp_string == NULL || strcmp(temp_string, filename_ext)) {
                        repr->setAttribute("inkscape:export-filename", filename_ext);
                        modified = true;
                    }
                }
                temp_string = repr->attribute("inkscape:export-xdpi");
                if (temp_string == NULL || xdpi != atof(temp_string)) {
                    sp_repr_set_svg_double(repr, "inkscape:export-xdpi", xdpi);
                    modified = true;
                }
                temp_string = repr->attribute("inkscape:export-ydpi");
                if (temp_string == NULL || xdpi != atof(temp_string)) {
                    sp_repr_set_svg_double(repr, "inkscape:export-ydpi", ydpi);
                    modified = true;
                }
            }
            sp_document_set_undo_sensitive(doc, saved);

            if (modified) {
                doc->setModifiedSinceSave();
            }
            break;
        }
        default:
            break;
    }

    g_free (filename_ext);
    g_free (path);

    }

} // end of sp_export_export_clicked()

/// Called when Browse button is clicked
/// @todo refactor this code to use ui/dialogs/filedialog.cpp
static void
sp_export_browse_clicked (GtkButton */*button*/, gpointer /*userdata*/)
{
    GtkWidget *fs, *fe;
    const gchar *filename;

    fs = gtk_file_chooser_dialog_new (_("Select a filename for exporting"),
                                      (GtkWindow*)dlg,
                                      GTK_FILE_CHOOSER_ACTION_SAVE,
                                      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                      NULL );

#ifdef WITH_GNOME_VFS
    if (gnome_vfs_initialized()) {
        gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(fs), false);
    }
#endif

    fe = (GtkWidget *)g_object_get_data (G_OBJECT (dlg), "filename");

    sp_transientize (fs);

    gtk_window_set_modal(GTK_WINDOW (fs), true);

    filename = gtk_entry_get_text (GTK_ENTRY (fe));

    if (*filename == '\0') {
        filename = create_filepath_from_id(NULL, NULL);
    }

    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (fs), filename);

#ifdef WIN32
	// code in this section is borrowed from ui/dialogs/filedialogimpl-win32.cpp
	OPENFILENAMEW opf;
	WCHAR* filter_string = (WCHAR*)g_utf8_to_utf16("PNG\0*.png\0\0", 12, NULL, NULL, NULL);
	WCHAR* title_string = (WCHAR*)g_utf8_to_utf16(_("Select a filename for exporting"), -1, NULL, NULL, NULL);
	WCHAR* extension_string = (WCHAR*)g_utf8_to_utf16("*.png", -1, NULL, NULL, NULL);
	// Copy the selected file name, converting from UTF-8 to UTF-16
	WCHAR _filename[_MAX_PATH + 1];
    memset(_filename, 0, sizeof(_filename));
    gunichar2* utf16_path_string = g_utf8_to_utf16(filename, -1, NULL, NULL, NULL);
    wcsncpy(_filename, (wchar_t*)utf16_path_string, _MAX_PATH);
    g_free(utf16_path_string);

	opf.hwndOwner = (HWND)(GDK_WINDOW_HWND(GTK_WIDGET(dlg)->window));
	opf.lpstrFilter = filter_string;
	opf.lpstrCustomFilter = 0;
	opf.nMaxCustFilter = 0L;
	opf.nFilterIndex = 1L;
	opf.lpstrFile = _filename;
	opf.nMaxFile = _MAX_PATH;
	opf.lpstrFileTitle = NULL;
	opf.nMaxFileTitle=0;
	opf.lpstrInitialDir = 0;
	opf.lpstrTitle = title_string;
	opf.nFileOffset = 0;
	opf.nFileExtension = 2;
	opf.lpstrDefExt = extension_string;
	opf.lpfnHook = NULL;
	opf.lCustData = 0;
	opf.Flags = OFN_PATHMUSTEXIST;
	opf.lStructSize = sizeof(OPENFILENAMEW);
	if (GetSaveFileNameW(&opf) != 0)
	{
		// Copy the selected file name, converting from UTF-16 to UTF-8
		gchar *utf8string = g_utf16_to_utf8((const gunichar2*)opf.lpstrFile, _MAX_PATH, NULL, NULL, NULL);
		gtk_entry_set_text (GTK_ENTRY (fe), utf8string);
        g_object_set_data (G_OBJECT (dlg), "filename", fe);
		g_free(utf8string);

	}
	g_free(extension_string);
	g_free(title_string);
	g_free(filter_string);
#else
    if (gtk_dialog_run (GTK_DIALOG (fs)) == GTK_RESPONSE_ACCEPT)
    {
        gchar *file;

        file = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fs));

        gchar * utf8file = g_filename_to_utf8( file, -1, NULL, NULL, NULL );
        gtk_entry_set_text (GTK_ENTRY (fe), utf8file);

        g_object_set_data (G_OBJECT (dlg), "filename", fe);

        g_free(utf8file);
        g_free(file);
    }
#endif

    gtk_widget_destroy (fs);

    return;
} // end of sp_export_browse_clicked()

// TODO: Move this to nr-rect-fns.h.
static bool
sp_export_bbox_equal(Geom::Rect const &one, Geom::Rect const &two)
{
    double const epsilon = pow(10.0, -EXPORT_COORD_PRECISION);
    return (
        (fabs(one.min()[Geom::X] - two.min()[Geom::X]) < epsilon) &&
        (fabs(one.min()[Geom::Y] - two.min()[Geom::Y]) < epsilon) &&
        (fabs(one.max()[Geom::X] - two.max()[Geom::X]) < epsilon) &&
        (fabs(one.max()[Geom::Y] - two.max()[Geom::Y]) < epsilon)
        );
}

/**
    \brief  This function is used to detect the current selection setting
            based on the values in the x0, y0, x1 and y0 fields.
    \param  base  The export dialog itself

    One of the most confusing parts of this function is why the array
    is built at the beginning.  What needs to happen here is that we
    should always check the current selection to see if it is the valid
    one.  While this is a performance improvement it is also a usability
    one during the cases where things like selections and drawings match
    size.  This way buttons change less 'randomly' (atleast in the eyes
    of the user).  To do this an array is built where the current selection
    type is placed first, and then the others in an order from smallest
    to largest (this can be configured by reshuffling \c test_order).

    All of the values in this function are rounded to two decimal places
    because that is what is shown to the user.  While everything is kept
    more accurate than that, the user can't control more acurrate than
    that, so for this to work for them - it needs to check on that level
    of accuracy.

    \todo finish writing this up
*/
static void
sp_export_detect_size(GtkObject * base) {
    static const selection_type test_order[SELECTION_NUMBER_OF] = {SELECTION_SELECTION, SELECTION_DRAWING, SELECTION_PAGE, SELECTION_CUSTOM};
    selection_type this_test[SELECTION_NUMBER_OF + 1];
    selection_type key = SELECTION_NUMBER_OF;

    Geom::Point x(sp_export_value_get_px (base, "x0"),
                  sp_export_value_get_px (base, "y0"));
    Geom::Point y(sp_export_value_get_px (base, "x1"),
                  sp_export_value_get_px (base, "y1"));
    Geom::Rect current_bbox(x, y);
    //std::cout << "Current " << current_bbox;

    this_test[0] = (selection_type)(GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(base), "selection-type")));
    for (int i = 0; i < SELECTION_NUMBER_OF; i++) {
        this_test[i + 1] = test_order[i];
    }

    for (int i = 0;
            i < SELECTION_NUMBER_OF + 1 &&
                key == SELECTION_NUMBER_OF &&
                SP_ACTIVE_DESKTOP != NULL;
            i++) {
        // std::cout << "Looking at: " << selection_names[this_test[i]] << std::endl;
        switch (this_test[i]) {
            case SELECTION_SELECTION:
                if ((sp_desktop_selection(SP_ACTIVE_DESKTOP))->isEmpty() == false) {
                    Geom::OptRect bbox = (sp_desktop_selection (SP_ACTIVE_DESKTOP))->bounds();

                    //std::cout << "Selection " << bbox;
                    if ( bbox && sp_export_bbox_equal(*bbox,current_bbox)) {
                        key = SELECTION_SELECTION;
                    }
                }
                break;
            case SELECTION_DRAWING: {
                SPDocument *doc = sp_desktop_document (SP_ACTIVE_DESKTOP);

                Geom::OptRect bbox = sp_item_bbox_desktop (SP_ITEM (SP_DOCUMENT_ROOT (doc)));

                // std::cout << "Drawing " << bbox2;
                if ( bbox && sp_export_bbox_equal(*bbox,current_bbox) ) {
                    key = SELECTION_DRAWING;
                }
                break;
            }

            case SELECTION_PAGE: {
                SPDocument *doc;

                doc = sp_desktop_document (SP_ACTIVE_DESKTOP);

                Geom::Point x(0.0, 0.0);
                Geom::Point y(sp_document_width(doc),
                              sp_document_height(doc));
                Geom::Rect bbox(x, y);

                // std::cout << "Page " << bbox;
                if (sp_export_bbox_equal(bbox,current_bbox)) {
                    key = SELECTION_PAGE;
                }

                break;
           }
        default:
           break;
        }
    }
    // std::cout << std::endl;

    if (key == SELECTION_NUMBER_OF) {
        key = SELECTION_CUSTOM;
    }

    /* We're now using a custom size, not a fixed one */
    /* printf("Detecting state: %s\n", selection_names[key]); */
    selection_type old = (selection_type)(GPOINTER_TO_INT(gtk_object_get_data(GTK_OBJECT(base), "selection-type")));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(gtk_object_get_data(base, selection_names[old])), FALSE);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(gtk_object_get_data(base, selection_names[key])), TRUE);
    gtk_object_set_data(GTK_OBJECT(base), "selection-type", (gpointer)key);

    return;
} /* sp_export_detect_size */

/// Called when area x0 value is changed
static void
sp_export_area_x_value_changed (GtkAdjustment *adj, GtkObject *base)
{
    float x0, x1, xdpi, width;

    if (gtk_object_get_data (base, "update"))
        return;

    if (sp_unit_selector_update_test ((SPUnitSelector *)gtk_object_get_data
            (base, "units")))
    {
        return;
    }

    gtk_object_set_data ( base, "update", GUINT_TO_POINTER (TRUE) );

    x0 = sp_export_value_get_px (base, "x0");
    x1 = sp_export_value_get_px (base, "x1");
    xdpi = sp_export_value_get (base, "xdpi");

    width = floor ((x1 - x0) * xdpi / DPI_BASE + 0.5);

    if (width < SP_EXPORT_MIN_SIZE) {
        const gchar *key;
        width = SP_EXPORT_MIN_SIZE;
        key = (const gchar *)gtk_object_get_data (GTK_OBJECT (adj), "key");

        if (!strcmp (key, "x0")) {
            x1 = x0 + width * DPI_BASE / xdpi;
            sp_export_value_set_px (base, "x1", x1);
        } else {
            x0 = x1 - width * DPI_BASE / xdpi;
            sp_export_value_set_px (base, "x0", x0);
        }
    }

    sp_export_value_set_px (base, "width", x1 - x0);
    sp_export_value_set (base, "bmwidth", width);

    sp_export_detect_size(base);

    gtk_object_set_data (base, "update", GUINT_TO_POINTER (FALSE));

    return;
} // end of sp_export_area_x_value_changed()

/// Called when area y0 value is changed.
static void
sp_export_area_y_value_changed (GtkAdjustment *adj, GtkObject *base)
{
    float y0, y1, ydpi, height;

    if (gtk_object_get_data (base, "update"))
        return;

    if (sp_unit_selector_update_test ((SPUnitSelector *)gtk_object_get_data
           (base, "units")))
    {
        return;
    }

    gtk_object_set_data (base, "update", GUINT_TO_POINTER (TRUE));

    y0 = sp_export_value_get_px (base, "y0");
    y1 = sp_export_value_get_px (base, "y1");
    ydpi = sp_export_value_get (base, "ydpi");

    height = floor ((y1 - y0) * ydpi / DPI_BASE + 0.5);

    if (height < SP_EXPORT_MIN_SIZE) {
        const gchar *key;
        height = SP_EXPORT_MIN_SIZE;
        key = (const gchar *)gtk_object_get_data (GTK_OBJECT (adj), "key");
        if (!strcmp (key, "y0")) {
            y1 = y0 + height * DPI_BASE / ydpi;
            sp_export_value_set_px (base, "y1", y1);
        } else {
            y0 = y1 - height * DPI_BASE / ydpi;
            sp_export_value_set_px (base, "y0", y0);
        }
    }

    sp_export_value_set_px (base, "height", y1 - y0);
    sp_export_value_set (base, "bmheight", height);

    sp_export_detect_size(base);

    gtk_object_set_data (base, "update", GUINT_TO_POINTER (FALSE));

    return;
} // end of sp_export_area_y_value_changed()

/// Called when x1-x0 or area width is changed
static void
sp_export_area_width_value_changed (GtkAdjustment */*adj*/, GtkObject *base)
{
    float x0, x1, xdpi, width, bmwidth;

    if (gtk_object_get_data (base, "update"))
        return;

    if (sp_unit_selector_update_test ((SPUnitSelector *)gtk_object_get_data
           (base, "units"))) {
        return;
    }

    gtk_object_set_data (base, "update", GUINT_TO_POINTER (TRUE));

    x0 = sp_export_value_get_px (base, "x0");
    x1 = sp_export_value_get_px (base, "x1");
    xdpi = sp_export_value_get (base, "xdpi");
    width = sp_export_value_get_px (base, "width");
    bmwidth = floor (width * xdpi / DPI_BASE + 0.5);

    if (bmwidth < SP_EXPORT_MIN_SIZE) {

        bmwidth = SP_EXPORT_MIN_SIZE;
        width = bmwidth * DPI_BASE / xdpi;
        sp_export_value_set_px (base, "width", width);
    }

    sp_export_value_set_px (base, "x1", x0 + width);
    sp_export_value_set (base, "bmwidth", bmwidth);

    gtk_object_set_data (base, "update", GUINT_TO_POINTER (FALSE));

    return;
} // end of sp_export_area_width_value_changed()

/// Called when y1-y0 or area height is changed.
static void
sp_export_area_height_value_changed (GtkAdjustment */*adj*/, GtkObject *base)
{

    float y0, y1, ydpi, height, bmheight;

    if (gtk_object_get_data (base, "update"))
        return;

    if (sp_unit_selector_update_test ((SPUnitSelector *)gtk_object_get_data
           (base, "units"))) {
        return;
    }

    gtk_object_set_data (base, "update", GUINT_TO_POINTER (TRUE));

    y0 = sp_export_value_get_px (base, "y0");
    y1 = sp_export_value_get_px (base, "y1");
    ydpi = sp_export_value_get (base, "ydpi");
    height = sp_export_value_get_px (base, "height");
    bmheight = floor (height * ydpi / DPI_BASE + 0.5);

    if (bmheight < SP_EXPORT_MIN_SIZE) {
        bmheight = SP_EXPORT_MIN_SIZE;
        height = bmheight * DPI_BASE / ydpi;
        sp_export_value_set_px (base, "height", height);
    }

    sp_export_value_set_px (base, "y1", y0 + height);
    sp_export_value_set (base, "bmheight", bmheight);

    gtk_object_set_data (base, "update", GUINT_TO_POINTER (FALSE));

    return;
} // end of sp_export_area_height_value_changed()

/**
    \brief  A function to set the ydpi
    \param  base  The export dialog

    This function grabs all of the y values and then figures out the
    new bitmap size based on the changing dpi value.  The dpi value is
    gotten from the xdpi setting as these can not currently be independent.
*/
static void
sp_export_set_image_y (GtkObject *base)
{
    float y0, y1, xdpi;

    y0 = sp_export_value_get_px (base, "y0");
    y1 = sp_export_value_get_px (base, "y1");
    xdpi = sp_export_value_get (base, "xdpi");

    sp_export_value_set (base, "ydpi", xdpi);
    sp_export_value_set (base, "bmheight", (y1 - y0) * xdpi / DPI_BASE);

    return;
} // end of sp_export_set_image_y()

/**
    \brief  A function to set the xdpi
    \param  base  The export dialog

    This function grabs all of the x values and then figures out the
    new bitmap size based on the changing dpi value.  The dpi value is
    gotten from the xdpi setting as these can not currently be independent.
*/
static void
sp_export_set_image_x (GtkObject *base)
{
    float x0, x1, xdpi;

    x0 = sp_export_value_get_px (base, "x0");
    x1 = sp_export_value_get_px (base, "x1");
    xdpi = sp_export_value_get (base, "xdpi");

    sp_export_value_set (base, "ydpi", xdpi);
    sp_export_value_set (base, "bmwidth", (x1 - x0) * xdpi / DPI_BASE);

    return;
} // end of sp_export_set_image_x()

/// Called when pixel width is changed
static void
sp_export_bitmap_width_value_changed (GtkAdjustment */*adj*/, GtkObject *base)
{
    float x0, x1, bmwidth, xdpi;

    if (gtk_object_get_data (base, "update"))
        return;

    if (sp_unit_selector_update_test ((SPUnitSelector *)gtk_object_get_data
           (base, "units"))) {
       return;
    }

    gtk_object_set_data (base, "update", GUINT_TO_POINTER (TRUE));

    x0 = sp_export_value_get_px (base, "x0");
    x1 = sp_export_value_get_px (base, "x1");
    bmwidth = sp_export_value_get (base, "bmwidth");

    if (bmwidth < SP_EXPORT_MIN_SIZE) {
        bmwidth = SP_EXPORT_MIN_SIZE;
        sp_export_value_set (base, "bmwidth", bmwidth);
    }

    xdpi = bmwidth * DPI_BASE / (x1 - x0);
    sp_export_value_set (base, "xdpi", xdpi);

    sp_export_set_image_y (base);

    gtk_object_set_data (base, "update", GUINT_TO_POINTER (FALSE));

    return;
} // end of sp_export_bitmap_width_value_changed()

/// Called when pixel height is changed
static void
sp_export_bitmap_height_value_changed (GtkAdjustment */*adj*/, GtkObject *base)
{
    float y0, y1, bmheight, xdpi;

    if (gtk_object_get_data (base, "update"))
        return;

    if (sp_unit_selector_update_test ((SPUnitSelector *)gtk_object_get_data
           (base, "units"))) {
       return;
    }

    gtk_object_set_data (base, "update", GUINT_TO_POINTER (TRUE));

    y0 = sp_export_value_get_px (base, "y0");
    y1 = sp_export_value_get_px (base, "y1");
    bmheight = sp_export_value_get (base, "bmheight");

    if (bmheight < SP_EXPORT_MIN_SIZE) {
        bmheight = SP_EXPORT_MIN_SIZE;
        sp_export_value_set (base, "bmheight", bmheight);
    }

    xdpi = bmheight * DPI_BASE / (y1 - y0);
    sp_export_value_set (base, "xdpi", xdpi);

    sp_export_set_image_x (base);

    gtk_object_set_data (base, "update", GUINT_TO_POINTER (FALSE));

    return;
} // end of sp_export_bitmap_width_value_changed()

/**
    \brief  A function to adjust the bitmap width when the xdpi value changes
    \param  adj  The adjustment that was changed
    \param  base The export dialog itself

    The first thing this function checks is to see if we are doing an
    update.  If we are, this function just returns because there is another
    instance of it that will handle everything for us.  If there is a
    units change, we also assume that everyone is being updated appropriately
    and there is nothing for us to do.

    If we're the highest level function, we set the update flag, and
    continue on our way.

    All of the values are grabbed using the \c sp_export_value_get functions
    (call to the _pt ones for x0 and x1 but just standard for xdpi).  The
    xdpi value is saved in the preferences for the next time the dialog
    is opened.  (does the selection dpi need to be set here?)

    A check is done to to ensure that we aren't outputing an invalid width,
    this is set by SP_EXPORT_MIN_SIZE.  If that is the case the dpi is
    changed to make it valid.

    After all of this the bitmap width is changed.

    We also change the ydpi.  This is a temporary hack as these can not
    currently be independent.  This is likely to change in the future.
*/
void
sp_export_xdpi_value_changed (GtkAdjustment */*adj*/, GtkObject *base)
{
    float x0, x1, xdpi, bmwidth;

    if (gtk_object_get_data (base, "update"))
        return;

    if (sp_unit_selector_update_test ((SPUnitSelector *)gtk_object_get_data
           (base, "units"))) {
       return;
    }

    gtk_object_set_data (base, "update", GUINT_TO_POINTER (TRUE));

    x0 = sp_export_value_get_px (base, "x0");
    x1 = sp_export_value_get_px (base, "x1");
    xdpi = sp_export_value_get (base, "xdpi");

    // remember xdpi setting
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setDouble("/dialogs/export/defaultxdpi/value", xdpi);

    bmwidth = (x1 - x0) * xdpi / DPI_BASE;

    if (bmwidth < SP_EXPORT_MIN_SIZE) {
        bmwidth = SP_EXPORT_MIN_SIZE;
        if (x1 != x0)
            xdpi = bmwidth * DPI_BASE / (x1 - x0);
        else
            xdpi = DPI_BASE;
        sp_export_value_set (base, "xdpi", xdpi);
    }

    sp_export_value_set (base, "bmwidth", bmwidth);

    sp_export_set_image_y (base);

    gtk_object_set_data (base, "update", GUINT_TO_POINTER (FALSE));

    return;
} // end of sp_export_xdpi_value_changed()


/**
    \brief  A function to change the area that is used for the exported
            bitmap.
    \param  base  This is the export dialog
    \param  x0    Horizontal upper left hand corner of the picture in points
    \param  y0    Vertical upper left hand corner of the picture in points
    \param  x1    Horizontal lower right hand corner of the picture in points
    \param  y1    Vertical lower right hand corner of the picture in points

    This function just calls \c sp_export_value_set_px for each of the
    parameters that is passed in.  This allows for setting them all in
    one convient area.

    Update is set to suspend all of the other test running while all the
    values are being set up.  This allows for a performance increase, but
    it also means that the wrong type won't be detected with only some of
    the values set.  After all the values are set everyone is told that
    there has been an update.
*/
static void
sp_export_set_area ( GtkObject *base, double x0, double y0, double x1, double y1 )
{
    gtk_object_set_data ( base, "update", GUINT_TO_POINTER (TRUE) );
    sp_export_value_set_px (base, "x1", x1);
    sp_export_value_set_px (base, "y1", y1);
    sp_export_value_set_px (base, "x0", x0);
    sp_export_value_set_px (base, "y0", y0);
    gtk_object_set_data ( base, "update", GUINT_TO_POINTER (FALSE) );

    sp_export_area_x_value_changed ((GtkAdjustment *)gtk_object_get_data (base, "x1"), base);
    sp_export_area_y_value_changed ((GtkAdjustment *)gtk_object_get_data (base, "y1"), base);

    return;
}

/**
    \brief  Sets the value of an adjustment
    \param  base  The export dialog
    \param  key   Which adjustment to set
    \param  val   What value to set it to

    This function finds the adjustment using the data stored in the
    export dialog.  After finding the adjustment it then sets
    the value of it.
*/
static void
sp_export_value_set ( GtkObject *base, const gchar *key, double val )
{
    GtkAdjustment *adj;

    adj = (GtkAdjustment *)gtk_object_get_data (base, key);

    gtk_adjustment_set_value (adj, val);
}

/**
    \brief  A function to set a value using the units points
    \param  base  The export dialog
    \param  key   Which value should be set
    \param  val   What the value should be in points

    This function first gets the adjustment for the key that is passed
    in.  It then figures out what units are currently being used in the
    dialog.  After doing all of that, it then converts the incoming
    value and sets the adjustment.
*/
static void
sp_export_value_set_px (GtkObject *base, const gchar *key, double val)
{
    const SPUnit *unit = sp_unit_selector_get_unit ((SPUnitSelector *)gtk_object_get_data (base, "units") );

    sp_export_value_set (base, key, sp_pixels_get_units (val, *unit));

    return;
}

/**
    \brief  Get the value of an adjustment in the export dialog
    \param  base  The export dialog
    \param  key   Which adjustment is being looked for
    \return The value in the specified adjustment

    This function gets the adjustment from the data field in the export
    dialog.  It then grabs the value from the adjustment.
*/
static float
sp_export_value_get ( GtkObject *base, const gchar *key )
{
    GtkAdjustment *adj;

    adj = (GtkAdjustment *)gtk_object_get_data (base, key);

    return adj->value;
}

/**
    \brief  Grabs a value in the export dialog and converts the unit
            to points
    \param  base  The export dialog
    \param  key   Which value should be returned
    \return The value in the adjustment in points

    This function, at its most basic, is a call to \c sp_export_value_get
    to get the value of the adjustment.  It then finds the units that
    are being used by looking at the "units" attribute of the export
    dialog.  Using that it converts the returned value into points.
*/
static float
sp_export_value_get_px ( GtkObject *base, const gchar *key )
{
    float value = sp_export_value_get(base, key);
    const SPUnit *unit = sp_unit_selector_get_unit ((SPUnitSelector *)gtk_object_get_data (base, "units"));

    return sp_units_get_pixels (value, *unit);
} // end of sp_export_value_get_px()

/**
    \brief  This function is called when the filename is changed by
            anyone.  It resets the virgin bit.
    \param  object  Text entry box
    \param  data    The export dialog
    \return None

    This function gets called when the text area is modified.  It is
    looking for the case where the text area is modified from its
    original value.  In that case it sets the "filename-modified" bit
    to TRUE.  If the text dialog returns back to the original text, the
    bit gets reset.  This should stop simple mistakes.
*/
static void
sp_export_filename_modified (GtkObject * object, gpointer data)
{
    GtkWidget * text_entry = (GtkWidget *)object;
    GtkWidget * export_dialog = (GtkWidget *)data;

    if (!strcmp(original_name, gtk_entry_get_text(GTK_ENTRY(text_entry)))) {
        gtk_object_set_data (GTK_OBJECT (export_dialog), "filename-modified", (gpointer)FALSE);
//        printf("Modified: FALSE\n");
    } else {
        gtk_object_set_data (GTK_OBJECT (export_dialog), "filename-modified", (gpointer)TRUE);
//        printf("Modified: TRUE\n");
    }

    return;
} // end sp_export_filename_modified

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
