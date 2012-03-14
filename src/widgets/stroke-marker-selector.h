#ifndef SEEN_SP_MARKER_SELECTOR_NEW_H
#define SEEN_SP_MARKER_SELECTOR_NEW_H

/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Maximilian Albert <maximilian.albert> (gtkmm-ification)
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtkmm/box.h>
#include <gtkmm/combobox.h>
#include <gtkmm/liststore.h>

#include <sigc++/signal.h>

#include "document.h"
#include "inkscape.h"
#include "display/drawing.h"

namespace Gtk {
class Container;
class Adjustment;
}

/**
 * ComboBox derived class for selecting stroke markers.
 */

class MarkerComboBox : public Gtk::ComboBox {
public:
    MarkerComboBox(gchar const *id);
    ~MarkerComboBox();

    sigc::signal<void> changed_signal;

    void set_current(SPObject *marker);
    void set_active_history();
    void set_selected(const gchar *name);
    const gchar *get_active_marker_uri();
    bool update() { return updating; };
    gchar const *get_id() { return combo_id; };

private:

    Glib::RefPtr<Gtk::ListStore> marker_store;
    gchar const *combo_id;
    bool updating;
    bool is_history;
    SPDesktop *desktop;
    SPDocument *doc;
    SPDocument *sandbox;
    Gtk::Image  *empty_image;
    Gtk::CellRendererText label_renderer;
    Gtk::CellRendererPixbuf image_renderer;

    class MarkerColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        Gtk::TreeModelColumn<Glib::ustring> label;
        Gtk::TreeModelColumn<const gchar *> marker;   // ustring doesnt work here on windows due to unicode
        Gtk::TreeModelColumn<bool> isstock;
        Gtk::TreeModelColumn<Gtk::Image *> image;
        Gtk::TreeModelColumn<bool> history;
        Gtk::TreeModelColumn<bool> isseparator;

        MarkerColumns() {
            add(label); add(marker); add(isstock);  add(image); add(history); add(isseparator);
        }
    };
    MarkerColumns marker_columns;

    void init_combo();
    void set_history(Gtk::TreeModel::Row match_row);
    void sp_marker_list_from_doc(SPDocument *source);
    GSList *get_marker_list (SPDocument *source);
    void add_markers (GSList *marker_list, SPDocument *source);
    SPDocument *ink_markers_preview_doc ();
    Gtk::Image * create_marker_image(unsigned psize, gchar const *mname,
                       SPDocument *source, Inkscape::Drawing &drawing, unsigned /*visionkey*/);

    /*
     * Callbacks for drawing the combo box
     */
    void prepareLabelRenderer( Gtk::TreeModel::const_iterator const &row );
    void prepareImageRenderer( Gtk::TreeModel::const_iterator const &row );
    static gboolean separator_cb (GtkTreeModel *model, GtkTreeIter *iter, gpointer data);


};

#endif // SEEN_SP_MARKER_SELECTOR_NEW_H

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
