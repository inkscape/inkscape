#define __SP_DASH_SELECTOR_NEW_C__

/** @file
 * @brief Option menu for selecting dash patterns - implementation
 */
/* Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define DASH_PREVIEW_WIDTH 2
#define DASH_PREVIEW_LENGTH 80

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cstring>
#include <string>
#include <libnr/nr-macros.h>
#include <gtk/gtk.h>
#include <glibmm/i18n.h>

#include "style.h"
#include "dialogs/dialog-events.h"
#include "preferences.h"

#include <gtkmm/optionmenu.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/spinbutton.h>


#include "dash-selector.h"

gchar const *const SPDashSelector::_prefs_path = "/palette/dashes";

static double dash_0[] = {-1.0};
static double dash_1_1[] = {1.0, 1.0, -1.0};
static double dash_2_1[] = {2.0, 1.0, -1.0};
static double dash_4_1[] = {4.0, 1.0, -1.0};
static double dash_1_2[] = {1.0, 2.0, -1.0};
static double dash_1_4[] = {1.0, 4.0, -1.0};

static double *builtin_dashes[] = {dash_0, dash_1_1, dash_2_1, dash_4_1, dash_1_2, dash_1_4, NULL};

static double **dashes = NULL;

static void sp_dash_selector_menu_item_image_realize(Gtk::Image *px, double *pattern);

SPDashSelector::SPDashSelector() {
    // TODO: find something more sensible here!!
    init_dashes();

    Gtk::Tooltips *tt = new Gtk::Tooltips();

    dash = new Gtk::OptionMenu();
    tt->set_tip(*dash, _("Dash pattern"));
    dash->show();
    this->pack_start(*dash, false, false, 0);

    Gtk::Menu *m = new Gtk::Menu();
    m->show();
    for (int i = 0; dashes[i]; i++) {
        Gtk::MenuItem *mi = menu_item_new(dashes[i]);
        mi->show();
        m->append(*mi);
    }
    dash->set_menu(*m);

    offset = new Gtk::Adjustment(0.0, 0.0, 10.0, 0.1, 1.0, 1.0);
    Gtk::SpinButton *sb = new Gtk::SpinButton(*offset, 0.1, 2);
    tt->set_tip(*sb, _("Pattern offset"));

    sp_dialog_defocus_on_enter_cpp(sb);
    sb->show();
    this->pack_start(*sb, false, false, 0);
    offset->signal_value_changed().connect(sigc::mem_fun(*this, &SPDashSelector::offset_value_changed));

    this->set_data("pattern", dashes[0]);
}

SPDashSelector::~SPDashSelector() {
    // FIXME: for some reason this doesn't get called; does the call to manage() in
    // sp_stroke_style_line_widget_new() not processed correctly?
    delete dash;
    delete offset;
}

void
SPDashSelector::init_dashes() {
    if (!dashes) {
        
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        std::vector<Glib::ustring> dash_prefs = prefs->getAllDirs(_prefs_path);
        
        if (!dash_prefs.empty()) {
            int pos = 0;
            SPStyle *style = sp_style_new (NULL);
            dashes = g_new (double *, dash_prefs.size() + 1);
            
            for (std::vector<Glib::ustring>::iterator i = dash_prefs.begin(); i != dash_prefs.end(); ++i) {
                sp_style_read_from_prefs(style, *i);
                
                if (style->stroke_dash.n_dash > 0) {
                    dashes[pos] = g_new (double, style->stroke_dash.n_dash + 1);
                    double *d = dashes[pos];
                    int i = 0;
                    for (; i < style->stroke_dash.n_dash; i++) {
                        d[i] = style->stroke_dash.dash[i];
                    }
                    d[i] = -1;
                } else {
                    dashes[pos] = dash_0;
                }
                pos += 1;
            }
        } else {
            dashes = builtin_dashes;
        }
    }
}

void
SPDashSelector::set_dash (int ndash, double *dash, double o)
{
    int pos = 0;
    if (ndash > 0) {
        double delta = 0.0;
        for (int i = 0; i < ndash; i++)
            delta += dash[i];
        delta /= 1000.0;

        for (int i = 0; dashes[i]; i++) {
            double *pattern = dashes[i];
            int np = 0;
            while (pattern[np] >= 0.0)
                np += 1;
            if (np == ndash) {
                int j;
                for (j = 0; j < ndash; j++) {
                    if (!NR_DF_TEST_CLOSE (dash[j], pattern[j], delta))
                        break;
                }
                if (j == ndash) {
                    pos = i;
                    break;
                }
            }
        }
    }

    this->set_data("pattern", dashes[pos]);
    this->dash->set_history(pos);
    this->offset->set_value(o);
}

void
SPDashSelector::get_dash(int *ndash, double **dash, double *off)
{
    double *pattern = (double*) this->get_data("pattern");

    int nd = 0;
    while (pattern[nd] >= 0.0)
        nd += 1;

    if (nd > 0) {
        if (ndash)
            *ndash = nd;
        if (dash) {
            *dash = g_new (double, nd);
            memcpy (*dash, pattern, nd * sizeof (double));
        }
        if (off)
            *off = offset->get_value();
    } else {
        if (ndash)
            *ndash = 0;
        if (dash)
            *dash = NULL;
        if (off)
            *off = 0.0;
    }
}

Gtk::MenuItem *
SPDashSelector::menu_item_new(double *pattern)
{
    Gtk::MenuItem *mi = new Gtk::MenuItem();
    Gtk::Image *px = new Gtk::Image();

    px->show();
    mi->add(*px);

    mi->set_data("pattern", pattern);
    mi->set_data("px", px);
    mi->signal_activate().connect(sigc::bind(sigc::mem_fun(*this, &SPDashSelector::dash_activate), mi));

    px->signal_realize().connect(sigc::bind(sigc::ptr_fun(&sp_dash_selector_menu_item_image_realize), px, pattern));

    return mi;
}

static bool
all_even_are_zero (double *pattern, int n)
{
	for (int i = 0; i < n; i += 2) {
		if (pattern[i] != 0)
			return false;
	}
	return true;
}

static bool
all_odd_are_zero (double *pattern, int n)
{
	for (int i = 1; i < n; i += 2) {
		if (pattern[i] != 0)
			return false;
	}
	return true;
}

static void sp_dash_selector_menu_item_image_realize(Gtk::Image *px, double *pattern) {
    Glib::RefPtr<Gdk::Pixmap> pixmap = Gdk::Pixmap::create(px->get_window(), DASH_PREVIEW_LENGTH + 4, 16, -1);
    Glib::RefPtr<Gdk::GC> gc = Gdk::GC::create(pixmap);

    gc->set_rgb_fg_color(Gdk::Color("#ffffff"));
    pixmap->draw_rectangle(gc, true, 0, 0, DASH_PREVIEW_LENGTH + 4, 16);

    // FIXME: all of the below twibblering is due to the limitations of gdk_gc_set_dashes (only integers, no zeroes).
    // Perhaps would make sense to rework this with manually drawn dashes.

    // Fill in the integer array of pixel-lengths, for display
    gint8 pixels_i[64];
    gdouble pixels_d[64];
    int n_source_dashes = 0;
    int n_pixel_dashes = 0;

    signed int i_s, i_p;
    for (i_s = 0, i_p = 0; pattern[i_s] >= 0.0; i_s ++, i_p ++) {
        pixels_d[i_p] = 0.0;
    }

    n_source_dashes = i_s;

    for (i_s = 0, i_p = 0; i_s < n_source_dashes; i_s ++, i_p ++) {
        // calculate the pixel length corresponding to the current dash
        gdouble pixels = DASH_PREVIEW_WIDTH * pattern[i_s];

        if (pixels > 0.0)
            pixels_d [i_p] += pixels;
        else {
            if (i_p >= 1) {
                // dash is zero, skip this element in the array, and set pointer backwards
                // so the next dash is added to the previous
                i_p -= 2;
            } else {
                // the first dash is zero; bad luck, gdk cannot start pattern with non-stroke, so we
                // put a 1-pixel stub here (it may turn out not shown, though, see special cases below)
                pixels_d [i_p] = 1.0;
            }
        }
    }

    n_pixel_dashes = i_p;

    gdouble longest_dash = 0.0;

    // after summation, convert double dash lengths to ints
    for (i_p = 0; i_p < n_pixel_dashes; i_p ++) {
        pixels_i [i_p] = (gint8) (pixels_d [i_p] + 0.5);
        // zero-length dashes are already eliminated, so the <1 dash is short but not zero;
        // we approximate it with a one-pixel mark
        if (pixels_i [i_p] < 1)
            pixels_i [i_p] = 1;
        if (i_p % 2 == 0) { // it's a dash
            if (pixels_d [i_p] > longest_dash)
                longest_dash = pixels_d [i_p];
        }
    }

    Gdk::Color color;
    if (longest_dash > 1e-18 && longest_dash < 0.5) {
        // fake "shortening" of one-pixel marks by painting them lighter-than-black
        gint rgb = 0xffff - (gint) (0xffff * longest_dash / 0.5);
        color.set_rgb(rgb, rgb, rgb);
        gc->set_rgb_fg_color(color);
    } else {
        color.set_rgb(0, 0, 0);
        gc->set_rgb_fg_color(color);
    }

    if (n_source_dashes > 0) {
        // special cases:
        if (all_even_are_zero (pattern, n_source_dashes)) {
            ; // do not draw anything, only gaps are non-zero
        } else if (all_odd_are_zero (pattern, n_source_dashes)) {
            // draw solid line, only dashes are non-zero
            gc->set_line_attributes(DASH_PREVIEW_WIDTH,
                                    Gdk::LINE_SOLID, Gdk::CAP_BUTT,
                                    Gdk::JOIN_MITER);
            pixmap->draw_line(gc, 4, 8, DASH_PREVIEW_LENGTH, 8);
        } else {
            // regular pattern with both gaps and dashes non-zero
            gc->set_line_attributes(DASH_PREVIEW_WIDTH, Gdk::LINE_ON_OFF_DASH, Gdk::CAP_BUTT, Gdk::JOIN_MITER);
            gc->set_dashes(0, pixels_i, n_pixel_dashes);
            pixmap->draw_line(gc, 4, 8, DASH_PREVIEW_LENGTH, 8);
        }
    } else {
        // no pattern, draw solid line
        gc->set_line_attributes(DASH_PREVIEW_WIDTH, Gdk::LINE_SOLID, Gdk::CAP_BUTT, Gdk::JOIN_MITER);
        pixmap->draw_line(gc, 4, 8, DASH_PREVIEW_LENGTH, 8);
    }

    Glib::RefPtr<Gdk::Bitmap> null_ptr;
    px->set(pixmap, null_ptr);
}

void
SPDashSelector::dash_activate (Gtk::MenuItem *mi)
{
    double *pattern = (double*) mi->get_data("pattern");
    this->set_data ("pattern", pattern);

    changed_signal.emit();
}


void
SPDashSelector::offset_value_changed()
{
    changed_signal.emit();
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
