#define __SP_CLONE_TILER_C__

/*
 * Clone tiling dialog
 *
 * Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *
 * Copyright (C) 2004-2006 Authors
 * Released under GNU GPL
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <glib/gmem.h>
#include <gtk/gtk.h>
#include <glibmm/i18n.h>

#include "application/application.h"
#include "application/editor.h"
#include "helper/window.h"
#include "helper/unit-menu.h"
#include "helper/units.h"
#include "widgets/icon.h"
#include "../inkscape.h"
#include "../prefs-utils.h"
#include "dialog-events.h"
#include "../macros.h"
#include "../verbs.h"
#include "../interface.h"
#include "../selection.h"
#include "../style.h"
#include "../desktop.h"
#include "../desktop-handles.h"
#include "../sp-namedview.h"
#include "../document.h"
#include "../message-stack.h"
#include "../sp-use.h"
#include "unclump.h"

#include "xml/repr.h"

#include "svg/svg.h"
#include "svg/svg-color.h"

#include "libnr/nr-matrix-fns.h"
#include "libnr/nr-matrix-ops.h"

#include "libnr/nr-matrix-translate-ops.h"
#include "libnr/nr-translate-ops.h"
#include "libnr/nr-translate-rotate-ops.h"
#include "libnr/nr-translate-scale-ops.h"

#include "display/nr-arena.h"
#include "display/nr-arena-item.h"

#include "ui/widget/color-picker.h"

#include "../sp-filter.h"
#include "../filter-chemistry.h"

#define MIN_ONSCREEN_DISTANCE 50

static GtkWidget *dlg = NULL;
static win_data wd;

// impossible original values to make sure they are read from prefs
static gint x = -1000, y = -1000, w = 0, h = 0;
static gchar const *prefs_path = "dialogs.clonetiler";

#define SB_MARGIN 1
#define VB_MARGIN 4

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

static GtkSizeGroup* table_row_labels = NULL;

static sigc::connection _shutdown_connection;
static sigc::connection _dialogs_hidden_connection;
static sigc::connection _dialogs_unhidden_connection;
static sigc::connection _desktop_activated_connection;
static sigc::connection _color_changed_connection;

static Inkscape::UI::Widget::ColorPicker *color_picker;

static void
clonetiler_dialog_destroy( GtkObject */*object*/, gpointer /*data*/ )
{
    if (Inkscape::NSApplication::Application::getNewGui())
    {
        _shutdown_connection.disconnect();
        _dialogs_hidden_connection.disconnect();
        _dialogs_unhidden_connection.disconnect();
        _desktop_activated_connection.disconnect();
    } else {
        sp_signal_disconnect_by_data (INKSCAPE, dlg);
    }
    _color_changed_connection.disconnect();

    delete color_picker;

    wd.win = dlg = NULL;
    wd.stop = 0;

}

static gboolean
clonetiler_dialog_delete (GtkObject */*object*/, GdkEvent * /*event*/, gpointer /*data*/)
{
    gtk_window_get_position ((GtkWindow *) dlg, &x, &y);
    gtk_window_get_size ((GtkWindow *) dlg, &w, &h);

    if (x<0) x=0;
    if (y<0) y=0;

    prefs_set_int_attribute (prefs_path, "x", x);
    prefs_set_int_attribute (prefs_path, "y", y);
    prefs_set_int_attribute (prefs_path, "w", w);
    prefs_set_int_attribute (prefs_path, "h", h);

    return FALSE; // which means, go ahead and destroy it

}

static void on_delete()
{
    (void)clonetiler_dialog_delete (0, 0, NULL);
}

static void
on_picker_color_changed (guint rgba)
{
    static bool is_updating = false;
    if (is_updating || !SP_ACTIVE_DESKTOP)
        return;

    is_updating = true;

    Inkscape::XML::Node *repr = inkscape_get_repr(INKSCAPE, prefs_path);
    gchar c[32];
    sp_svg_write_color(c, sizeof(c), rgba);
    repr->setAttribute("initial_color", c);

    is_updating = false;
}

static guint clonetiler_number_of_clones (SPObject *obj);

static void
clonetiler_change_selection (Inkscape::Application * /*inkscape*/, Inkscape::Selection *selection, GtkWidget *dlg)
{
    GtkWidget *buttons = (GtkWidget *) g_object_get_data (G_OBJECT(dlg), "buttons_on_tiles");
    GtkWidget *status = (GtkWidget *) g_object_get_data (G_OBJECT(dlg), "status");

    if (selection->isEmpty()) {
        gtk_widget_set_sensitive (buttons, FALSE);
        gtk_label_set_markup (GTK_LABEL(status), _("<small>Nothing selected.</small>"));
        return;
    }

    if (g_slist_length ((GSList *) selection->itemList()) > 1) {
        gtk_widget_set_sensitive (buttons, FALSE);
        gtk_label_set_markup (GTK_LABEL(status), _("<small>More than one object selected.</small>"));
        return;
    }

    guint n = clonetiler_number_of_clones(selection->singleItem());
    if (n > 0) {
        gtk_widget_set_sensitive (buttons, TRUE);
        gchar *sta = g_strdup_printf (_("<small>Object has <b>%d</b> tiled clones.</small>"), n);
        gtk_label_set_markup (GTK_LABEL(status), sta);
        g_free (sta);
    } else {
        gtk_widget_set_sensitive (buttons, FALSE);
        gtk_label_set_markup (GTK_LABEL(status), _("<small>Object has no tiled clones.</small>"));
    }
}

static void
clonetiler_external_change (Inkscape::Application * /*inkscape*/, GtkWidget *dlg)
{
    clonetiler_change_selection (NULL, sp_desktop_selection(SP_ACTIVE_DESKTOP), dlg);
}

static void clonetiler_disconnect_gsignal (GObject *widget, gpointer source) {
    if (source && G_IS_OBJECT(source))
        sp_signal_disconnect_by_data (source, widget);
}


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


static NR::Matrix
clonetiler_get_transform (

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
    )
{

    // Shift (in units of tile width or height) -------------
    double delta_shifti = 0.0;
    double delta_shiftj = 0.0;

    if( shiftx_alternate ) {
        delta_shifti = (double)(i%2);
    } else {
        if( shiftx_cumulate ) {  // Should the delta shifts be cumulative (i.e. 1, 1+2, 1+2+3, ...)
            delta_shifti = (double)(i*i);
        } else {
            delta_shifti = (double)i;
        }
    }

    if( shifty_alternate ) {
        delta_shiftj = (double)(j%2);
    } else {
        if( shifty_cumulate ) {
            delta_shiftj = (double)(j*j);
        } else {
            delta_shiftj = (double)j;
        }
    }

    // Random shift, only calculate if non-zero.
    double delta_shiftx_rand = 0.0;
    double delta_shifty_rand = 0.0;
    if( shiftx_rand != 0.0 ) delta_shiftx_rand = shiftx_rand * g_random_double_range (-1, 1);
    if( shifty_rand != 0.0 ) delta_shifty_rand = shifty_rand * g_random_double_range (-1, 1);


    // Delta shift (units of tile width/height)
    double di = shiftx_per_i * delta_shifti  + shiftx_per_j * delta_shiftj + delta_shiftx_rand;
    double dj = shifty_per_i * delta_shifti  + shifty_per_j * delta_shiftj + delta_shifty_rand;

    // Shift in actual x and y, used below
    double dx = w * di;
    double dy = h * dj;

    double shifti = di;
    double shiftj = dj;

    // Include tile width and height in shift if required
    if( !shiftx_excludew ) shifti += i;
    if( !shifty_excludeh ) shiftj += j;

    // Add exponential shift if necessary
    if ( shiftx_exp != 1.0 ) shifti = pow( shifti, shiftx_exp );
    if ( shifty_exp != 1.0 ) shiftj = pow( shiftj, shifty_exp );

    // Final shift
    NR::Matrix rect_translate (NR::translate (w * shifti, h * shiftj));

    // Rotation (in degrees) ------------
    double delta_rotationi = 0.0;
    double delta_rotationj = 0.0;

    if( rotate_alternatei ) {
        delta_rotationi = (double)(i%2);
    } else {
        if( rotate_cumulatei ) {
            delta_rotationi = (double)(i*i + i)/2.0;
        } else {
            delta_rotationi = (double)i;
        }
    }

    if( rotate_alternatej ) {
        delta_rotationj = (double)(j%2);
    } else {
        if( rotate_cumulatej ) {
            delta_rotationj = (double)(j*j + j)/2.0;
        } else {
            delta_rotationj = (double)j;
        }
    }

    double delta_rotate_rand = 0.0;
    if( rotate_rand != 0.0 ) delta_rotate_rand = rotate_rand * 180.0 * g_random_double_range (-1, 1);

    double dr = rotate_per_i * delta_rotationi + rotate_per_j * delta_rotationj + delta_rotate_rand;

    // Scale (times the original) -----------
    double delta_scalei = 0.0;
    double delta_scalej = 0.0;

    if( scalex_alternate ) {
        delta_scalei = (double)(i%2);
    } else {
        if( scalex_cumulate ) {  // Should the delta scales be cumulative (i.e. 1, 1+2, 1+2+3, ...)
            delta_scalei = (double)(i*i + i)/2.0;
        } else {
            delta_scalei = (double)i;
        }
    }

    if( scaley_alternate ) {
        delta_scalej = (double)(j%2);
    } else {
        if( scaley_cumulate ) {
            delta_scalej = (double)(j*j + j)/2.0;
        } else {
            delta_scalej = (double)j;
        }
    }

    // Random scale, only calculate if non-zero.
    double delta_scalex_rand = 0.0;
    double delta_scaley_rand = 0.0;
    if( scalex_rand != 0.0 ) delta_scalex_rand = scalex_rand * g_random_double_range (-1, 1);
    if( scaley_rand != 0.0 ) delta_scaley_rand = scaley_rand * g_random_double_range (-1, 1);
    // But if random factors are same, scale x and y proportionally
    if( scalex_rand == scaley_rand ) delta_scalex_rand = delta_scaley_rand;

    // Total delta scale
    double scalex = 1.0 + scalex_per_i * delta_scalei  + scalex_per_j * delta_scalej + delta_scalex_rand;
    double scaley = 1.0 + scaley_per_i * delta_scalei  + scaley_per_j * delta_scalej + delta_scaley_rand;

    if( scalex < 0.0 ) scalex = 0.0;
    if( scaley < 0.0 ) scaley = 0.0;

    // Add exponential scale if necessary
    if ( scalex_exp != 1.0 ) scalex = pow( scalex, scalex_exp );
    if ( scaley_exp != 1.0 ) scaley = pow( scaley, scaley_exp );

    // Add logarithmic factor if necessary
    if ( scalex_log  > 0.0 ) scalex = pow( scalex_log, scalex - 1.0 );
    if ( scaley_log  > 0.0 ) scaley = pow( scaley_log, scaley - 1.0 );
    // Alternative using rotation angle
    //if ( scalex_log  != 1.0 ) scalex *= pow( scalex_log, M_PI*dr/180 );
    //if ( scaley_log  != 1.0 ) scaley *= pow( scaley_log, M_PI*dr/180 );


    // Calculate transformation matrices, translating back to "center of tile" (rotation center) before transforming
    NR::Matrix drot_c   = NR::translate(-cx, -cy) * NR::rotate (M_PI*dr/180)    * NR::translate(cx, cy);

    NR::Matrix dscale_c = NR::translate(-cx, -cy) * NR::scale (scalex, scaley)  * NR::translate(cx, cy);

    NR::Matrix d_s_r = dscale_c * drot_c;

    NR::Matrix rotate_180_c  = NR::translate(-cx, -cy) * NR::rotate (M_PI)      * NR::translate(cx, cy);

    NR::Matrix rotate_90_c   = NR::translate(-cx, -cy) * NR::rotate (-M_PI/2)   * NR::translate(cx, cy);
    NR::Matrix rotate_m90_c  = NR::translate(-cx, -cy) * NR::rotate ( M_PI/2)   * NR::translate(cx, cy);

    NR::Matrix rotate_120_c  = NR::translate(-cx, -cy) * NR::rotate (-2*M_PI/3) * NR::translate(cx, cy);
    NR::Matrix rotate_m120_c = NR::translate(-cx, -cy) * NR::rotate ( 2*M_PI/3) * NR::translate(cx, cy);

    NR::Matrix rotate_60_c   = NR::translate(-cx, -cy) * NR::rotate (-M_PI/3)   * NR::translate(cx, cy);
    NR::Matrix rotate_m60_c  = NR::translate(-cx, -cy) * NR::rotate ( M_PI/3)   * NR::translate(cx, cy);

    NR::Matrix flip_x        = NR::translate(-cx, -cy) * NR::scale (-1, 1)      * NR::translate(cx, cy);
    NR::Matrix flip_y        = NR::translate(-cx, -cy) * NR::scale (1, -1)      * NR::translate(cx, cy);


    // Create tile with required symmetry
    const double cos60 = cos(M_PI/3);
    const double sin60 = sin(M_PI/3);
    const double cos30 = cos(M_PI/6);
    const double sin30 = sin(M_PI/6);

    switch (type) {

    case TILE_P1:
        return d_s_r * rect_translate;
        break;

    case TILE_P2:
        if (i % 2 == 0) {
            return d_s_r * rect_translate;
        } else {
            return d_s_r * rotate_180_c * rect_translate;
        }
        break;

    case TILE_PM:
        if (i % 2 == 0) {
            return d_s_r * rect_translate;
        } else {
            return d_s_r * flip_x * rect_translate;
        }
        break;

    case TILE_PG:
        if (j % 2 == 0) {
            return d_s_r * rect_translate;
        } else {
            return d_s_r * flip_x * rect_translate;
        }
        break;

    case TILE_CM:
        if ((i + j) % 2 == 0) {
            return d_s_r * rect_translate;
        } else {
            return d_s_r * flip_x * rect_translate;
        }
        break;

    case TILE_PMM:
        if (j % 2 == 0) {
            if (i % 2 == 0) {
                return d_s_r * rect_translate;
            } else {
                return d_s_r * flip_x * rect_translate;
            }
        } else {
            if (i % 2 == 0) {
                return d_s_r * flip_y * rect_translate;
            } else {
                return d_s_r * flip_x * flip_y * rect_translate;
            }
        }
        break;

    case TILE_PMG:
        if (j % 2 == 0) {
            if (i % 2 == 0) {
                return d_s_r * rect_translate;
            } else {
                return d_s_r * rotate_180_c * rect_translate;
            }
        } else {
            if (i % 2 == 0) {
                return d_s_r * flip_y * rect_translate;
            } else {
                return d_s_r * rotate_180_c * flip_y * rect_translate;
            }
        }
        break;

    case TILE_PGG:
        if (j % 2 == 0) {
            if (i % 2 == 0) {
                return d_s_r * rect_translate;
            } else {
                return d_s_r * flip_y * rect_translate;
            }
        } else {
            if (i % 2 == 0) {
                return d_s_r * rotate_180_c * rect_translate;
            } else {
                return d_s_r * rotate_180_c * flip_y * rect_translate;
            }
        }
        break;

    case TILE_CMM:
        if (j % 4 == 0) {
            if (i % 2 == 0) {
                return d_s_r * rect_translate;
            } else {
                return d_s_r * flip_x * rect_translate;
            }
        } else if (j % 4 == 1) {
            if (i % 2 == 0) {
                return d_s_r * flip_y * rect_translate;
            } else {
                return d_s_r * flip_x * flip_y * rect_translate;
            }
        } else if (j % 4 == 2) {
            if (i % 2 == 1) {
                return d_s_r * rect_translate;
            } else {
                return d_s_r * flip_x * rect_translate;
            }
        } else {
            if (i % 2 == 1) {
                return d_s_r * flip_y * rect_translate;
            } else {
                return d_s_r * flip_x * flip_y * rect_translate;
            }
        }
        break;

    case TILE_P4:
    {
        NR::Matrix ori  (NR::translate ((w + h) * pow((i/2), shiftx_exp) + dx,  (h + w) * pow((j/2), shifty_exp) + dy));
        NR::Matrix dia1 (NR::translate (w/2 + h/2, -h/2 + w/2));
        NR::Matrix dia2 (NR::translate (-w/2 + h/2, h/2 + w/2));
        if (j % 2 == 0) {
            if (i % 2 == 0) {
                return d_s_r * ori;
            } else {
                return d_s_r * rotate_m90_c * dia1 * ori;
            }
        } else {
            if (i % 2 == 0) {
                return d_s_r * rotate_90_c * dia2 * ori;
            } else {
                return d_s_r * rotate_180_c * dia1 * dia2 * ori;
            }
        }
    }
    break;

    case TILE_P4M:
    {
        double max = MAX(w, h);
        NR::Matrix ori (NR::translate ((max + max) * pow((i/4), shiftx_exp) + dx,  (max + max) * pow((j/2), shifty_exp) + dy));
        NR::Matrix dia1 (NR::translate ( w/2 - h/2, h/2 - w/2));
        NR::Matrix dia2 (NR::translate (-h/2 + w/2, w/2 - h/2));
        if (j % 2 == 0) {
            if (i % 4 == 0) {
                return d_s_r * ori;
            } else if (i % 4 == 1) {
                return d_s_r * flip_y * rotate_m90_c * dia1 * ori;
            } else if (i % 4 == 2) {
                return d_s_r * rotate_m90_c * dia1 * NR::translate (h, 0) * ori;
            } else if (i % 4 == 3) {
                return d_s_r * flip_x * NR::translate (w, 0) * ori;
            }
        } else {
            if (i % 4 == 0) {
                return d_s_r * flip_y * NR::translate(0, h) * ori;
            } else if (i % 4 == 1) {
                return d_s_r * rotate_90_c * dia2 * NR::translate(0, h) * ori;
            } else if (i % 4 == 2) {
                return d_s_r * flip_y * rotate_90_c * dia2 * NR::translate(h, 0) * NR::translate(0, h) * ori;
            } else if (i % 4 == 3) {
                return d_s_r * flip_y * flip_x * NR::translate(w, 0) * NR::translate(0, h) * ori;
            }
        }
    }
    break;

    case TILE_P4G:
    {
        double max = MAX(w, h);
        NR::Matrix ori (NR::translate ((max + max) * pow((i/4), shiftx_exp) + dx,  (max + max) * pow(j, shifty_exp) + dy));
        NR::Matrix dia1 (NR::translate ( w/2 + h/2, h/2 - w/2));
        NR::Matrix dia2 (NR::translate (-h/2 + w/2, w/2 + h/2));
        if (((i/4) + j) % 2 == 0) {
            if (i % 4 == 0) {
                return d_s_r * ori;
            } else if (i % 4 == 1) {
                return d_s_r * rotate_m90_c * dia1 * ori;
            } else if (i % 4 == 2) {
                return d_s_r * rotate_90_c * dia2 * ori;
            } else if (i % 4 == 3) {
                return d_s_r * rotate_180_c * dia1 * dia2 * ori;
            }
        } else {
            if (i % 4 == 0) {
                return d_s_r * flip_y * NR::translate (0, h) * ori;
            } else if (i % 4 == 1) {
                return d_s_r * flip_y * rotate_m90_c * dia1 * NR::translate (-h, 0) * ori;
            } else if (i % 4 == 2) {
                return d_s_r * flip_y * rotate_90_c * dia2 * NR::translate (h, 0) * ori;
            } else if (i % 4 == 3) {
                return d_s_r * flip_x * NR::translate (w, 0) * ori;
            }
        }
    }
    break;

    case TILE_P3:
    {
        double width;
        double height;
        NR::Matrix dia1;
        NR::Matrix dia2;
        if (w > h) {
            width  = w + w * cos60;
            height = 2 * w * sin60;
            dia1 = NR::Matrix (NR::translate (w/2 + w/2 * cos60, -(w/2 * sin60)));
            dia2 = dia1 * NR::Matrix (NR::translate (0, 2 * (w/2 * sin60)));
        } else {
            width = h * cos (M_PI/6);
            height = h;
            dia1 = NR::Matrix (NR::translate (h/2 * cos30, -(h/2 * sin30)));
            dia2 = dia1 * NR::Matrix (NR::translate (0, h/2));
        }
        NR::Matrix ori (NR::translate (width * pow((2*(i/3) + j%2), shiftx_exp) + dx,  (height/2) * pow(j, shifty_exp) + dy));
        if (i % 3 == 0) {
            return d_s_r * ori;
        } else if (i % 3 == 1) {
            return d_s_r * rotate_m120_c * dia1 * ori;
        } else if (i % 3 == 2) {
            return d_s_r * rotate_120_c * dia2 * ori;
        }
    }
    break;

    case TILE_P31M:
    {
        NR::Matrix ori;
        NR::Matrix dia1;
        NR::Matrix dia2;
        NR::Matrix dia3;
        NR::Matrix dia4;
        if (w > h) {
            ori = NR::Matrix(NR::translate (w * pow((i/6) + 0.5*(j%2), shiftx_exp) + dx,  (w * cos30) * pow(j, shifty_exp) + dy));
            dia1 = NR::Matrix (NR::translate (0, h/2) * NR::translate (w/2, 0) * NR::translate (w/2 * cos60, -w/2 * sin60) * NR::translate (-h/2 * cos30, -h/2 * sin30) );
            dia2 = dia1 * NR::Matrix (NR::translate (h * cos30, h * sin30));
            dia3 = dia2 * NR::Matrix (NR::translate (0, 2 * (w/2 * sin60 - h/2 * sin30)));
            dia4 = dia3 * NR::Matrix (NR::translate (-h * cos30, h * sin30));
        } else {
            ori  = NR::Matrix (NR::translate (2*h * cos30  * pow((i/6 + 0.5*(j%2)), shiftx_exp) + dx,  (2*h - h * sin30) * pow(j, shifty_exp) + dy));
            dia1 = NR::Matrix (NR::translate (0, -h/2) * NR::translate (h/2 * cos30, h/2 * sin30));
            dia2 = dia1 * NR::Matrix (NR::translate (h * cos30, h * sin30));
            dia3 = dia2 * NR::Matrix (NR::translate (0, h/2));
            dia4 = dia3 * NR::Matrix (NR::translate (-h * cos30, h * sin30));
        }
        if (i % 6 == 0) {
            return d_s_r * ori;
        } else if (i % 6 == 1) {
            return d_s_r * flip_y * rotate_m120_c * dia1 * ori;
        } else if (i % 6 == 2) {
            return d_s_r * rotate_m120_c * dia2 * ori;
        } else if (i % 6 == 3) {
            return d_s_r * flip_y * rotate_120_c * dia3 * ori;
        } else if (i % 6 == 4) {
            return d_s_r * rotate_120_c * dia4 * ori;
        } else if (i % 6 == 5) {
            return d_s_r * flip_y * NR::translate(0, h) * ori;
        }
    }
    break;

    case TILE_P3M1:
    {
        double width;
        double height;
        NR::Matrix dia1;
        NR::Matrix dia2;
        NR::Matrix dia3;
        NR::Matrix dia4;
        if (w > h) {
            width = w + w * cos60;
            height = 2 * w * sin60;
            dia1 = NR::Matrix (NR::translate (0, h/2) * NR::translate (w/2, 0) * NR::translate (w/2 * cos60, -w/2 * sin60) * NR::translate (-h/2 * cos30, -h/2 * sin30) );
            dia2 = dia1 * NR::Matrix (NR::translate (h * cos30, h * sin30));
            dia3 = dia2 * NR::Matrix (NR::translate (0, 2 * (w/2 * sin60 - h/2 * sin30)));
            dia4 = dia3 * NR::Matrix (NR::translate (-h * cos30, h * sin30));
        } else {
            width = 2 * h * cos (M_PI/6);
            height = 2 * h;
            dia1 = NR::Matrix (NR::translate (0, -h/2) * NR::translate (h/2 * cos30, h/2 * sin30));
            dia2 = dia1 * NR::Matrix (NR::translate (h * cos30, h * sin30));
            dia3 = dia2 * NR::Matrix (NR::translate (0, h/2));
            dia4 = dia3 * NR::Matrix (NR::translate (-h * cos30, h * sin30));
        }
        NR::Matrix ori (NR::translate (width * pow((2*(i/6) + j%2), shiftx_exp) + dx,  (height/2) * pow(j, shifty_exp) + dy));
        if (i % 6 == 0) {
            return d_s_r * ori;
        } else if (i % 6 == 1) {
            return d_s_r * flip_y * rotate_m120_c * dia1 * ori;
        } else if (i % 6 == 2) {
            return d_s_r * rotate_m120_c * dia2 * ori;
        } else if (i % 6 == 3) {
            return d_s_r * flip_y * rotate_120_c * dia3 * ori;
        } else if (i % 6 == 4) {
            return d_s_r * rotate_120_c * dia4 * ori;
        } else if (i % 6 == 5) {
            return d_s_r * flip_y * NR::translate(0, h) * ori;
        }
    }
    break;

    case TILE_P6:
    {
        NR::Matrix ori;
        NR::Matrix dia1;
        NR::Matrix dia2;
        NR::Matrix dia3;
        NR::Matrix dia4;
        NR::Matrix dia5;
        if (w > h) {
            ori = NR::Matrix(NR::translate (w * pow((2*(i/6) + (j%2)), shiftx_exp) + dx,  (2*w * sin60) * pow(j, shifty_exp) + dy));
            dia1 = NR::Matrix (NR::translate (w/2 * cos60, -w/2 * sin60));
            dia2 = dia1 * NR::Matrix (NR::translate (w/2, 0));
            dia3 = dia2 * NR::Matrix (NR::translate (w/2 * cos60, w/2 * sin60));
            dia4 = dia3 * NR::Matrix (NR::translate (-w/2 * cos60, w/2 * sin60));
            dia5 = dia4 * NR::Matrix (NR::translate (-w/2, 0));
        } else {
            ori = NR::Matrix(NR::translate (2*h * cos30 * pow((i/6 + 0.5*(j%2)), shiftx_exp) + dx,  (h + h * sin30) * pow(j, shifty_exp) + dy));
            dia1 = NR::Matrix (NR::translate (-w/2, -h/2) * NR::translate (h/2 * cos30, -h/2 * sin30) * NR::translate (w/2 * cos60, w/2 * sin60));
            dia2 = dia1 * NR::Matrix (NR::translate (-w/2 * cos60, -w/2 * sin60) * NR::translate (h/2 * cos30, -h/2 * sin30) * NR::translate (h/2 * cos30, h/2 * sin30) * NR::translate (-w/2 * cos60, w/2 * sin60));
            dia3 = dia2 * NR::Matrix (NR::translate (w/2 * cos60, -w/2 * sin60) * NR::translate (h/2 * cos30, h/2 * sin30) * NR::translate (-w/2, h/2));
            dia4 = dia3 * dia1.inverse();
            dia5 = dia3 * dia2.inverse();
        }
        if (i % 6 == 0) {
            return d_s_r * ori;
        } else if (i % 6 == 1) {
            return d_s_r * rotate_m60_c * dia1 * ori;
        } else if (i % 6 == 2) {
            return d_s_r * rotate_m120_c * dia2 * ori;
        } else if (i % 6 == 3) {
            return d_s_r * rotate_180_c * dia3 * ori;
        } else if (i % 6 == 4) {
            return d_s_r * rotate_120_c * dia4 * ori;
        } else if (i % 6 == 5) {
            return d_s_r * rotate_60_c * dia5 * ori;
        }
    }
    break;

    case TILE_P6M:
    {

        NR::Matrix ori;
        NR::Matrix dia1, dia2, dia3, dia4, dia5, dia6, dia7, dia8, dia9, dia10;
        if (w > h) {
            ori = NR::Matrix(NR::translate (w * pow((2*(i/12) + (j%2)), shiftx_exp) + dx,  (2*w * sin60) * pow(j, shifty_exp) + dy));
            dia1 = NR::Matrix (NR::translate (w/2, h/2) * NR::translate (-w/2 * cos60, -w/2 * sin60) * NR::translate (-h/2 * cos30, h/2 * sin30));
            dia2 = dia1 * NR::Matrix (NR::translate (h * cos30, -h * sin30));
            dia3 = dia2 * NR::Matrix (NR::translate (-h/2 * cos30, h/2 * sin30) * NR::translate (w * cos60, 0) * NR::translate (-h/2 * cos30, -h/2 * sin30));
            dia4 = dia3 * NR::Matrix (NR::translate (h * cos30, h * sin30));
            dia5 = dia4 * NR::Matrix (NR::translate (-h/2 * cos30, -h/2 * sin30) * NR::translate (-w/2 * cos60, w/2 * sin60) * NR::translate (w/2, -h/2));
            dia6 = dia5 * NR::Matrix (NR::translate (0, h));
            dia7 = dia6 * dia1.inverse();
            dia8 = dia6 * dia2.inverse();
            dia9 = dia6 * dia3.inverse();
            dia10 = dia6 * dia4.inverse();
        } else {
            ori = NR::Matrix(NR::translate (4*h * cos30 * pow((i/12 + 0.5*(j%2)), shiftx_exp) + dx,  (2*h  + 2*h * sin30) * pow(j, shifty_exp) + dy));
            dia1 = NR::Matrix (NR::translate (-w/2, -h/2) * NR::translate (h/2 * cos30, -h/2 * sin30) * NR::translate (w/2 * cos60, w/2 * sin60));
            dia2 = dia1 * NR::Matrix (NR::translate (h * cos30, -h * sin30));
            dia3 = dia2 * NR::Matrix (NR::translate (-w/2 * cos60, -w/2 * sin60) * NR::translate (h * cos30, 0) * NR::translate (-w/2 * cos60, w/2 * sin60));
            dia4 = dia3 * NR::Matrix (NR::translate (h * cos30, h * sin30));
            dia5 = dia4 * NR::Matrix (NR::translate (w/2 * cos60, -w/2 * sin60) * NR::translate (h/2 * cos30, h/2 * sin30) * NR::translate (-w/2, h/2));
            dia6 = dia5 * NR::Matrix (NR::translate (0, h));
            dia7 = dia6 * dia1.inverse();
            dia8 = dia6 * dia2.inverse();
            dia9 = dia6 * dia3.inverse();
            dia10 = dia6 * dia4.inverse();
        }
        if (i % 12 == 0) {
            return d_s_r * ori;
        } else if (i % 12 == 1) {
            return d_s_r * flip_y * rotate_m60_c * dia1 * ori;
        } else if (i % 12 == 2) {
            return d_s_r * rotate_m60_c * dia2 * ori;
        } else if (i % 12 == 3) {
            return d_s_r * flip_y * rotate_m120_c * dia3 * ori;
        } else if (i % 12 == 4) {
            return d_s_r * rotate_m120_c * dia4 * ori;
        } else if (i % 12 == 5) {
            return d_s_r * flip_x * dia5 * ori;
        } else if (i % 12 == 6) {
            return d_s_r * flip_x * flip_y * dia6 * ori;
        } else if (i % 12 == 7) {
            return d_s_r * flip_y * rotate_120_c * dia7 * ori;
        } else if (i % 12 == 8) {
            return d_s_r * rotate_120_c * dia8 * ori;
        } else if (i % 12 == 9) {
            return d_s_r * flip_y * rotate_60_c * dia9 * ori;
        } else if (i % 12 == 10) {
            return d_s_r * rotate_60_c * dia10 * ori;
        } else if (i % 12 == 11) {
            return d_s_r * flip_y * NR::translate (0, h) * ori;
        }
    }
    break;

    default:
        break;
    }

    return NR::identity();
}

static bool
clonetiler_is_a_clone_of (SPObject *tile, SPObject *obj)
{
    char *id_href = NULL;

    if (obj) {
        Inkscape::XML::Node *obj_repr = SP_OBJECT_REPR(obj);
        id_href = g_strdup_printf("#%s", obj_repr->attribute("id"));
    }

    if (SP_IS_USE(tile) &&
        SP_OBJECT_REPR(tile)->attribute("xlink:href") &&
        (!id_href || !strcmp(id_href, SP_OBJECT_REPR(tile)->attribute("xlink:href"))) &&
        SP_OBJECT_REPR(tile)->attribute("inkscape:tiled-clone-of") &&
        (!id_href || !strcmp(id_href, SP_OBJECT_REPR(tile)->attribute("inkscape:tiled-clone-of"))))
    {
        if (id_href)
            g_free (id_href);
        return true;
    } else {
        if (id_href)
            g_free (id_href);
        return false;
    }
}

static NRArena const *trace_arena = NULL;
static unsigned trace_visionkey;
static NRArenaItem *trace_root;
static gdouble trace_zoom;

static void
clonetiler_trace_hide_tiled_clones_recursively (SPObject *from)
{
    if (!trace_arena)
        return;

    for (SPObject *o = sp_object_first_child(from); o != NULL; o = SP_OBJECT_NEXT(o)) {
        if (SP_IS_ITEM(o) && clonetiler_is_a_clone_of (o, NULL))
            sp_item_invoke_hide(SP_ITEM(o), trace_visionkey); // FIXME: hide each tiled clone's original too!
        clonetiler_trace_hide_tiled_clones_recursively (o);
    }
}

static void
clonetiler_trace_setup (SPDocument *doc, gdouble zoom, SPItem *original)
{
    trace_arena = NRArena::create();
    /* Create ArenaItem and set transform */
    trace_visionkey = sp_item_display_key_new(1);
    trace_root = sp_item_invoke_show( SP_ITEM(SP_DOCUMENT_ROOT (doc)),
                                      (NRArena *) trace_arena, trace_visionkey, SP_ITEM_SHOW_DISPLAY);

    // hide the (current) original and any tiled clones, we only want to pick the background
    sp_item_invoke_hide(original, trace_visionkey);
    clonetiler_trace_hide_tiled_clones_recursively (SP_OBJECT(SP_DOCUMENT_ROOT (doc)));

    sp_document_root (doc)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
    sp_document_ensure_up_to_date(doc);

    trace_zoom = zoom;
}

static guint32
clonetiler_trace_pick (NR::Rect box)
{
    if (!trace_arena)
        return 0;

    NR::Matrix t(NR::scale(trace_zoom, trace_zoom));
    nr_arena_item_set_transform(trace_root, &t);
    NRGC gc(NULL);
    gc.transform.set_identity();
    nr_arena_item_invoke_update( trace_root, NULL, &gc,
                                 NR_ARENA_ITEM_STATE_ALL,
                                 NR_ARENA_ITEM_STATE_NONE );

    /* Item integer bbox in points */
    NRRectL ibox;
    ibox.x0 = (int) floor(trace_zoom * box.min()[NR::X] + 0.5);
    ibox.y0 = (int) floor(trace_zoom * box.min()[NR::Y] + 0.5);
    ibox.x1 = (int) floor(trace_zoom * box.max()[NR::X] + 0.5);
    ibox.y1 = (int) floor(trace_zoom * box.max()[NR::Y] + 0.5);

    /* Find visible area */
    int width = ibox.x1 - ibox.x0;
    int height = ibox.y1 - ibox.y0;

    /* Set up pixblock */
    guchar *px = g_new(guchar, 4 * width * height);

    if (px == NULL) {
        return 0; // buffer is too big or too small, cannot pick, so return 0
    }

    memset(px, 0x00, 4 * width * height);

    /* Render */
    NRPixBlock pb;
    nr_pixblock_setup_extern( &pb, NR_PIXBLOCK_MODE_R8G8B8A8N,
                              ibox.x0, ibox.y0, ibox.x1, ibox.y1,
                              px, 4 * width, FALSE, FALSE );
    nr_arena_item_invoke_render(NULL, trace_root, &ibox, &pb,
                                 NR_ARENA_ITEM_RENDER_NO_CACHE );

    double R = 0, G = 0, B = 0, A = 0;
    double count = 0;
    double weight = 0;

    for (int y = ibox.y0; y < ibox.y1; y++) {
        const unsigned char *s = NR_PIXBLOCK_PX (&pb) + (y - ibox.y0) * pb.rs;
        for (int x = ibox.x0; x < ibox.x1; x++) {
            count += 1;
            weight += s[3] / 255.0;
            R += s[0] / 255.0;
            G += s[1] / 255.0;
            B += s[2] / 255.0;
            A += s[3] / 255.0;
            s += 4;
        }
    }

    nr_pixblock_release(&pb);

    R = R / weight;
    G = G / weight;
    B = B / weight;
    A = A / count;

    R = CLAMP (R, 0.0, 1.0);
    G = CLAMP (G, 0.0, 1.0);
    B = CLAMP (B, 0.0, 1.0);
    A = CLAMP (A, 0.0, 1.0);

    return SP_RGBA32_F_COMPOSE (R, G, B, A);
}

static void
clonetiler_trace_finish ()
{
    if (trace_arena) {
        ((NRObject *) trace_arena)->unreference();
        trace_arena = NULL;
    }
}

static void
clonetiler_unclump( GtkWidget */*widget*/, void * )
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop == NULL)
        return;

    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    // check if something is selected
    if (selection->isEmpty() || g_slist_length((GSList *) selection->itemList()) > 1) {
        sp_desktop_message_stack(desktop)->flash(Inkscape::WARNING_MESSAGE, _("Select <b>one object</b> whose tiled clones to unclump."));
        return;
    }

    SPObject *obj = SP_OBJECT(selection->singleItem());
    SPObject *parent = SP_OBJECT_PARENT (obj);

    GSList *to_unclump = NULL; // not including the original

    for (SPObject *child = sp_object_first_child(parent); child != NULL; child = SP_OBJECT_NEXT(child)) {
        if (clonetiler_is_a_clone_of (child, obj)) {
            to_unclump = g_slist_prepend (to_unclump, child);
        }
    }

    sp_document_ensure_up_to_date(sp_desktop_document(desktop));

    unclump (to_unclump);

    g_slist_free (to_unclump);

    sp_document_done (sp_desktop_document (desktop), SP_VERB_DIALOG_CLONETILER,
                      _("Unclump tiled clones"));
}

static guint
clonetiler_number_of_clones (SPObject *obj)
{
    SPObject *parent = SP_OBJECT_PARENT (obj);

    guint n = 0;

    for (SPObject *child = sp_object_first_child(parent); child != NULL; child = SP_OBJECT_NEXT(child)) {
        if (clonetiler_is_a_clone_of (child, obj)) {
            n ++;
        }
    }

    return n;
}

static void
clonetiler_remove( GtkWidget */*widget*/, void *, bool do_undo = true )
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop == NULL)
        return;

    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    // check if something is selected
    if (selection->isEmpty() || g_slist_length((GSList *) selection->itemList()) > 1) {
        sp_desktop_message_stack(desktop)->flash(Inkscape::WARNING_MESSAGE, _("Select <b>one object</b> whose tiled clones to remove."));
        return;
    }

    SPObject *obj = SP_OBJECT(selection->singleItem());
    SPObject *parent = SP_OBJECT_PARENT (obj);

// remove old tiling
    GSList *to_delete = NULL;
    for (SPObject *child = sp_object_first_child(parent); child != NULL; child = SP_OBJECT_NEXT(child)) {
        if (clonetiler_is_a_clone_of (child, obj)) {
            to_delete = g_slist_prepend (to_delete, child);
        }
    }
    for (GSList *i = to_delete; i; i = i->next) {
        SP_OBJECT(i->data)->deleteObject();
    }
    g_slist_free (to_delete);

    clonetiler_change_selection (NULL, selection, dlg);

    if (do_undo)
        sp_document_done (sp_desktop_document (desktop), SP_VERB_DIALOG_CLONETILER,
                          _("Delete tiled clones"));
}

static NR::Rect
transform_rect(NR::Rect const &r, NR::Matrix const &m)
{
    using NR::X;
    using NR::Y;
    NR::Point const p1 = r.corner(1) * m;
    NR::Point const p2 = r.corner(2) * m;
    NR::Point const p3 = r.corner(3) * m;
    NR::Point const p4 = r.corner(4) * m;
    return NR::Rect(
        NR::Point(
            std::min(std::min(p1[X], p2[X]), std::min(p3[X], p4[X])),
            std::min(std::min(p1[Y], p2[Y]), std::min(p3[Y], p4[Y]))),
        NR::Point(
            std::max(std::max(p1[X], p2[X]), std::max(p3[X], p4[X])),
            std::max(std::max(p1[Y], p2[Y]), std::max(p3[Y], p4[Y]))));
}

/**
Randomizes \a val by \a rand, with 0 < val < 1 and all values (including 0, 1) having the same
probability of being displaced.
 */
static double
randomize01 (double val, double rand)
{
    double base = MIN (val - rand, 1 - 2*rand);
    if (base < 0) base = 0;
    val = base + g_random_double_range (0, MIN (2 * rand, 1 - base));
    return CLAMP(val, 0, 1); // this should be unnecessary with the above provisions, but just in case...
}


static void
clonetiler_apply( GtkWidget */*widget*/, void * )
{
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop == NULL)
        return;

    Inkscape::Selection *selection = sp_desktop_selection(desktop);

    // check if something is selected
    if (selection->isEmpty()) {
        sp_desktop_message_stack(desktop)->flash(Inkscape::WARNING_MESSAGE, _("Select an <b>object</b> to clone."));
        return;
    }

    // Check if more than one object is selected.
    if (g_slist_length((GSList *) selection->itemList()) > 1) {
        sp_desktop_message_stack(desktop)->flash(Inkscape::ERROR_MESSAGE, _("If you want to clone several objects, <b>group</b> them and <b>clone the group</b>."));
        return;
    }

    // set "busy" cursor
    desktop->setWaitingCursor();

    // set statusbar text
    GtkWidget *status = (GtkWidget *) g_object_get_data (G_OBJECT(dlg), "status");
    gtk_label_set_markup (GTK_LABEL(status), _("<small>Creating tiled clones...</small>"));
    gtk_widget_queue_draw(GTK_WIDGET(status));
    gdk_window_process_all_updates();

    SPObject *obj = SP_OBJECT(selection->singleItem());
    Inkscape::XML::Node *obj_repr = SP_OBJECT_REPR(obj);
    const char *id_href = g_strdup_printf("#%s", obj_repr->attribute("id"));
    SPObject *parent = SP_OBJECT_PARENT (obj);

    clonetiler_remove (NULL, NULL, false);

    double shiftx_per_i = 0.01 * prefs_get_double_attribute_limited (prefs_path, "shiftx_per_i", 0, -10000, 10000);
    double shifty_per_i = 0.01 * prefs_get_double_attribute_limited (prefs_path, "shifty_per_i", 0, -10000, 10000);
    double shiftx_per_j = 0.01 * prefs_get_double_attribute_limited (prefs_path, "shiftx_per_j", 0, -10000, 10000);
    double shifty_per_j = 0.01 * prefs_get_double_attribute_limited (prefs_path, "shifty_per_j", 0, -10000, 10000);
    double shiftx_rand  = 0.01 * prefs_get_double_attribute_limited (prefs_path, "shiftx_rand", 0, 0, 1000);
    double shifty_rand  = 0.01 * prefs_get_double_attribute_limited (prefs_path, "shifty_rand", 0, 0, 1000);
    double shiftx_exp   =        prefs_get_double_attribute_limited (prefs_path, "shiftx_exp",   1, 0, 10);
    double shifty_exp   =        prefs_get_double_attribute_limited (prefs_path, "shifty_exp", 1, 0, 10);
    int    shiftx_alternate =    prefs_get_int_attribute (prefs_path, "shiftx_alternate", 0);
    int    shifty_alternate =    prefs_get_int_attribute (prefs_path, "shifty_alternate", 0);
    int    shiftx_cumulate  =    prefs_get_int_attribute (prefs_path, "shiftx_cumulate", 0);
    int    shifty_cumulate  =    prefs_get_int_attribute (prefs_path, "shifty_cumulate", 0);
    int    shiftx_excludew  =    prefs_get_int_attribute (prefs_path, "shiftx_excludew", 0);
    int    shifty_excludeh  =    prefs_get_int_attribute (prefs_path, "shifty_excludeh", 0);

    double scalex_per_i = 0.01 * prefs_get_double_attribute_limited (prefs_path, "scalex_per_i", 0, -100, 1000);
    double scaley_per_i = 0.01 * prefs_get_double_attribute_limited (prefs_path, "scaley_per_i", 0, -100, 1000);
    double scalex_per_j = 0.01 * prefs_get_double_attribute_limited (prefs_path, "scalex_per_j", 0, -100, 1000);
    double scaley_per_j = 0.01 * prefs_get_double_attribute_limited (prefs_path, "scaley_per_j", 0, -100, 1000);
    double scalex_rand  = 0.01 * prefs_get_double_attribute_limited (prefs_path, "scalex_rand",  0, 0, 1000);
    double scaley_rand  = 0.01 * prefs_get_double_attribute_limited (prefs_path, "scaley_rand",  0, 0, 1000);
    double scalex_exp   =        prefs_get_double_attribute_limited (prefs_path, "scalex_exp",   1, 0, 10);
    double scaley_exp   =        prefs_get_double_attribute_limited (prefs_path, "scaley_exp",   1, 0, 10);
    double scalex_log       =    prefs_get_double_attribute_limited (prefs_path, "scalex_log",   0, 0, 10);
    double scaley_log       =    prefs_get_double_attribute_limited (prefs_path, "scaley_log",   0, 0, 10);
    int    scalex_alternate =    prefs_get_int_attribute (prefs_path, "scalex_alternate", 0);
    int    scaley_alternate =    prefs_get_int_attribute (prefs_path, "scaley_alternate", 0);
    int    scalex_cumulate  =    prefs_get_int_attribute (prefs_path, "scalex_cumulate", 0);
    int    scaley_cumulate  =    prefs_get_int_attribute (prefs_path, "scaley_cumulate", 0);

    double rotate_per_i =        prefs_get_double_attribute_limited (prefs_path, "rotate_per_i", 0, -180, 180);
    double rotate_per_j =        prefs_get_double_attribute_limited (prefs_path, "rotate_per_j", 0, -180, 180);
    double rotate_rand =  0.01 * prefs_get_double_attribute_limited (prefs_path, "rotate_rand", 0, 0, 100);
    int    rotate_alternatei   = prefs_get_int_attribute (prefs_path, "rotate_alternatei", 0);
    int    rotate_alternatej   = prefs_get_int_attribute (prefs_path, "rotate_alternatej", 0);
    int    rotate_cumulatei    = prefs_get_int_attribute (prefs_path, "rotate_cumulatei", 0);
    int    rotate_cumulatej    = prefs_get_int_attribute (prefs_path, "rotate_cumulatej", 0);

    double blur_per_i =   0.01 * prefs_get_double_attribute_limited (prefs_path, "blur_per_i", 0, 0, 100);
    double blur_per_j =   0.01 * prefs_get_double_attribute_limited (prefs_path, "blur_per_j", 0, 0, 100);
    int    blur_alternatei =     prefs_get_int_attribute (prefs_path, "blur_alternatei", 0);
    int    blur_alternatej =     prefs_get_int_attribute (prefs_path, "blur_alternatej", 0);
    double blur_rand =    0.01 * prefs_get_double_attribute_limited (prefs_path, "blur_rand", 0, 0, 100);

    double opacity_per_i = 0.01 * prefs_get_double_attribute_limited (prefs_path, "opacity_per_i", 0, 0, 100);
    double opacity_per_j = 0.01 * prefs_get_double_attribute_limited (prefs_path, "opacity_per_j", 0, 0, 100);
    int    opacity_alternatei =   prefs_get_int_attribute (prefs_path, "opacity_alternatei", 0);
    int    opacity_alternatej =   prefs_get_int_attribute (prefs_path, "opacity_alternatej", 0);
    double opacity_rand =  0.01 * prefs_get_double_attribute_limited (prefs_path, "opacity_rand", 0, 0, 100);

    const gchar *initial_color =     prefs_get_string_attribute (prefs_path, "initial_color");
    double hue_per_j =        0.01 * prefs_get_double_attribute_limited (prefs_path, "hue_per_j", 0, -100, 100);
    double hue_per_i =        0.01 * prefs_get_double_attribute_limited (prefs_path, "hue_per_i", 0, -100, 100);
    double hue_rand  =        0.01 * prefs_get_double_attribute_limited (prefs_path, "hue_rand", 0, 0, 100);
    double saturation_per_j = 0.01 * prefs_get_double_attribute_limited (prefs_path, "saturation_per_j", 0, -100, 100);
    double saturation_per_i = 0.01 * prefs_get_double_attribute_limited (prefs_path, "saturation_per_i", 0, -100, 100);
    double saturation_rand =  0.01 * prefs_get_double_attribute_limited (prefs_path, "saturation_rand", 0, 0, 100);
    double lightness_per_j =  0.01 * prefs_get_double_attribute_limited (prefs_path, "lightness_per_j", 0, -100, 100);
    double lightness_per_i =  0.01 * prefs_get_double_attribute_limited (prefs_path, "lightness_per_i", 0, -100, 100);
    double lightness_rand =   0.01 * prefs_get_double_attribute_limited (prefs_path, "lightness_rand", 0, 0, 100);
    int    color_alternatej = prefs_get_int_attribute (prefs_path, "color_alternatej", 0);
    int    color_alternatei = prefs_get_int_attribute (prefs_path, "color_alternatei", 0);

    int type = prefs_get_int_attribute (prefs_path, "symmetrygroup", 0);

    int keepbbox = prefs_get_int_attribute (prefs_path, "keepbbox", 1);

    int imax = prefs_get_int_attribute (prefs_path, "imax", 2);
    int jmax = prefs_get_int_attribute (prefs_path, "jmax", 2);

    int fillrect = prefs_get_int_attribute (prefs_path, "fillrect", 0);
    double fillwidth = prefs_get_double_attribute_limited (prefs_path, "fillwidth", 50, 0, 1e6);
    double fillheight = prefs_get_double_attribute_limited (prefs_path, "fillheight", 50, 0, 1e6);

    int dotrace = prefs_get_int_attribute (prefs_path, "dotrace", 0);
    int pick = prefs_get_int_attribute (prefs_path, "pick", 0);
    int pick_to_presence = prefs_get_int_attribute (prefs_path, "pick_to_presence", 0);
    int pick_to_size = prefs_get_int_attribute (prefs_path, "pick_to_size", 0);
    int pick_to_color = prefs_get_int_attribute (prefs_path, "pick_to_color", 0);
    int pick_to_opacity = prefs_get_int_attribute (prefs_path, "pick_to_opacity", 0);
    double rand_picked = 0.01 * prefs_get_double_attribute_limited (prefs_path, "rand_picked", 0, 0, 100);
    int invert_picked = prefs_get_int_attribute (prefs_path, "invert_picked", 0);
    double gamma_picked = prefs_get_double_attribute_limited (prefs_path, "gamma_picked", 0, -10, 10);

    if (dotrace) {
        clonetiler_trace_setup (sp_desktop_document(desktop), 1.0, SP_ITEM (obj));
    }

    NR::Point center;
    double w;
    double h;
    double x0;
    double y0;

    if (keepbbox &&
        obj_repr->attribute("inkscape:tile-w") &&
        obj_repr->attribute("inkscape:tile-h") &&
        obj_repr->attribute("inkscape:tile-x0") &&
        obj_repr->attribute("inkscape:tile-y0") &&
        obj_repr->attribute("inkscape:tile-cx") &&
        obj_repr->attribute("inkscape:tile-cy")) {

        double cx = sp_repr_get_double_attribute (obj_repr, "inkscape:tile-cx", 0);
        double cy = sp_repr_get_double_attribute (obj_repr, "inkscape:tile-cy", 0);
        center = NR::Point (cx, cy);

        w = sp_repr_get_double_attribute (obj_repr, "inkscape:tile-w", 0);
        h = sp_repr_get_double_attribute (obj_repr, "inkscape:tile-h", 0);
        x0 = sp_repr_get_double_attribute (obj_repr, "inkscape:tile-x0", 0);
        y0 = sp_repr_get_double_attribute (obj_repr, "inkscape:tile-y0", 0);
    } else {
        int prefs_bbox = prefs_get_int_attribute("tools", "bounding_box", 0);
        SPItem::BBoxType bbox_type = (prefs_bbox ==0)? 
            SPItem::APPROXIMATE_BBOX : SPItem::GEOMETRIC_BBOX;
        boost::optional<NR::Rect> r = SP_ITEM(obj)->getBounds(from_2geom(sp_item_i2doc_affine(SP_ITEM(obj))),
                                                        bbox_type);
        if (r) {
            w = r->dimensions()[NR::X];
            h = r->dimensions()[NR::Y];
            x0 = r->min()[NR::X];
            y0 = r->min()[NR::Y];
            center = desktop->dt2doc(SP_ITEM(obj)->getCenter());

            sp_repr_set_svg_double(obj_repr, "inkscape:tile-cx", center[NR::X]);
            sp_repr_set_svg_double(obj_repr, "inkscape:tile-cy", center[NR::Y]);
            sp_repr_set_svg_double(obj_repr, "inkscape:tile-w", w);
            sp_repr_set_svg_double(obj_repr, "inkscape:tile-h", h);
            sp_repr_set_svg_double(obj_repr, "inkscape:tile-x0", x0);
            sp_repr_set_svg_double(obj_repr, "inkscape:tile-y0", y0);
        } else {
            center = NR::Point(0, 0);
            w = h = 0;
            x0 = y0 = 0;
        }
    }

    NR::Point cur = NR::Point (0, 0);
    NR::Rect bbox_original = NR::Rect (NR::Point (x0, y0), NR::Point (x0 + w, y0 + h));
    double perimeter_original = (w + h)/4;

    // The integers i and j are reserved for tile column and row.
    // The doubles x and y are used for coordinates
    for (int i = 0;
         fillrect?
             (fabs(cur[NR::X]) < fillwidth && i < 200) // prevent "freezing" with too large fillrect, arbitrarily limit rows
             : (i < imax);
         i ++) {
        for (int j = 0;
             fillrect?
                 (fabs(cur[NR::Y]) < fillheight && j < 200) // prevent "freezing" with too large fillrect, arbitrarily limit cols
                 : (j < jmax);
             j ++) {

            // Note: We create a clone at 0,0 too, right over the original, in case our clones are colored

            // Get transform from symmetry, shift, scale, rotation
            NR::Matrix t = clonetiler_get_transform (type, i, j, center[NR::X], center[NR::Y], w, h,
                                                     shiftx_per_i,     shifty_per_i,
                                                     shiftx_per_j,     shifty_per_j,
                                                     shiftx_rand,      shifty_rand,
                                                     shiftx_exp,       shifty_exp,
                                                     shiftx_alternate, shifty_alternate,
                                                     shiftx_cumulate,  shifty_cumulate,
                                                     shiftx_excludew,  shifty_excludeh,
                                                     scalex_per_i,     scaley_per_i,
                                                     scalex_per_j,     scaley_per_j,
                                                     scalex_rand,      scaley_rand,
                                                     scalex_exp,       scaley_exp,
                                                     scalex_log,       scaley_log,
                                                     scalex_alternate, scaley_alternate,
                                                     scalex_cumulate,  scaley_cumulate,
                                                     rotate_per_i,     rotate_per_j,
                                                     rotate_rand,
                                                     rotate_alternatei, rotate_alternatej,
                                                     rotate_cumulatei,  rotate_cumulatej      );

            cur = center * t - center;
            if (fillrect) {
                if ((cur[NR::X] > fillwidth) || (cur[NR::Y] > fillheight)) { // off limits
                    continue;
                }
            }

            gchar color_string[32]; *color_string = 0;

            // Color tab
            if (initial_color) {
                guint32 rgba = sp_svg_read_color (initial_color, 0x000000ff);
                float hsl[3];
                sp_color_rgb_to_hsl_floatv (hsl, SP_RGBA32_R_F(rgba), SP_RGBA32_G_F(rgba), SP_RGBA32_B_F(rgba));

                double eff_i = (color_alternatei? (i%2) : (i));
                double eff_j = (color_alternatej? (j%2) : (j));

                hsl[0] += hue_per_i * eff_i + hue_per_j * eff_j + hue_rand * g_random_double_range (-1, 1);
                double notused;
                hsl[0] = modf( hsl[0], &notused ); // Restrict to 0-1
                hsl[1] += saturation_per_i * eff_i + saturation_per_j * eff_j + saturation_rand * g_random_double_range (-1, 1);
                hsl[1] = CLAMP (hsl[1], 0, 1);
                hsl[2] += lightness_per_i * eff_i + lightness_per_j * eff_j + lightness_rand * g_random_double_range (-1, 1);
                hsl[2] = CLAMP (hsl[2], 0, 1);

                float rgb[3];
                sp_color_hsl_to_rgb_floatv (rgb, hsl[0], hsl[1], hsl[2]);
                sp_svg_write_color(color_string, sizeof(color_string), SP_RGBA32_F_COMPOSE(rgb[0], rgb[1], rgb[2], 1.0));
            }

            // Blur
            double blur = 0.0;
            {
            int eff_i = (blur_alternatei? (i%2) : (i));
            int eff_j = (blur_alternatej? (j%2) : (j));
            blur =  (blur_per_i * eff_i + blur_per_j * eff_j + blur_rand * g_random_double_range (-1, 1));
            blur = CLAMP (blur, 0, 1);
            }

            // Opacity
            double opacity = 1.0;
            {
            int eff_i = (opacity_alternatei? (i%2) : (i));
            int eff_j = (opacity_alternatej? (j%2) : (j));
            opacity = 1 - (opacity_per_i * eff_i + opacity_per_j * eff_j + opacity_rand * g_random_double_range (-1, 1));
            opacity = CLAMP (opacity, 0, 1);
            }

            // Trace tab
            if (dotrace) {
                NR::Rect bbox_t = transform_rect (bbox_original, t);

                guint32 rgba = clonetiler_trace_pick (bbox_t);
                float r = SP_RGBA32_R_F(rgba);
                float g = SP_RGBA32_G_F(rgba);
                float b = SP_RGBA32_B_F(rgba);
                float a = SP_RGBA32_A_F(rgba);

                float hsl[3];
                sp_color_rgb_to_hsl_floatv (hsl, r, g, b);

                gdouble val = 0;
                switch (pick) {
                case PICK_COLOR:
                    val = 1 - hsl[2]; // inverse lightness; to match other picks where black = max
                    break;
                case PICK_OPACITY:
                    val = a;
                    break;
                case PICK_R:
                    val = r;
                    break;
                case PICK_G:
                    val = g;
                    break;
                case PICK_B:
                    val = b;
                    break;
                case PICK_H:
                    val = hsl[0];
                    break;
                case PICK_S:
                    val = hsl[1];
                    break;
                case PICK_L:
                    val = 1 - hsl[2];
                    break;
                default:
                    break;
                }

                if (rand_picked > 0) {
                    val = randomize01 (val, rand_picked);
                    r = randomize01 (r, rand_picked);
                    g = randomize01 (g, rand_picked);
                    b = randomize01 (b, rand_picked);
                }

                if (gamma_picked != 0) {
                    double power;
                    if (gamma_picked < 0)
                        power = 1/(1 + fabs(gamma_picked));
                    else
                        power = 1 + gamma_picked;

                    val = pow (val, power);
                    r = pow (r, power);
                    g = pow (g, power);
                    b = pow (b, power);
                }

                if (invert_picked) {
                    val = 1 - val;
                    r = 1 - r;
                    g = 1 - g;
                    b = 1 - b;
                }

                val = CLAMP (val, 0, 1);
                r = CLAMP (r, 0, 1);
                g = CLAMP (g, 0, 1);
                b = CLAMP (b, 0, 1);

                // recompose tweaked color
                rgba = SP_RGBA32_F_COMPOSE(r, g, b, a);

                if (pick_to_presence) {
                    if (g_random_double_range (0, 1) > val) {
                        continue; // skip!
                    }
                }
                if (pick_to_size) {
                    t = NR::translate(-center[NR::X], -center[NR::Y]) * NR::scale (val, val) * NR::translate(center[NR::X], center[NR::Y]) * t;
                }
                if (pick_to_opacity) {
                    opacity *= val;
                }
                if (pick_to_color) {
                    sp_svg_write_color(color_string, sizeof(color_string), rgba);
                }
            }

            if (opacity < 1e-6) { // invisibly transparent, skip
                    continue;
            }

            if (fabs(t[0]) + fabs (t[1]) + fabs(t[2]) + fabs(t[3]) < 1e-6) { // too small, skip
                    continue;
            }

            // Create the clone
            Inkscape::XML::Node *clone = obj_repr->document()->createElement("svg:use");
            clone->setAttribute("x", "0");
            clone->setAttribute("y", "0");
            clone->setAttribute("inkscape:tiled-clone-of", id_href);
            clone->setAttribute("xlink:href", id_href);

            NR::Point new_center;
            bool center_set = false;
            if (obj_repr->attribute("inkscape:transform-center-x") || obj_repr->attribute("inkscape:transform-center-y")) {
                new_center = desktop->dt2doc(SP_ITEM(obj)->getCenter()) * t;
                center_set = true;
            }

            gchar *affinestr=sp_svg_transform_write(t);
            clone->setAttribute("transform", affinestr);
            g_free(affinestr);

            if (opacity < 1.0) {
                sp_repr_set_css_double(clone, "opacity", opacity);
            }

            if (*color_string) {
                clone->setAttribute("fill", color_string);
                clone->setAttribute("stroke", color_string);
            }

            // add the new clone to the top of the original's parent
            SP_OBJECT_REPR(parent)->appendChild(clone);

            if (blur > 0.0) {
                SPObject *clone_object = sp_desktop_document(desktop)->getObjectByRepr(clone);
                double perimeter = perimeter_original * NR::expansion(t);
                double radius = blur * perimeter;
                // this is necessary for all newly added clones to have correct bboxes,
                // otherwise filters won't work:
                sp_document_ensure_up_to_date(sp_desktop_document(desktop));
                // it's hard to figure out exact width/height of the tile without having an object
                // that we can take bbox of; however here we only need a lower bound so that blur
                // margins are not too small, and the perimeter should work
                SPFilter *constructed = new_filter_gaussian_blur(sp_desktop_document(desktop), radius, NR::expansion(t), NR::expansionX(t), NR::expansionY(t), perimeter, perimeter);
                sp_style_set_property_url (clone_object, "filter", SP_OBJECT(constructed), false);
            }

            if (center_set) {
                SPObject *clone_object = sp_desktop_document(desktop)->getObjectByRepr(clone);
                if (clone_object && SP_IS_ITEM(clone_object)) {
                    clone_object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
                    SP_ITEM(clone_object)->setCenter(desktop->doc2dt(new_center));
                    clone_object->updateRepr();
                }
            }

            Inkscape::GC::release(clone);
        }
        cur[NR::Y] = 0;
    }

    if (dotrace) {
        clonetiler_trace_finish ();
    }

    clonetiler_change_selection (NULL, selection, dlg);

    desktop->clearWaitingCursor();

    sp_document_done(sp_desktop_document(desktop), SP_VERB_DIALOG_CLONETILER,
                     _("Create tiled clones"));
}

static GtkWidget *
clonetiler_new_tab (GtkWidget *nb, const gchar *label)
{
    GtkWidget *l = gtk_label_new_with_mnemonic (label);
    GtkWidget *vb = gtk_vbox_new (FALSE, VB_MARGIN);
    gtk_container_set_border_width (GTK_CONTAINER (vb), VB_MARGIN);
    gtk_notebook_append_page (GTK_NOTEBOOK (nb), vb, l);
    return vb;
}

static void
clonetiler_checkbox_toggled (GtkToggleButton *tb, gpointer *data)
{
    const gchar *attr = (const gchar *) data;
    prefs_set_int_attribute (prefs_path, attr, gtk_toggle_button_get_active (tb));
}

static GtkWidget *
clonetiler_checkbox (GtkTooltips *tt, const char *tip, const char *attr)
{
    GtkWidget *hb = gtk_hbox_new(FALSE, VB_MARGIN);

    GtkWidget *b = gtk_check_button_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), b, tip, NULL);

    int value = prefs_get_int_attribute (prefs_path, attr, 0);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(b), value);

    gtk_box_pack_end (GTK_BOX (hb), b, FALSE, TRUE, 0);
    gtk_signal_connect ( GTK_OBJECT (b), "clicked",
                         GTK_SIGNAL_FUNC (clonetiler_checkbox_toggled), (gpointer) attr);

    g_object_set_data (G_OBJECT(b), "uncheckable", GINT_TO_POINTER(TRUE));

    return hb;
}


static void
clonetiler_value_changed (GtkAdjustment *adj, gpointer data)
{
    const gchar *pref = (const gchar *) data;
    prefs_set_double_attribute (prefs_path, pref, adj->value);
}

static GtkWidget *
clonetiler_spinbox (GtkTooltips *tt, const char *tip, const char *attr, double lower, double upper, const gchar *suffix, bool exponent = false)
{
    GtkWidget *hb = gtk_hbox_new(FALSE, 0);

    {
        GtkObject *a;
        if (exponent)
            a = gtk_adjustment_new(1.0, lower, upper, 0.01, 0.05, 0.1);
        else
            a = gtk_adjustment_new(0.0, lower, upper, 0.1, 0.5, 2);

        GtkWidget *sb;
        if (exponent)
            sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 0.01, 2);
        else
            sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 0.1, 1);

        gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), sb, tip, NULL);
        gtk_entry_set_width_chars (GTK_ENTRY (sb), 4);
        gtk_box_pack_start (GTK_BOX (hb), sb, FALSE, FALSE, SB_MARGIN);

        double value = prefs_get_double_attribute_limited (prefs_path, attr, exponent? 1 : 0, lower, upper);
        gtk_adjustment_set_value (GTK_ADJUSTMENT (a), value);
        gtk_signal_connect(GTK_OBJECT(a), "value_changed",
                           GTK_SIGNAL_FUNC(clonetiler_value_changed), (gpointer) attr);

        if (exponent)
            g_object_set_data (G_OBJECT(sb), "oneable", GINT_TO_POINTER(TRUE));
        else
            g_object_set_data (G_OBJECT(sb), "zeroable", GINT_TO_POINTER(TRUE));
    }

    {
        GtkWidget *l = gtk_label_new ("");
        gtk_label_set_markup (GTK_LABEL(l), suffix);
        gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0);
        gtk_box_pack_start (GTK_BOX (hb), l, FALSE, FALSE, 0);
    }

    return hb;
}

static void
clonetiler_symgroup_changed( GtkMenuItem */*item*/, gpointer data )
{
    gint group_new = GPOINTER_TO_INT (data);
    prefs_set_int_attribute ( prefs_path, "symmetrygroup", group_new );
}

static void
clonetiler_xy_changed (GtkAdjustment *adj, gpointer data)
{
    const gchar *pref = (const gchar *) data;
    prefs_set_int_attribute (prefs_path, pref, (int) floor(adj->value + 0.5));
}

static void
clonetiler_keep_bbox_toggled( GtkToggleButton *tb, gpointer /*data*/ )
{
    prefs_set_int_attribute (prefs_path, "keepbbox", gtk_toggle_button_get_active (tb));
}

static void
clonetiler_pick_to (GtkToggleButton *tb, gpointer data)
{
    const gchar *pref = (const gchar *) data;
    prefs_set_int_attribute (prefs_path, pref, gtk_toggle_button_get_active (tb));
}


static void
clonetiler_reset_recursive (GtkWidget *w)
{
    if (w && GTK_IS_OBJECT(w)) {
        {
            int r = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT(w), "zeroable"));
            if (r && GTK_IS_SPIN_BUTTON(w)) { // spinbutton
                GtkAdjustment *a = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON(w));
                gtk_adjustment_set_value (a, 0);
            }
        }
        {
            int r = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT(w), "oneable"));
            if (r && GTK_IS_SPIN_BUTTON(w)) { // spinbutton
                GtkAdjustment *a = gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON(w));
                gtk_adjustment_set_value (a, 1);
            }
        }
        {
            int r = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT(w), "uncheckable"));
            if (r && GTK_IS_TOGGLE_BUTTON(w)) { // checkbox
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(w), FALSE);
            }
        }
    }

    if (GTK_IS_CONTAINER(w)) {
        GList *ch = gtk_container_get_children (GTK_CONTAINER(w));
        for (GList *i = ch; i != NULL; i = i->next) {
            clonetiler_reset_recursive (GTK_WIDGET(i->data));
        }
        g_list_free (ch);
    }
}

static void
clonetiler_reset( GtkWidget */*widget*/, void * )
{
    clonetiler_reset_recursive (dlg);
}

static void
clonetiler_table_attach (GtkWidget *table, GtkWidget *widget, float align, int row, int col)
{
    GtkWidget *a = gtk_alignment_new (align, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(a), widget);
    gtk_table_attach ( GTK_TABLE (table), a, col, col + 1, row, row + 1, (GtkAttachOptions)4, (GtkAttachOptions)0, 0, 0 );
}

static GtkWidget *
clonetiler_table_x_y_rand (int values)
{
    GtkWidget *table = gtk_table_new (values + 2, 5, FALSE);
    gtk_container_set_border_width (GTK_CONTAINER (table), VB_MARGIN);
    gtk_table_set_row_spacings (GTK_TABLE (table), 6);
    gtk_table_set_col_spacings (GTK_TABLE (table), 8);

    {
        GtkWidget *hb = gtk_hbox_new (FALSE, 0);

        GtkWidget *i = sp_icon_new (Inkscape::ICON_SIZE_DECORATION, "clonetiler_per_row");
        gtk_box_pack_start (GTK_BOX (hb), i, FALSE, FALSE, 2);

        GtkWidget *l = gtk_label_new ("");
        gtk_label_set_markup (GTK_LABEL(l), _("<small>Per row:</small>"));
        gtk_box_pack_start (GTK_BOX (hb), l, FALSE, FALSE, 2);

        clonetiler_table_attach (table, hb, 0, 1, 2);
    }

    {
        GtkWidget *hb = gtk_hbox_new (FALSE, 0);

        GtkWidget *i = sp_icon_new (Inkscape::ICON_SIZE_DECORATION, "clonetiler_per_column");
        gtk_box_pack_start (GTK_BOX (hb), i, FALSE, FALSE, 2);

        GtkWidget *l = gtk_label_new ("");
        gtk_label_set_markup (GTK_LABEL(l), _("<small>Per column:</small>"));
        gtk_box_pack_start (GTK_BOX (hb), l, FALSE, FALSE, 2);

        clonetiler_table_attach (table, hb, 0, 1, 3);
    }

    {
        GtkWidget *l = gtk_label_new ("");
        gtk_label_set_markup (GTK_LABEL(l), _("<small>Randomize:</small>"));
        clonetiler_table_attach (table, l, 0, 1, 4);
    }

    return table;
}

static void
clonetiler_pick_switched( GtkToggleButton */*tb*/, gpointer data )
{
    guint v = GPOINTER_TO_INT (data);
    prefs_set_int_attribute (prefs_path, "pick", v);
}


static void
clonetiler_switch_to_create( GtkToggleButton */*tb*/, GtkWidget *dlg )
{
    GtkWidget *rowscols = (GtkWidget *) g_object_get_data (G_OBJECT(dlg), "rowscols");
    GtkWidget *widthheight = (GtkWidget *) g_object_get_data (G_OBJECT(dlg), "widthheight");

    if (rowscols) {
        gtk_widget_set_sensitive (rowscols, TRUE);
    }
    if (widthheight) {
        gtk_widget_set_sensitive (widthheight, FALSE);
    }

    prefs_set_int_attribute (prefs_path, "fillrect", 0);
}


static void
clonetiler_switch_to_fill( GtkToggleButton */*tb*/, GtkWidget *dlg )
{
    GtkWidget *rowscols = (GtkWidget *) g_object_get_data (G_OBJECT(dlg), "rowscols");
    GtkWidget *widthheight = (GtkWidget *) g_object_get_data (G_OBJECT(dlg), "widthheight");

    if (rowscols) {
        gtk_widget_set_sensitive (rowscols, FALSE);
    }
    if (widthheight) {
        gtk_widget_set_sensitive (widthheight, TRUE);
    }

    prefs_set_int_attribute (prefs_path, "fillrect", 1);
}




static void
clonetiler_fill_width_changed (GtkAdjustment *adj, GtkWidget *u)
{
    gdouble const raw_dist = adj->value;
    SPUnit const &unit = *sp_unit_selector_get_unit(SP_UNIT_SELECTOR(u));
    gdouble const pixels = sp_units_get_pixels (raw_dist, unit);

    prefs_set_double_attribute (prefs_path, "fillwidth", pixels);
}

static void
clonetiler_fill_height_changed (GtkAdjustment *adj, GtkWidget *u)
{
    gdouble const raw_dist = adj->value;
    SPUnit const &unit = *sp_unit_selector_get_unit(SP_UNIT_SELECTOR(u));
    gdouble const pixels = sp_units_get_pixels (raw_dist, unit);

    prefs_set_double_attribute (prefs_path, "fillheight", pixels);
}


static void
clonetiler_do_pick_toggled( GtkToggleButton *tb, gpointer /*data*/ )
{
    GtkWidget *vvb = (GtkWidget *) g_object_get_data (G_OBJECT(dlg), "dotrace");

    prefs_set_int_attribute (prefs_path, "dotrace", gtk_toggle_button_get_active (tb));

    if (vvb)
        gtk_widget_set_sensitive (vvb, gtk_toggle_button_get_active (tb));
}




void
clonetiler_dialog (void)
{
    if (!dlg)
    {
        gchar title[500];
        sp_ui_dialog_title_string (Inkscape::Verb::get(SP_VERB_DIALOG_CLONETILER), title);

        dlg = sp_window_new (title, TRUE);
        if (x == -1000 || y == -1000) {
            x = prefs_get_int_attribute (prefs_path, "x", -1000);
            y = prefs_get_int_attribute (prefs_path, "y", -1000);
        }

        if (w ==0 || h == 0) {
            w = prefs_get_int_attribute (prefs_path, "w", 0);
            h = prefs_get_int_attribute (prefs_path, "h", 0);
        }

//        if (x<0) x=0;
//        if (y<0) y=0;

        if (w && h) {
            gtk_window_resize ((GtkWindow *) dlg, w, h);
        }
        if (x >= 0 && y >= 0 && (x < (gdk_screen_width()-MIN_ONSCREEN_DISTANCE)) && (y < (gdk_screen_height()-MIN_ONSCREEN_DISTANCE))) {
            gtk_window_move ((GtkWindow *) dlg, x, y);

        } else {
            gtk_window_set_position(GTK_WINDOW(dlg), GTK_WIN_POS_CENTER);
        }


        sp_transientize (dlg);
        wd.win = dlg;
        wd.stop = 0;


        gtk_signal_connect ( GTK_OBJECT (dlg), "event", GTK_SIGNAL_FUNC (sp_dialog_event_handler), dlg);

        gtk_signal_connect ( GTK_OBJECT (dlg), "destroy", G_CALLBACK (clonetiler_dialog_destroy), dlg);
        gtk_signal_connect ( GTK_OBJECT (dlg), "delete_event", G_CALLBACK (clonetiler_dialog_delete), dlg);

        if (Inkscape::NSApplication::Application::getNewGui())
        {
            _shutdown_connection = Inkscape::NSApplication::Editor::connectShutdown (&on_delete);
            _dialogs_hidden_connection = Inkscape::NSApplication::Editor::connectDialogsHidden (sigc::bind (&on_dialog_hide, dlg));
            _dialogs_unhidden_connection = Inkscape::NSApplication::Editor::connectDialogsUnhidden (sigc::bind (&on_dialog_unhide, dlg));
            _desktop_activated_connection = Inkscape::NSApplication::Editor::connectDesktopActivated (sigc::bind (&on_transientize, &wd));
        } else {
            g_signal_connect   ( G_OBJECT (INKSCAPE), "shut_down", G_CALLBACK (clonetiler_dialog_delete), dlg);
            g_signal_connect   ( G_OBJECT (INKSCAPE), "dialogs_hide", G_CALLBACK (sp_dialog_hide), dlg);
            g_signal_connect   ( G_OBJECT (INKSCAPE), "dialogs_unhide", G_CALLBACK (sp_dialog_unhide), dlg);
            g_signal_connect   ( G_OBJECT (INKSCAPE), "activate_desktop", G_CALLBACK (sp_transientize_callback), &wd);
        }

        GtkTooltips *tt = gtk_tooltips_new();

        GtkWidget *mainbox = gtk_vbox_new(FALSE, 4);
        gtk_container_set_border_width (GTK_CONTAINER (mainbox), 6);
        gtk_container_add (GTK_CONTAINER (dlg), mainbox);

        GtkWidget *nb = gtk_notebook_new ();
        gtk_box_pack_start (GTK_BOX (mainbox), nb, FALSE, FALSE, 0);


// Symmetry
        {
            GtkWidget *vb = clonetiler_new_tab (nb, _("_Symmetry"));

            GtkWidget *om = gtk_option_menu_new ();
            /* TRANSLATORS: For the following 17 symmetry groups, see
             * http://www.bib.ulb.ac.be/coursmath/doc/17.htm (visual examples);
             * http://www.clarku.edu/~djoyce/wallpaper/seventeen.html (English vocabulary); or
             * http://membres.lycos.fr/villemingerard/Geometri/Sym1D.htm (French vocabulary).
             */
            gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), om, _("Select one of the 17 symmetry groups for the tiling"), NULL);
            gtk_box_pack_start (GTK_BOX (vb), om, FALSE, FALSE, SB_MARGIN);

            GtkWidget *m = gtk_menu_new ();
            int current = prefs_get_int_attribute (prefs_path, "symmetrygroup", 0);

            struct SymGroups {
                int group;
                gchar const *label;
            } const sym_groups[] = {
                // TRANSLATORS: "translation" means "shift" / "displacement" here.
                {TILE_P1, _("<b>P1</b>: simple translation")},
                {TILE_P2, _("<b>P2</b>: 180&#176; rotation")},
                {TILE_PM, _("<b>PM</b>: reflection")},
                // TRANSLATORS: "glide reflection" is a reflection and a translation combined.
                //  For more info, see http://mathforum.org/sum95/suzanne/symsusan.html
                {TILE_PG, _("<b>PG</b>: glide reflection")},
                {TILE_CM, _("<b>CM</b>: reflection + glide reflection")},
                {TILE_PMM, _("<b>PMM</b>: reflection + reflection")},
                {TILE_PMG, _("<b>PMG</b>: reflection + 180&#176; rotation")},
                {TILE_PGG, _("<b>PGG</b>: glide reflection + 180&#176; rotation")},
                {TILE_CMM, _("<b>CMM</b>: reflection + reflection + 180&#176; rotation")},
                {TILE_P4, _("<b>P4</b>: 90&#176; rotation")},
                {TILE_P4M, _("<b>P4M</b>: 90&#176; rotation + 45&#176; reflection")},
                {TILE_P4G, _("<b>P4G</b>: 90&#176; rotation + 90&#176; reflection")},
                {TILE_P3, _("<b>P3</b>: 120&#176; rotation")},
                {TILE_P31M, _("<b>P31M</b>: reflection + 120&#176; rotation, dense")},
                {TILE_P3M1, _("<b>P3M1</b>: reflection + 120&#176; rotation, sparse")},
                {TILE_P6, _("<b>P6</b>: 60&#176; rotation")},
                {TILE_P6M, _("<b>P6M</b>: reflection + 60&#176; rotation")},
            };

            for (unsigned j = 0; j < G_N_ELEMENTS(sym_groups); ++j) {
                SymGroups const &sg = sym_groups[j];

                GtkWidget *l = gtk_label_new ("");
                gtk_label_set_markup (GTK_LABEL(l), sg.label);
                gtk_misc_set_alignment (GTK_MISC(l), 0, 0.5);

                GtkWidget *item = gtk_menu_item_new ();
                gtk_container_add (GTK_CONTAINER (item), l);

                gtk_signal_connect ( GTK_OBJECT (item), "activate",
                                     GTK_SIGNAL_FUNC (clonetiler_symgroup_changed),
                                     GINT_TO_POINTER (sg.group) );

                gtk_menu_append (GTK_MENU (m), item);
            }

            gtk_option_menu_set_menu (GTK_OPTION_MENU (om), m);
            gtk_option_menu_set_history ( GTK_OPTION_MENU (om), current);
        }

        table_row_labels = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

// Shift
        {
            GtkWidget *vb = clonetiler_new_tab (nb, _("S_hift"));

            GtkWidget *table = clonetiler_table_x_y_rand (3);
            gtk_box_pack_start (GTK_BOX (vb), table, FALSE, FALSE, 0);

            // X
            {
                GtkWidget *l = gtk_label_new ("");
                    // TRANSLATORS: "shift" means: the tiles will be shifted (offset) horizontally by this amount
                    // xgettext:no-c-format
                gtk_label_set_markup (GTK_LABEL(l), _("<b>Shift X:</b>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 2, 1);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                    // xgettext:no-c-format
                                                   _("Horizontal shift per row (in % of tile width)"), "shiftx_per_j",
                                                   -10000, 10000, "%");
                clonetiler_table_attach (table, l, 0, 2, 2);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                    // xgettext:no-c-format
                                                   _("Horizontal shift per column (in % of tile width)"), "shiftx_per_i",
                                                   -10000, 10000, "%");
                clonetiler_table_attach (table, l, 0, 2, 3);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                                                   _("Randomize the horizontal shift by this percentage"), "shiftx_rand",
                                                   0, 1000, "%");
                clonetiler_table_attach (table, l, 0, 2, 4);
            }

            // Y
            {
                GtkWidget *l = gtk_label_new ("");
                    // TRANSLATORS: "shift" means: the tiles will be shifted (offset) vertically by this amount
                    // xgettext:no-c-format
                gtk_label_set_markup (GTK_LABEL(l), _("<b>Shift Y:</b>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 3, 1);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                    // xgettext:no-c-format
                                                   _("Vertical shift per row (in % of tile height)"), "shifty_per_j",
                                                   -10000, 10000, "%");
                clonetiler_table_attach (table, l, 0, 3, 2);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                    // xgettext:no-c-format
                                                   _("Vertical shift per column (in % of tile height)"), "shifty_per_i",
                                                   -10000, 10000, "%");
                clonetiler_table_attach (table, l, 0, 3, 3);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                                                   _("Randomize the vertical shift by this percentage"), "shifty_rand",
                                                   0, 1000, "%");
                clonetiler_table_attach (table, l, 0, 3, 4);
            }

            // Exponent
            {
                GtkWidget *l = gtk_label_new ("");
                gtk_label_set_markup (GTK_LABEL(l), _("<b>Exponent:</b>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 4, 1);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                                                   _("Whether rows are spaced evenly (1), converge (<1) or diverge (>1)"), "shifty_exp",
                                                   0, 10, "", true);
                clonetiler_table_attach (table, l, 0, 4, 2);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                                                   _("Whether columns are spaced evenly (1), converge (<1) or diverge (>1)"), "shiftx_exp",
                                                   0, 10, "", true);
                clonetiler_table_attach (table, l, 0, 4, 3);
            }

            { // alternates
                GtkWidget *l = gtk_label_new ("");
                // TRANSLATORS: "Alternate" is a verb here
                gtk_label_set_markup (GTK_LABEL(l), _("<small>Alternate:</small>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 5, 1);
            }

            {
                GtkWidget *l = clonetiler_checkbox (tt, _("Alternate the sign of shifts for each row"), "shifty_alternate");
                clonetiler_table_attach (table, l, 0, 5, 2);
            }

            {
                GtkWidget *l = clonetiler_checkbox (tt, _("Alternate the sign of shifts for each column"), "shiftx_alternate");
                clonetiler_table_attach (table, l, 0, 5, 3);
            }

            { // Cumulate
                GtkWidget *l = gtk_label_new ("");
                // TRANSLATORS: "Cumulate" is a verb here
                gtk_label_set_markup (GTK_LABEL(l), _("<small>Cumulate:</small>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 6, 1);
            }

            {
                GtkWidget *l = clonetiler_checkbox (tt, _("Cumulate the shifts for each row"), "shifty_cumulate");
                clonetiler_table_attach (table, l, 0, 6, 2);
            }

            {
                GtkWidget *l = clonetiler_checkbox (tt, _("Cumulate the shifts for each column"), "shiftx_cumulate");
                clonetiler_table_attach (table, l, 0, 6, 3);
            }

            { // Exclude tile width and height in shift
                GtkWidget *l = gtk_label_new ("");
                // TRANSLATORS: "Cumulate" is a verb here
                gtk_label_set_markup (GTK_LABEL(l), _("<small>Exclude tile:</small>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 7, 1);
            }

            {
                GtkWidget *l = clonetiler_checkbox (tt, _("Exclude tile height in shift"), "shifty_excludeh");
                clonetiler_table_attach (table, l, 0, 7, 2);
            }

            {
                GtkWidget *l = clonetiler_checkbox (tt, _("Exclude tile width in shift"), "shiftx_excludew");
                clonetiler_table_attach (table, l, 0, 7, 3);
            }

        }


// Scale
        {
            GtkWidget *vb = clonetiler_new_tab (nb, _("Sc_ale"));

            GtkWidget *table = clonetiler_table_x_y_rand (2);
            gtk_box_pack_start (GTK_BOX (vb), table, FALSE, FALSE, 0);

            // X
            {
                GtkWidget *l = gtk_label_new ("");
                gtk_label_set_markup (GTK_LABEL(l), _("<b>Scale X:</b>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 2, 1);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                    // xgettext:no-c-format
                                                   _("Horizontal scale per row (in % of tile width)"), "scalex_per_j",
                                                   -100, 1000, "%");
                clonetiler_table_attach (table, l, 0, 2, 2);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                    // xgettext:no-c-format
                                                   _("Horizontal scale per column (in % of tile width)"), "scalex_per_i",
                                                   -100, 1000, "%");
                clonetiler_table_attach (table, l, 0, 2, 3);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                                                   _("Randomize the horizontal scale by this percentage"), "scalex_rand",
                                                   0, 1000, "%");
                clonetiler_table_attach (table, l, 0, 2, 4);
            }

            // Y
            {
                GtkWidget *l = gtk_label_new ("");
                gtk_label_set_markup (GTK_LABEL(l), _("<b>Scale Y:</b>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 3, 1);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                    // xgettext:no-c-format
                                                   _("Vertical scale per row (in % of tile height)"), "scaley_per_j",
                                                   -100, 1000, "%");
                clonetiler_table_attach (table, l, 0, 3, 2);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                    // xgettext:no-c-format
                                                   _("Vertical scale per column (in % of tile height)"), "scaley_per_i",
                                                   -100, 1000, "%");
                clonetiler_table_attach (table, l, 0, 3, 3);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                                                   _("Randomize the vertical scale by this percentage"), "scaley_rand",
                                                   0, 1000, "%");
                clonetiler_table_attach (table, l, 0, 3, 4);
            }

            // Exponent
            {
                GtkWidget *l = gtk_label_new ("");
                gtk_label_set_markup (GTK_LABEL(l), _("<b>Exponent:</b>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 4, 1);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                                                   _("Whether row scaling is uniform (1), converge (<1) or diverge (>1)"), "scaley_exp",
                                                   0, 10, "", true);
                clonetiler_table_attach (table, l, 0, 4, 2);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                                                   _("Whether column scaling is uniform (1), converge (<1) or diverge (>1)"), "scalex_exp",
                                                   0, 10, "", true);
                clonetiler_table_attach (table, l, 0, 4, 3);
            }

            // Logarithmic (as in logarithmic spiral)
            {
                GtkWidget *l = gtk_label_new ("");
                gtk_label_set_markup (GTK_LABEL(l), _("<b>Base:</b>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 5, 1);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                                                   _("Base for a logarithmic spiral: not used (0), converge (<1), or diverge (>1)"), "scaley_log",
                                                   0, 10, "", false);
                clonetiler_table_attach (table, l, 0, 5, 2);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                                                   _("Base for a logarithmic spiral: not used (0), converge (<1), or diverge (>1)"), "scalex_log",
                                                   0, 10, "", false);
                clonetiler_table_attach (table, l, 0, 5, 3);
            }

            { // alternates
                GtkWidget *l = gtk_label_new ("");
                // TRANSLATORS: "Alternate" is a verb here
                gtk_label_set_markup (GTK_LABEL(l), _("<small>Alternate:</small>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 6, 1);
            }

            {
                GtkWidget *l = clonetiler_checkbox (tt, _("Alternate the sign of scales for each row"), "scaley_alternate");
                clonetiler_table_attach (table, l, 0, 6, 2);
            }

            {
                GtkWidget *l = clonetiler_checkbox (tt, _("Alternate the sign of scales for each column"), "scalex_alternate");
                clonetiler_table_attach (table, l, 0, 6, 3);
            }

            { // Cumulate
                GtkWidget *l = gtk_label_new ("");
                // TRANSLATORS: "Cumulate" is a verb here
                gtk_label_set_markup (GTK_LABEL(l), _("<small>Cumulate:</small>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 7, 1);
            }

            {
                GtkWidget *l = clonetiler_checkbox (tt, _("Cumulate the scales for each row"), "scaley_cumulate");
                clonetiler_table_attach (table, l, 0, 7, 2);
            }

            {
                GtkWidget *l = clonetiler_checkbox (tt, _("Cumulate the scales for each column"), "scalex_cumulate");
                clonetiler_table_attach (table, l, 0, 7, 3);
            }

        }


// Rotation
        {
            GtkWidget *vb = clonetiler_new_tab (nb, _("_Rotation"));

            GtkWidget *table = clonetiler_table_x_y_rand (1);
            gtk_box_pack_start (GTK_BOX (vb), table, FALSE, FALSE, 0);

            // Angle
            {
                GtkWidget *l = gtk_label_new ("");
                gtk_label_set_markup (GTK_LABEL(l), _("<b>Angle:</b>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 2, 1);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                    // xgettext:no-c-format
                                                   _("Rotate tiles by this angle for each row"), "rotate_per_j",
                                                   -180, 180, "&#176;");
                clonetiler_table_attach (table, l, 0, 2, 2);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                    // xgettext:no-c-format
                                                   _("Rotate tiles by this angle for each column"), "rotate_per_i",
                                                   -180, 180, "&#176;");
                clonetiler_table_attach (table, l, 0, 2, 3);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                                                   _("Randomize the rotation angle by this percentage"), "rotate_rand",
                                                   0, 100, "%");
                clonetiler_table_attach (table, l, 0, 2, 4);
            }

            { // alternates
                GtkWidget *l = gtk_label_new ("");
                // TRANSLATORS: "Alternate" is a verb here
                gtk_label_set_markup (GTK_LABEL(l), _("<small>Alternate:</small>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 3, 1);
            }

            {
                GtkWidget *l = clonetiler_checkbox (tt, _("Alternate the rotation direction for each row"), "rotate_alternatej");
                clonetiler_table_attach (table, l, 0, 3, 2);
            }

            {
                GtkWidget *l = clonetiler_checkbox (tt, _("Alternate the rotation direction for each column"), "rotate_alternatei");
                clonetiler_table_attach (table, l, 0, 3, 3);
            }

            { // Cumulate
                GtkWidget *l = gtk_label_new ("");
                // TRANSLATORS: "Cumulate" is a verb here
                gtk_label_set_markup (GTK_LABEL(l), _("<small>Cumulate:</small>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 4, 1);
            }

            {
                GtkWidget *l = clonetiler_checkbox (tt, _("Cumulate the rotation for each row"), "rotate_cumulatej");
                clonetiler_table_attach (table, l, 0, 4, 2);
            }

            {
                GtkWidget *l = clonetiler_checkbox (tt, _("Cumulate the rotation for each column"), "rotate_cumulatei");
                clonetiler_table_attach (table, l, 0, 4, 3);
            }

        }


// Blur and opacity
        {
            GtkWidget *vb = clonetiler_new_tab (nb, _("_Blur & opacity"));

            GtkWidget *table = clonetiler_table_x_y_rand (1);
            gtk_box_pack_start (GTK_BOX (vb), table, FALSE, FALSE, 0);


            // Blur
            {
                GtkWidget *l = gtk_label_new ("");
                gtk_label_set_markup (GTK_LABEL(l), _("<b>Blur:</b>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 2, 1);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                                                   _("Blur tiles by this percentage for each row"), "blur_per_j",
                                                   0, 100, "%");
                clonetiler_table_attach (table, l, 0, 2, 2);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                                                   _("Blur tiles by this percentage for each column"), "blur_per_i",
                                                   0, 100, "%");
                clonetiler_table_attach (table, l, 0, 2, 3);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                                                   _("Randomize the tile blur by this percentage"), "blur_rand",
                                                   0, 100, "%");
                clonetiler_table_attach (table, l, 0, 2, 4);
            }

            { // alternates
                GtkWidget *l = gtk_label_new ("");
                // TRANSLATORS: "Alternate" is a verb here
                gtk_label_set_markup (GTK_LABEL(l), _("<small>Alternate:</small>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 3, 1);
            }

            {
                GtkWidget *l = clonetiler_checkbox (tt, _("Alternate the sign of blur change for each row"), "blur_alternatej");
                clonetiler_table_attach (table, l, 0, 3, 2);
            }

            {
                GtkWidget *l = clonetiler_checkbox (tt, _("Alternate the sign of blur change for each column"), "blur_alternatei");
                clonetiler_table_attach (table, l, 0, 3, 3);
            }



            // Dissolve
            {
                GtkWidget *l = gtk_label_new ("");
                gtk_label_set_markup (GTK_LABEL(l), _("<b>Fade out:</b>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 4, 1);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                                                   _("Decrease tile opacity by this percentage for each row"), "opacity_per_j",
                                                   0, 100, "%");
                clonetiler_table_attach (table, l, 0, 4, 2);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                                                   _("Decrease tile opacity by this percentage for each column"), "opacity_per_i",
                                                   0, 100, "%");
                clonetiler_table_attach (table, l, 0, 4, 3);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                                                   _("Randomize the tile opacity by this percentage"), "opacity_rand",
                                                   0, 100, "%");
                clonetiler_table_attach (table, l, 0, 4, 4);
            }

            { // alternates
                GtkWidget *l = gtk_label_new ("");
                // TRANSLATORS: "Alternate" is a verb here
                gtk_label_set_markup (GTK_LABEL(l), _("<small>Alternate:</small>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 5, 1);
            }

            {
                GtkWidget *l = clonetiler_checkbox (tt, _("Alternate the sign of opacity change for each row"), "opacity_alternatej");
                clonetiler_table_attach (table, l, 0, 5, 2);
            }

            {
                GtkWidget *l = clonetiler_checkbox (tt, _("Alternate the sign of opacity change for each column"), "opacity_alternatei");
                clonetiler_table_attach (table, l, 0, 5, 3);
            }
        }


// Color
        {
            GtkWidget *vb = clonetiler_new_tab (nb, _("Co_lor"));

            {
            GtkWidget *hb = gtk_hbox_new (FALSE, 0);

            GtkWidget *l = gtk_label_new (_("Initial color: "));
            gtk_box_pack_start (GTK_BOX (hb), l, FALSE, FALSE, 0);

            guint32 rgba = 0x000000ff | sp_svg_read_color (prefs_get_string_attribute(prefs_path, "initial_color"), 0x000000ff);
            color_picker = new Inkscape::UI::Widget::ColorPicker (*new Glib::ustring(_("Initial color of tiled clones")), *new Glib::ustring(_("Initial color for clones (works only if the original has unset fill or stroke)")), rgba, false);
            _color_changed_connection = color_picker->connectChanged (sigc::ptr_fun(on_picker_color_changed));

            gtk_box_pack_start (GTK_BOX (hb), reinterpret_cast<GtkWidget*>(color_picker->gobj()), FALSE, FALSE, 0);

            gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);
            }


            GtkWidget *table = clonetiler_table_x_y_rand (3);
            gtk_box_pack_start (GTK_BOX (vb), table, FALSE, FALSE, 0);

            // Hue
            {
                GtkWidget *l = gtk_label_new ("");
                gtk_label_set_markup (GTK_LABEL(l), _("<b>H:</b>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 2, 1);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                                                   _("Change the tile hue by this percentage for each row"), "hue_per_j",
                                                   -100, 100, "%");
                clonetiler_table_attach (table, l, 0, 2, 2);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                                                   _("Change the tile hue by this percentage for each column"), "hue_per_i",
                                                   -100, 100, "%");
                clonetiler_table_attach (table, l, 0, 2, 3);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                                                   _("Randomize the tile hue by this percentage"), "hue_rand",
                                                   0, 100, "%");
                clonetiler_table_attach (table, l, 0, 2, 4);
            }


            // Saturation
            {
                GtkWidget *l = gtk_label_new ("");
                gtk_label_set_markup (GTK_LABEL(l), _("<b>S:</b>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 3, 1);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                                                   _("Change the color saturation by this percentage for each row"), "saturation_per_j",
                                                   -100, 100, "%");
                clonetiler_table_attach (table, l, 0, 3, 2);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                                                   _("Change the color saturation by this percentage for each column"), "saturation_per_i",
                                                   -100, 100, "%");
                clonetiler_table_attach (table, l, 0, 3, 3);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                                                   _("Randomize the color saturation by this percentage"), "saturation_rand",
                                                   0, 100, "%");
                clonetiler_table_attach (table, l, 0, 3, 4);
            }

            // Lightness
            {
                GtkWidget *l = gtk_label_new ("");
                gtk_label_set_markup (GTK_LABEL(l), _("<b>L:</b>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 4, 1);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                                                   _("Change the color lightness by this percentage for each row"), "lightness_per_j",
                                                   -100, 100, "%");
                clonetiler_table_attach (table, l, 0, 4, 2);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                                                   _("Change the color lightness by this percentage for each column"), "lightness_per_i",
                                                   -100, 100, "%");
                clonetiler_table_attach (table, l, 0, 4, 3);
            }

            {
                GtkWidget *l = clonetiler_spinbox (tt,
                                                   _("Randomize the color lightness by this percentage"), "lightness_rand",
                                                   0, 100, "%");
                clonetiler_table_attach (table, l, 0, 4, 4);
            }


            { // alternates
                GtkWidget *l = gtk_label_new ("");
                gtk_label_set_markup (GTK_LABEL(l), _("<small>Alternate:</small>"));
                gtk_size_group_add_widget(table_row_labels, l);
                clonetiler_table_attach (table, l, 1, 5, 1);
            }

            {
                GtkWidget *l = clonetiler_checkbox (tt, _("Alternate the sign of color changes for each row"), "color_alternatej");
                clonetiler_table_attach (table, l, 0, 5, 2);
            }

            {
                GtkWidget *l = clonetiler_checkbox (tt, _("Alternate the sign of color changes for each column"), "color_alternatei");
                clonetiler_table_attach (table, l, 0, 5, 3);
            }

        }

// Trace
        {
            GtkWidget *vb = clonetiler_new_tab (nb, _("_Trace"));


        {
            GtkWidget *hb = gtk_hbox_new(FALSE, VB_MARGIN);
            gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 0);

            GtkWidget *b  = gtk_check_button_new_with_label (_("Trace the drawing under the tiles"));
            g_object_set_data (G_OBJECT(b), "uncheckable", GINT_TO_POINTER(TRUE));
            gint old = prefs_get_int_attribute (prefs_path, "dotrace", 0);
            gtk_toggle_button_set_active ((GtkToggleButton *) b, old != 0);
            gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), b, _("For each clone, pick a value from the drawing in that clone's location and apply it to the clone"), NULL);
            gtk_box_pack_start (GTK_BOX (hb), b, FALSE, FALSE, 0);

            gtk_signal_connect(GTK_OBJECT(b), "toggled",
                               GTK_SIGNAL_FUNC(clonetiler_do_pick_toggled), dlg);
        }

        {
            GtkWidget *vvb = gtk_vbox_new (FALSE, 0);
            gtk_box_pack_start (GTK_BOX (vb), vvb, FALSE, FALSE, 0);
            g_object_set_data (G_OBJECT(dlg), "dotrace", (gpointer) vvb);


            {
                GtkWidget *frame = gtk_frame_new (_("1. Pick from the drawing:"));
                gtk_box_pack_start (GTK_BOX (vvb), frame, FALSE, FALSE, 0);

                GtkWidget *table = gtk_table_new (3, 3, FALSE);
                gtk_table_set_row_spacings (GTK_TABLE (table), 4);
                gtk_table_set_col_spacings (GTK_TABLE (table), 6);
                gtk_container_add(GTK_CONTAINER(frame), table);


                GtkWidget* radio;
                {
                    radio = gtk_radio_button_new_with_label (NULL, _("Color"));
                    gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), radio, _("Pick the visible color and opacity"), NULL);
                    clonetiler_table_attach (table, radio, 0.0, 1, 1);
                    gtk_signal_connect (GTK_OBJECT (radio), "toggled",
                                        GTK_SIGNAL_FUNC (clonetiler_pick_switched), GINT_TO_POINTER(PICK_COLOR));
                    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), prefs_get_int_attribute(prefs_path, "pick", 0) == PICK_COLOR);
                }
                {
                    radio = gtk_radio_button_new_with_label (gtk_radio_button_group (GTK_RADIO_BUTTON (radio)), _("Opacity"));
                    gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), radio, _("Pick the total accumulated opacity"), NULL);
                    clonetiler_table_attach (table, radio, 0.0, 2, 1);
                    gtk_signal_connect (GTK_OBJECT (radio), "toggled",
                                        GTK_SIGNAL_FUNC (clonetiler_pick_switched), GINT_TO_POINTER(PICK_OPACITY));
                    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), prefs_get_int_attribute(prefs_path, "pick", 0) == PICK_OPACITY);
                }
                {
                    radio = gtk_radio_button_new_with_label (gtk_radio_button_group (GTK_RADIO_BUTTON (radio)), _("R"));
                    gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), radio, _("Pick the Red component of the color"), NULL);
                    clonetiler_table_attach (table, radio, 0.0, 1, 2);
                    gtk_signal_connect (GTK_OBJECT (radio), "toggled",
                                        GTK_SIGNAL_FUNC (clonetiler_pick_switched), GINT_TO_POINTER(PICK_R));
                    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), prefs_get_int_attribute(prefs_path, "pick", 0) == PICK_R);
                }
                {
                    radio = gtk_radio_button_new_with_label (gtk_radio_button_group (GTK_RADIO_BUTTON (radio)), _("G"));
                    gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), radio, _("Pick the Green component of the color"), NULL);
                    clonetiler_table_attach (table, radio, 0.0, 2, 2);
                    gtk_signal_connect (GTK_OBJECT (radio), "toggled",
                                        GTK_SIGNAL_FUNC (clonetiler_pick_switched), GINT_TO_POINTER(PICK_G));
                    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), prefs_get_int_attribute(prefs_path, "pick", 0) == PICK_G);
                }
                {
                    radio = gtk_radio_button_new_with_label (gtk_radio_button_group (GTK_RADIO_BUTTON (radio)), _("B"));
                    gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), radio, _("Pick the Blue component of the color"), NULL);
                    clonetiler_table_attach (table, radio, 0.0, 3, 2);
                    gtk_signal_connect (GTK_OBJECT (radio), "toggled",
                                        GTK_SIGNAL_FUNC (clonetiler_pick_switched), GINT_TO_POINTER(PICK_B));
                    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), prefs_get_int_attribute(prefs_path, "pick", 0) == PICK_B);
                }
                {
                    //TRANSLATORS: only translate "string" in "context|string".
                    // For more details, see http://developer.gnome.org/doc/API/2.0/glib/glib-I18N.html#Q-:CAPS
                    radio = gtk_radio_button_new_with_label (gtk_radio_button_group (GTK_RADIO_BUTTON (radio)), Q_("clonetiler|H"));
                    gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), radio, _("Pick the hue of the color"), NULL);
                    clonetiler_table_attach (table, radio, 0.0, 1, 3);
                    gtk_signal_connect (GTK_OBJECT (radio), "toggled",
                                        GTK_SIGNAL_FUNC (clonetiler_pick_switched), GINT_TO_POINTER(PICK_H));
                    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), prefs_get_int_attribute(prefs_path, "pick", 0) == PICK_H);
                }
                {
                    //TRANSLATORS: only translate "string" in "context|string".
                    // For more details, see http://developer.gnome.org/doc/API/2.0/glib/glib-I18N.html#Q-:CAPS
                    radio = gtk_radio_button_new_with_label (gtk_radio_button_group (GTK_RADIO_BUTTON (radio)), Q_("clonetiler|S"));
                    gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), radio, _("Pick the saturation of the color"), NULL);
                    clonetiler_table_attach (table, radio, 0.0, 2, 3);
                    gtk_signal_connect (GTK_OBJECT (radio), "toggled",
                                        GTK_SIGNAL_FUNC (clonetiler_pick_switched), GINT_TO_POINTER(PICK_S));
                    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), prefs_get_int_attribute(prefs_path, "pick", 0) == PICK_S);
                }
                {
                    //TRANSLATORS: only translate "string" in "context|string".
                    // For more details, see http://developer.gnome.org/doc/API/2.0/glib/glib-I18N.html#Q-:CAPS
                    radio = gtk_radio_button_new_with_label (gtk_radio_button_group (GTK_RADIO_BUTTON (radio)), Q_("clonetiler|L"));
                    gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), radio, _("Pick the lightness of the color"), NULL);
                    clonetiler_table_attach (table, radio, 0.0, 3, 3);
                    gtk_signal_connect (GTK_OBJECT (radio), "toggled",
                                        GTK_SIGNAL_FUNC (clonetiler_pick_switched), GINT_TO_POINTER(PICK_L));
                    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), prefs_get_int_attribute(prefs_path, "pick", 0) == PICK_L);
                }

            }

            {
                GtkWidget *frame = gtk_frame_new (_("2. Tweak the picked value:"));
                gtk_box_pack_start (GTK_BOX (vvb), frame, FALSE, FALSE, VB_MARGIN);

                GtkWidget *table = gtk_table_new (4, 2, FALSE);
                gtk_table_set_row_spacings (GTK_TABLE (table), 4);
                gtk_table_set_col_spacings (GTK_TABLE (table), 6);
                gtk_container_add(GTK_CONTAINER(frame), table);

                {
                    GtkWidget *l = gtk_label_new ("");
                    gtk_label_set_markup (GTK_LABEL(l), _("Gamma-correct:"));
                    clonetiler_table_attach (table, l, 1.0, 1, 1);
                }
                {
                    GtkWidget *l = clonetiler_spinbox (tt,
                                                       _("Shift the mid-range of the picked value upwards (>0) or downwards (<0)"), "gamma_picked",
                                                       -10, 10, "");
                    clonetiler_table_attach (table, l, 0.0, 1, 2);
                }

                {
                    GtkWidget *l = gtk_label_new ("");
                    gtk_label_set_markup (GTK_LABEL(l), _("Randomize:"));
                    clonetiler_table_attach (table, l, 1.0, 1, 3);
                }
                {
                    GtkWidget *l = clonetiler_spinbox (tt,
                                                       _("Randomize the picked value by this percentage"), "rand_picked",
                                                       0, 100, "%");
                    clonetiler_table_attach (table, l, 0.0, 1, 4);
                }

                {
                    GtkWidget *l = gtk_label_new ("");
                    gtk_label_set_markup (GTK_LABEL(l), _("Invert:"));
                    clonetiler_table_attach (table, l, 1.0, 2, 1);
                }
                {
                    GtkWidget *l = clonetiler_checkbox (tt, _("Invert the picked value"), "invert_picked");
                    clonetiler_table_attach (table, l, 0.0, 2, 2);
                }
            }

            {
                GtkWidget *frame = gtk_frame_new (_("3. Apply the value to the clones':"));
                gtk_box_pack_start (GTK_BOX (vvb), frame, FALSE, FALSE, 0);


                GtkWidget *table = gtk_table_new (2, 2, FALSE);
                gtk_table_set_row_spacings (GTK_TABLE (table), 4);
                gtk_table_set_col_spacings (GTK_TABLE (table), 6);
                gtk_container_add(GTK_CONTAINER(frame), table);

                {
                    GtkWidget *b  = gtk_check_button_new_with_label (_("Presence"));
                    gint old = prefs_get_int_attribute (prefs_path, "pick_to_presence", 1);
                    gtk_toggle_button_set_active ((GtkToggleButton *) b, old != 0);
                    gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), b, _("Each clone is created with the probability determined by the picked value in that point"), NULL);
                    clonetiler_table_attach (table, b, 0.0, 1, 1);
                    gtk_signal_connect(GTK_OBJECT(b), "toggled",
                                       GTK_SIGNAL_FUNC(clonetiler_pick_to), (gpointer) "pick_to_presence");
                }

                {
                    GtkWidget *b  = gtk_check_button_new_with_label (_("Size"));
                    gint old = prefs_get_int_attribute (prefs_path, "pick_to_size", 0);
                    gtk_toggle_button_set_active ((GtkToggleButton *) b, old != 0);
                    gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), b, _("Each clone's size is determined by the picked value in that point"), NULL);
                    clonetiler_table_attach (table, b, 0.0, 2, 1);
                    gtk_signal_connect(GTK_OBJECT(b), "toggled",
                                       GTK_SIGNAL_FUNC(clonetiler_pick_to), (gpointer) "pick_to_size");
                }

                {
                    GtkWidget *b  = gtk_check_button_new_with_label (_("Color"));
                    gint old = prefs_get_int_attribute (prefs_path, "pick_to_color", 0);
                    gtk_toggle_button_set_active ((GtkToggleButton *) b, old != 0);
                    gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), b, _("Each clone is painted by the picked color (the original must have unset fill or stroke)"), NULL);
                    clonetiler_table_attach (table, b, 0.0, 1, 2);
                    gtk_signal_connect(GTK_OBJECT(b), "toggled",
                                       GTK_SIGNAL_FUNC(clonetiler_pick_to), (gpointer) "pick_to_color");
                }

                {
                    GtkWidget *b  = gtk_check_button_new_with_label (_("Opacity"));
                    gint old = prefs_get_int_attribute (prefs_path, "pick_to_opacity", 0);
                    gtk_toggle_button_set_active ((GtkToggleButton *) b, old != 0);
                    gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), b, _("Each clone's opacity is determined by the picked value in that point"), NULL);
                    clonetiler_table_attach (table, b, 0.0, 2, 2);
                    gtk_signal_connect(GTK_OBJECT(b), "toggled",
                                       GTK_SIGNAL_FUNC(clonetiler_pick_to), (gpointer) "pick_to_opacity");
                }
            }
           gtk_widget_set_sensitive (vvb, prefs_get_int_attribute (prefs_path, "dotrace", 0));
        }
        }

// Rows/columns, width/height
        {
            GtkWidget *table = gtk_table_new (2, 2, FALSE);
            gtk_container_set_border_width (GTK_CONTAINER (table), VB_MARGIN);
            gtk_table_set_row_spacings (GTK_TABLE (table), 4);
            gtk_table_set_col_spacings (GTK_TABLE (table), 6);
            gtk_box_pack_start (GTK_BOX (mainbox), table, FALSE, FALSE, 0);

            {
                GtkWidget *hb = gtk_hbox_new(FALSE, VB_MARGIN);
                g_object_set_data (G_OBJECT(dlg), "rowscols", (gpointer) hb);

                {
                    GtkObject *a = gtk_adjustment_new(0.0, 1, 500, 1, 10, 10);
                    int value = prefs_get_int_attribute (prefs_path, "jmax", 2);
                    gtk_adjustment_set_value (GTK_ADJUSTMENT (a), value);
                    GtkWidget *sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 1.0, 0);
                    gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), sb, _("How many rows in the tiling"), NULL);
                    gtk_entry_set_width_chars (GTK_ENTRY (sb), 5);
                    gtk_box_pack_start (GTK_BOX (hb), sb, TRUE, TRUE, 0);

                    gtk_signal_connect(GTK_OBJECT(a), "value_changed",
                                       GTK_SIGNAL_FUNC(clonetiler_xy_changed), (gpointer) "jmax");
                }

                {
                    GtkWidget *l = gtk_label_new ("");
                    gtk_label_set_markup (GTK_LABEL(l), "&#215;");
                    gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
                    gtk_box_pack_start (GTK_BOX (hb), l, TRUE, TRUE, 0);
                }

                {
                    GtkObject *a = gtk_adjustment_new(0.0, 1, 500, 1, 10, 10);
                    int value = prefs_get_int_attribute (prefs_path, "imax", 2);
                    gtk_adjustment_set_value (GTK_ADJUSTMENT (a), value);
                    GtkWidget *sb = gtk_spin_button_new (GTK_ADJUSTMENT (a), 1.0, 0);
                    gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), sb, _("How many columns in the tiling"), NULL);
                    gtk_entry_set_width_chars (GTK_ENTRY (sb), 5);
                    gtk_box_pack_start (GTK_BOX (hb), sb, TRUE, TRUE, 0);

                    gtk_signal_connect(GTK_OBJECT(a), "value_changed",
                                       GTK_SIGNAL_FUNC(clonetiler_xy_changed), (gpointer) "imax");
                }

                clonetiler_table_attach (table, hb, 0.0, 1, 2);
            }

            {
                GtkWidget *hb = gtk_hbox_new(FALSE, VB_MARGIN);
                g_object_set_data (G_OBJECT(dlg), "widthheight", (gpointer) hb);

                // unitmenu
                GtkWidget *u = sp_unit_selector_new (SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE);
                sp_unit_selector_set_unit (SP_UNIT_SELECTOR(u), sp_desktop_namedview(SP_ACTIVE_DESKTOP)->doc_units);

                {
                    // Width spinbutton
                    GtkObject *a = gtk_adjustment_new (0.0, -1e6, 1e6, 1.0, 10.0, 10.0);
                    sp_unit_selector_add_adjustment (SP_UNIT_SELECTOR (u), GTK_ADJUSTMENT (a));

                    double value = prefs_get_double_attribute (prefs_path, "fillwidth", 50);
                    SPUnit const &unit = *sp_unit_selector_get_unit(SP_UNIT_SELECTOR(u));
                    gdouble const units = sp_pixels_get_units (value, unit);
                    gtk_adjustment_set_value (GTK_ADJUSTMENT (a), units);

                    GtkWidget *e = gtk_spin_button_new (GTK_ADJUSTMENT (a), 1.0 , 2);
                    gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), e, _("Width of the rectangle to be filled"), NULL);
                    gtk_entry_set_width_chars (GTK_ENTRY (e), 5);
                    gtk_box_pack_start (GTK_BOX (hb), e, TRUE, TRUE, 0);
                    gtk_signal_connect(GTK_OBJECT(a), "value_changed",
                                       GTK_SIGNAL_FUNC(clonetiler_fill_width_changed), u);
                }
                {
                    GtkWidget *l = gtk_label_new ("");
                    gtk_label_set_markup (GTK_LABEL(l), "&#215;");
                    gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
                    gtk_box_pack_start (GTK_BOX (hb), l, TRUE, TRUE, 0);
                }

                {
                    // Height spinbutton
                    GtkObject *a = gtk_adjustment_new (0.0, -1e6, 1e6, 1.0, 10.0, 10.0);
                    sp_unit_selector_add_adjustment (SP_UNIT_SELECTOR (u), GTK_ADJUSTMENT (a));

                    double value = prefs_get_double_attribute (prefs_path, "fillheight", 50);
                    SPUnit const &unit = *sp_unit_selector_get_unit(SP_UNIT_SELECTOR(u));
                    gdouble const units = sp_pixels_get_units (value, unit);
                    gtk_adjustment_set_value (GTK_ADJUSTMENT (a), units);


                    GtkWidget *e = gtk_spin_button_new (GTK_ADJUSTMENT (a), 1.0 , 2);
                    gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), e, _("Height of the rectangle to be filled"), NULL);
                    gtk_entry_set_width_chars (GTK_ENTRY (e), 5);
                    gtk_box_pack_start (GTK_BOX (hb), e, TRUE, TRUE, 0);
                    gtk_signal_connect(GTK_OBJECT(a), "value_changed",
                                       GTK_SIGNAL_FUNC(clonetiler_fill_height_changed), u);
                }

                gtk_box_pack_start (GTK_BOX (hb), u, TRUE, TRUE, 0);
                clonetiler_table_attach (table, hb, 0.0, 2, 2);

            }

            // Switch
            GtkWidget* radio;
            {
                radio = gtk_radio_button_new_with_label (NULL, _("Rows, columns: "));
                gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), radio, _("Create the specified number of rows and columns"), NULL);
                clonetiler_table_attach (table, radio, 0.0, 1, 1);
                gtk_signal_connect (GTK_OBJECT (radio), "toggled", GTK_SIGNAL_FUNC (clonetiler_switch_to_create), (gpointer) dlg);
            }
            if (prefs_get_int_attribute(prefs_path, "fillrect", 0) == 0) {
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), TRUE);
                gtk_toggle_button_toggled (GTK_TOGGLE_BUTTON (radio));
            }
            {
                radio = gtk_radio_button_new_with_label (gtk_radio_button_group (GTK_RADIO_BUTTON (radio)), _("Width, height: "));
                gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), radio, _("Fill the specified width and height with the tiling"), NULL);
                clonetiler_table_attach (table, radio, 0.0, 2, 1);
                gtk_signal_connect (GTK_OBJECT (radio), "toggled", GTK_SIGNAL_FUNC (clonetiler_switch_to_fill), (gpointer) dlg);
            }
            if (prefs_get_int_attribute(prefs_path, "fillrect", 0) == 1) {
                gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio), TRUE);
                gtk_toggle_button_toggled (GTK_TOGGLE_BUTTON (radio));
            }
        }


// Use saved pos
        {
            GtkWidget *hb = gtk_hbox_new(FALSE, VB_MARGIN);
            gtk_box_pack_start (GTK_BOX (mainbox), hb, FALSE, FALSE, 0);

            GtkWidget *b  = gtk_check_button_new_with_label (_("Use saved size and position of the tile"));
            gint keepbbox = prefs_get_int_attribute (prefs_path, "keepbbox", 1);
            gtk_toggle_button_set_active ((GtkToggleButton *) b, keepbbox != 0);
            gtk_tooltips_set_tip (GTK_TOOLTIPS (tt), b, _("Pretend that the size and position of the tile are the same as the last time you tiled it (if any), instead of using the current size"), NULL);
            gtk_box_pack_start (GTK_BOX (hb), b, FALSE, FALSE, 0);

            gtk_signal_connect(GTK_OBJECT(b), "toggled",
                               GTK_SIGNAL_FUNC(clonetiler_keep_bbox_toggled), NULL);
        }

// Statusbar
        {
            GtkWidget *hb = gtk_hbox_new(FALSE, VB_MARGIN);
            gtk_box_pack_end (GTK_BOX (mainbox), hb, FALSE, FALSE, 0);
            GtkWidget *l = gtk_label_new("");
            g_object_set_data (G_OBJECT(dlg), "status", (gpointer) l);
            gtk_box_pack_start (GTK_BOX (hb), l, FALSE, FALSE, 0);
        }

// Buttons
        {
            GtkWidget *hb = gtk_hbox_new(FALSE, VB_MARGIN);
            gtk_box_pack_start (GTK_BOX (mainbox), hb, FALSE, FALSE, 0);

            {
                GtkWidget *b = gtk_button_new ();
                GtkWidget *l = gtk_label_new ("");
                gtk_label_set_markup_with_mnemonic (GTK_LABEL(l), _(" <b>_Create</b> "));
                gtk_container_add (GTK_CONTAINER(b), l);
                gtk_tooltips_set_tip (tt, b, _("Create and tile the clones of the selection"), NULL);
                gtk_signal_connect (GTK_OBJECT (b), "clicked", GTK_SIGNAL_FUNC (clonetiler_apply), NULL);
                gtk_box_pack_end (GTK_BOX (hb), b, FALSE, FALSE, 0);
            }

            { // buttons which are enabled only when there are tiled clones
                GtkWidget *sb = gtk_hbox_new(FALSE, 0);
                gtk_box_pack_end (GTK_BOX (hb), sb, FALSE, FALSE, 0);
                g_object_set_data (G_OBJECT(dlg), "buttons_on_tiles", (gpointer) sb);
                {
                    // TRANSLATORS: if a group of objects are "clumped" together, then they
                    //  are unevenly spread in the given amount of space - as shown in the
                    //  diagrams on the left in the following screenshot:
                    //  http://www.inkscape.org/screenshots/gallery/inkscape-0.42-CVS-tiles-unclump.png
                    //  So unclumping is the process of spreading a number of objects out more evenly.
                    GtkWidget *b = gtk_button_new_with_mnemonic (_(" _Unclump "));
                    gtk_tooltips_set_tip (tt, b, _("Spread out clones to reduce clumping; can be applied repeatedly"), NULL);
                    gtk_signal_connect (GTK_OBJECT (b), "clicked", GTK_SIGNAL_FUNC (clonetiler_unclump), NULL);
                    gtk_box_pack_end (GTK_BOX (sb), b, FALSE, FALSE, 0);
                }

                {
                    GtkWidget *b = gtk_button_new_with_mnemonic (_(" Re_move "));
                    gtk_tooltips_set_tip (tt, b, _("Remove existing tiled clones of the selected object (siblings only)"), NULL);
                    gtk_signal_connect (GTK_OBJECT (b), "clicked", GTK_SIGNAL_FUNC (clonetiler_remove), NULL);
                    gtk_box_pack_end (GTK_BOX (sb), b, FALSE, FALSE, 0);
                }

                // connect to global selection changed signal (so we can change desktops) and
                // external_change (so we're not fooled by undo)
                g_signal_connect (G_OBJECT (INKSCAPE), "change_selection", G_CALLBACK (clonetiler_change_selection), dlg);
                g_signal_connect (G_OBJECT (INKSCAPE), "external_change", G_CALLBACK (clonetiler_external_change), dlg);
                g_signal_connect(G_OBJECT(dlg), "destroy", G_CALLBACK(clonetiler_disconnect_gsignal), G_OBJECT (INKSCAPE));

                // update now
                clonetiler_change_selection (NULL, sp_desktop_selection(SP_ACTIVE_DESKTOP), dlg);
            }

            {
                GtkWidget *b = gtk_button_new_with_mnemonic (_(" R_eset "));
                // TRANSLATORS: "change" is a noun here
                gtk_tooltips_set_tip (tt, b, _("Reset all shifts, scales, rotates, opacity and color changes in the dialog to zero"), NULL);
                gtk_signal_connect (GTK_OBJECT (b), "clicked", GTK_SIGNAL_FUNC (clonetiler_reset), NULL);
                gtk_box_pack_start (GTK_BOX (hb), b, FALSE, FALSE, 0);
            }
        }

        gtk_widget_show_all (mainbox);

    } // end of if (!dlg)

    gtk_window_present ((GtkWindow *) dlg);
}


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
