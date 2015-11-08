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

    GtkWidget * clonetiler_new_tab(GtkWidget *nb, const gchar *label);
    GtkWidget * clonetiler_table_x_y_rand(int values);
    GtkWidget * clonetiler_spinbox(const char *tip, const char *attr, double lower, double upper, const gchar *suffix, bool exponent = false);
    GtkWidget * clonetiler_checkbox(const char *tip, const char *attr);
    void clonetiler_table_attach(GtkWidget *table, GtkWidget *widget, float align, int row, int col);

    static void clonetiler_symgroup_changed(GtkComboBox *cb, gpointer /*data*/);
    static void clonetiler_remove(GtkWidget */*widget*/, GtkWidget *dlg, bool do_undo = true);
    static void on_picker_color_changed(guint rgba);
    static void clonetiler_trace_hide_tiled_clones_recursively(SPObject *from);
    static void clonetiler_checkbox_toggled(GtkToggleButton *tb, gpointer *data);
    static void clonetiler_pick_switched(GtkToggleButton */*tb*/, gpointer data);
    static void clonetiler_do_pick_toggled(GtkToggleButton *tb, GtkWidget *dlg);
    static void clonetiler_pick_to(GtkToggleButton *tb, gpointer data);
    static void clonetiler_xy_changed(GtkAdjustment *adj, gpointer data);
    static void clonetiler_fill_width_changed(GtkAdjustment *adj, Inkscape::UI::Widget::UnitMenu *u);
    static void clonetiler_fill_height_changed(GtkAdjustment *adj, Inkscape::UI::Widget::UnitMenu *u);
    void clonetiler_unit_changed();
    static void clonetiler_switch_to_create(GtkToggleButton */*tb*/, GtkWidget *dlg);
    static void clonetiler_switch_to_fill(GtkToggleButton */*tb*/, GtkWidget *dlg);
    static void clonetiler_keep_bbox_toggled(GtkToggleButton *tb, gpointer /*data*/);
    static void clonetiler_apply(GtkWidget */*widget*/, GtkWidget *dlg);
    static void clonetiler_unclump(GtkWidget */*widget*/, void *);
    static void clonetiler_change_selection(Inkscape::Selection *selection, GtkWidget *dlg);
    static void clonetiler_external_change(GtkWidget *dlg);
    static void clonetiler_disconnect_gsignal(GObject *widget, gpointer source);
    static void clonetiler_reset(GtkWidget */*widget*/, GtkWidget *dlg);
    static guint clonetiler_number_of_clones(SPObject *obj);
    static void clonetiler_trace_setup(SPDocument *doc, gdouble zoom, SPItem *original);
    static guint32 clonetiler_trace_pick(Geom::Rect box);
    static void clonetiler_trace_finish();
    static bool clonetiler_is_a_clone_of(SPObject *tile, SPObject *obj);
    static Geom::Rect transform_rect(Geom::Rect const &r, Geom::Affine const &m);
    static double randomize01(double val, double rand);
    static void clonetiler_value_changed(GtkAdjustment *adj, gpointer data);
    static void clonetiler_reset_recursive(GtkWidget *w);

    static Geom::Affine clonetiler_get_transform(    // symmetry group
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

    GtkWidget *dlg;
    GtkWidget *nb;
    GtkWidget *b;
    SPDesktop *desktop;
    DesktopTracker deskTrack;
    Inkscape::UI::Widget::ColorPicker *color_picker;
    GtkSizeGroup* table_row_labels;
    Inkscape::UI::Widget::UnitMenu *unit_menu;

#if WITH_GTKMM_3_0
    Glib::RefPtr<Gtk::Adjustment> fill_width;
    Glib::RefPtr<Gtk::Adjustment> fill_height;
#else
    Gtk::Adjustment *fill_width;
    Gtk::Adjustment *fill_height;
#endif

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

};


enum {
    PICK_COLOR,
    PICK_OPACITY,
    PICK_R,
    PICK_G,
    PICK_B,
    PICK_H,
    PICK_S,
    PICK_L
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
