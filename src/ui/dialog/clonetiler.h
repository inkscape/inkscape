/** @file
 * @brief Clone tiling dialog
 */
/* Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 2004 Authors
 * Released under the GNU GPL, read the file 'COPYING' for more information
 */
#ifndef __SP_CLONE_TILER_H__
#define __SP_CLONE_TILER_H__

#include "ui/widget/panel.h"

#include "ui/dialog/desktop-tracker.h"
#include "ui/widget/color-picker.h"
#include "sp-root.h"

namespace Gtk {
    class CheckButton;
    class ComboBox;
    class ToggleButton;
}

namespace Inkscape {
namespace UI {

namespace Widget {
    class UnitMenu;
}

namespace Dialog {

class CloneTiler : public Widget::Panel {
public:
    CloneTiler();
    virtual ~CloneTiler();

    static CloneTiler &getInstance() { return *new CloneTiler(); }
    void show_page_trace();
protected:
    enum PickType {
        PICK_COLOR,
        PICK_OPACITY,
        PICK_R,
        PICK_G,
        PICK_B,
        PICK_H,
        PICK_S,
        PICK_L
    };

    GtkWidget * new_tab(GtkWidget *nb, const gchar *label);
    GtkWidget * table_x_y_rand(int values);
    Gtk::Widget * spinbox(const char          *tip,
                          const Glib::ustring &attr,
                          double               lower,
                          double               upper,
                          const gchar         *suffix,
                          bool                 exponent = false);
    Gtk::Widget * checkbox(const char          *tip,
                           const Glib::ustring &attr);
    void table_attach(GtkWidget *table, GtkWidget *widget, float align, int row, int col);
    void table_attach(GtkWidget *table, Gtk::Widget *widget, float align, int row, int col);

    void       symgroup_changed(Gtk::ComboBox *cb);
    void       on_picker_color_changed(guint rgba);
    void       trace_hide_tiled_clones_recursively(SPObject *from);
    guint      number_of_clones(SPObject *obj);
    void       trace_setup(SPDocument *doc, gdouble zoom, SPItem *original);
    guint32    trace_pick(Geom::Rect box);
    void       trace_finish();
    bool       is_a_clone_of(SPObject *tile, SPObject *obj);
    Geom::Rect transform_rect(Geom::Rect const &r, Geom::Affine const &m);
    double     randomize01(double val, double rand);

    void apply();
    void change_selection(Inkscape::Selection *selection);
    void checkbox_toggled(Gtk::ToggleButton   *tb,
                          Glib::ustring const &attr);
    void do_pick_toggled();
    void external_change();
    void fill_width_changed();
    void fill_height_changed();
    void keep_bbox_toggled();
    void on_remove_button_clicked() {remove();}
    void pick_switched(PickType);
    void pick_to(Gtk::ToggleButton   *tb,
                 Glib::ustring const &pref);
    void remove(bool do_undo = true);
    void reset();
    void reset_recursive(GtkWidget *w);
    void switch_to_create();
    void switch_to_fill();
    void unclump();
    void unit_changed();
    void value_changed(Glib::RefPtr<Gtk::Adjustment> &adj, Glib::ustring const &pref);
    void xy_changed(Glib::RefPtr<Gtk::Adjustment> &adj, Glib::ustring const &pref);

    Geom::Affine get_transform(
            // symmetry group
            int type,

            // row, column
            int i, int j,

            // center, width, height of the tile
            double cx, double cy,
            double w,  double h,

            // values from the dialog:
            // Shift
            double shiftx_per_i,      double shifty_per_i,
            double shiftx_per_j,      double shifty_per_j,
            double shiftx_rand,       double shifty_rand,
            double shiftx_exp,        double shifty_exp,
            int    shiftx_alternate,  int    shifty_alternate,
            int    shiftx_cumulate,   int    shifty_cumulate,
            int    shiftx_excludew,   int    shifty_excludeh,

            // Scale
            double scalex_per_i,      double scaley_per_i,
            double scalex_per_j,      double scaley_per_j,
            double scalex_rand,       double scaley_rand,
            double scalex_exp,        double scaley_exp,
            double scalex_log,        double scaley_log,
            int    scalex_alternate,  int    scaley_alternate,
            int    scalex_cumulate,   int    scaley_cumulate,

            // Rotation
            double rotate_per_i,      double rotate_per_j,
            double rotate_rand,
            int    rotate_alternatei, int    rotate_alternatej,
            int    rotate_cumulatei,  int    rotate_cumulatej
            );


private:
    CloneTiler(CloneTiler const &d);
    CloneTiler& operator=(CloneTiler const &d);

    Gtk::CheckButton *_b;
    Gtk::CheckButton *_cb_keep_bbox;
    GtkWidget *nb;
    SPDesktop *desktop;
    DesktopTracker deskTrack;
    Inkscape::UI::Widget::ColorPicker *color_picker;
    GtkSizeGroup* table_row_labels;
    Inkscape::UI::Widget::UnitMenu *unit_menu;

    Glib::RefPtr<Gtk::Adjustment> fill_width;
    Glib::RefPtr<Gtk::Adjustment> fill_height;

    sigc::connection desktopChangeConn;
    sigc::connection selectChangedConn;
    sigc::connection externChangedConn;
    sigc::connection subselChangedConn;
    sigc::connection selectModifiedConn;
    sigc::connection color_changed_connection;
    sigc::connection unitChangedConn;

    /**
     * Can be invoked for setting the desktop. Currently not used.
     */
    void setDesktop(SPDesktop *desktop);

    /**
     * Is invoked by the desktop tracker when the desktop changes.
     */
    void setTargetDesktop(SPDesktop *desktop);

    // Variables that used to be set using GObject
    GtkWidget *_buttons_on_tiles;
    GtkWidget *_dotrace;
    GtkWidget *_status;
    Gtk::Box *_rowscols;
    Gtk::Box *_widthheight;
    
};


enum {
    TILE_P1,
    TILE_P2,
    TILE_PM,
    TILE_PG,
    TILE_CM,
    TILE_PMM,
    TILE_PMG,
    TILE_PGG,
    TILE_CMM,
    TILE_P4,
    TILE_P4M,
    TILE_P4G,
    TILE_P3,
    TILE_P31M,
    TILE_P3M1,
    TILE_P6,
    TILE_P6M
};

} // namespace Dialog
} // namespace UI
} // namespace Inkscape


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
