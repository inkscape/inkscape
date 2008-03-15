#define __SP_DASH_SELECTOR_C__

/*
 * Optionmenu for selecting dash patterns
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
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

#include "../style.h"
#include "../dialogs/dialog-events.h"

#include "dash-selector.h"

enum {CHANGED, LAST_SIGNAL};

struct SPDashSelector {
	GtkHBox hbox;

	GtkWidget *dash;
	GtkObject *offset;
};

struct SPDashSelectorClass {
	GtkHBoxClass parent_class;

	void (* changed) (SPDashSelector *dsel);
};

double dash_0[] = {-1.0};
double dash_1_1[] = {1.0, 1.0, -1.0};
double dash_2_1[] = {2.0, 1.0, -1.0};
double dash_4_1[] = {4.0, 1.0, -1.0};
double dash_1_2[] = {1.0, 2.0, -1.0};
double dash_1_4[] = {1.0, 4.0, -1.0};

double *builtin_dashes[] = {dash_0, dash_1_1, dash_2_1, dash_4_1, dash_1_2, dash_1_4, NULL};

static double **dashes = NULL;

static void sp_dash_selector_class_init (SPDashSelectorClass *klass);
static void sp_dash_selector_init (SPDashSelector *dsel);
static GtkWidget *sp_dash_selector_menu_item_new (SPDashSelector *dsel, double *pattern);
static void sp_dash_selector_menu_item_image_realize (GtkWidget *mi, double *pattern);
static void sp_dash_selector_dash_activate (GtkObject *object, SPDashSelector *dsel);
static void sp_dash_selector_offset_value_changed (GtkAdjustment *adj, SPDashSelector *dsel);

static GtkHBoxClass *parent_class;
static guint signals[LAST_SIGNAL] = {0};

GtkType
sp_dash_selector_get_type (void)
{
	static GtkType type = 0;
	if (!type) {
		GtkTypeInfo info = {
			"SPDashSelector",
			sizeof (SPDashSelector),
			sizeof (SPDashSelectorClass),
			(GtkClassInitFunc) sp_dash_selector_class_init,
			(GtkObjectInitFunc) sp_dash_selector_init,
			NULL, NULL, NULL
		};
		type = gtk_type_unique (GTK_TYPE_HBOX, &info);
	}
	return type;
}

static void
sp_dash_selector_class_init (SPDashSelectorClass *klass)
{
	parent_class = (GtkHBoxClass*)gtk_type_class (GTK_TYPE_HBOX);

	signals[CHANGED] = gtk_signal_new ("changed",
					   (GtkSignalRunType)(GTK_RUN_FIRST | GTK_RUN_NO_RECURSE),
					   G_TYPE_FROM_CLASS (klass),
					   GTK_SIGNAL_OFFSET (SPDashSelectorClass, changed),
					   gtk_marshal_NONE__NONE,
					   GTK_TYPE_NONE, 0);
}

static void
sp_dash_selector_init (SPDashSelector *dsel)
{
	GtkTooltips *tt = gtk_tooltips_new();

	dsel->dash = gtk_option_menu_new ();
	gtk_tooltips_set_tip (tt, dsel->dash, _("Dash pattern"), NULL);
	gtk_widget_show (dsel->dash);
	gtk_box_pack_start (GTK_BOX (dsel), dsel->dash, FALSE, FALSE, 0);

	GtkWidget *m = gtk_menu_new ();
	gtk_widget_show (m);
	for (int i = 0; dashes[i]; i++) {
		GtkWidget *mi = sp_dash_selector_menu_item_new (dsel, dashes[i]);
		gtk_widget_show (mi);
		gtk_menu_append (GTK_MENU (m), mi);
	}
	gtk_option_menu_set_menu (GTK_OPTION_MENU (dsel->dash), m);

	dsel->offset = gtk_adjustment_new (0.0, 0.0, 10.0, 0.1, 1.0, 1.0);
	GtkWidget *sb = gtk_spin_button_new (GTK_ADJUSTMENT (dsel->offset), 0.1, 2);
	gtk_tooltips_set_tip (tt, sb, _("Pattern offset"), NULL);

	sp_dialog_defocus_on_enter (sb);
	gtk_widget_show (sb);
	gtk_box_pack_start (GTK_BOX (dsel), sb, FALSE, FALSE, 0);
	gtk_signal_connect (dsel->offset, "value_changed", GTK_SIGNAL_FUNC (sp_dash_selector_offset_value_changed), dsel);

	gtk_object_set_data (GTK_OBJECT (dsel), "pattern", dashes[0]);
}

GtkWidget *
sp_dash_selector_new (Inkscape::XML::Node *drepr)
{
	if (!dashes) {
		int ndashes = 0;
		if (drepr) {
			for (Inkscape::XML::Node *dr = drepr->firstChild(); dr; dr = dr->next()) {
				if (!strcmp (dr->name(), "dash"))
					ndashes += 1;
			}
		}

		if (ndashes > 0) {
			int pos = 0;
			SPStyle *style = sp_style_new (NULL);
			dashes = g_new (double *, ndashes + 1);
			for (Inkscape::XML::Node *dr = drepr->firstChild(); dr; dr = dr->next()) {
				if (!strcmp (dr->name(), "dash")) {
					sp_style_read_from_repr (style, dr);
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
			}
			sp_style_unref (style);
			dashes[pos] = NULL;
		} else {
			dashes = builtin_dashes;
		}
	}

	GtkWidget *dsel = (GtkWidget*)gtk_type_new (SP_TYPE_DASH_SELECTOR);

	return dsel;
}

void
sp_dash_selector_set_dash (SPDashSelector *dsel, int ndash, double *dash, double offset)
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

	gtk_object_set_data (GTK_OBJECT (dsel), "pattern", dashes[pos]);
	gtk_option_menu_set_history (GTK_OPTION_MENU (dsel->dash), pos);
	gtk_adjustment_set_value (GTK_ADJUSTMENT (dsel->offset), offset);
}

void
sp_dash_selector_get_dash (SPDashSelector *dsel, int *ndash, double **dash, double *offset)
{
	double *pattern = (double*)gtk_object_get_data (GTK_OBJECT (dsel), "pattern");

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
		if (offset)
			*offset = GTK_ADJUSTMENT (dsel->offset)->value;
	} else {
		if (ndash)
			*ndash = 0;
		if (dash)
			*dash = NULL;
		if (offset)
			*offset = 0.0;
	}
}

bool
all_even_are_zero (double *pattern, int n)
{
	for (int i = 0; i < n; i += 2) {
		if (pattern[i] != 0)
			return false;
	}
	return true;
}

bool
all_odd_are_zero (double *pattern, int n)
{
	for (int i = 1; i < n; i += 2) {
		if (pattern[i] != 0)
			return false;
	}
	return true;
}

static GtkWidget *
sp_dash_selector_menu_item_new (SPDashSelector *dsel, double *pattern)
{
	GtkWidget *mi = gtk_menu_item_new ();
	GtkWidget *px = gtk_image_new_from_pixmap (NULL, NULL);

	gtk_widget_show (px);
	gtk_container_add (GTK_CONTAINER (mi), px);

	gtk_object_set_data (GTK_OBJECT (mi), "pattern", pattern);
	gtk_object_set_data (GTK_OBJECT (mi), "px", px);
	gtk_signal_connect (GTK_OBJECT (mi), "activate", G_CALLBACK (sp_dash_selector_dash_activate), dsel);

	g_signal_connect_after(G_OBJECT(px), "realize", G_CALLBACK(sp_dash_selector_menu_item_image_realize), pattern);

	return mi;
}

static void sp_dash_selector_menu_item_image_realize (GtkWidget *px, double *pattern) {
	GdkPixmap *pixmap = gdk_pixmap_new(px->window, DASH_PREVIEW_LENGTH + 4, 16, -1);
	GdkGC *gc = gdk_gc_new (pixmap);

	gdk_rgb_gc_set_foreground (gc, 0xffffffff);
	gdk_draw_rectangle (pixmap, gc, TRUE, 0, 0, DASH_PREVIEW_LENGTH + 4, 16);

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
				// dash is zero, skip this element in the array, and set pointer backwards so the next dash is added to the previous
				i_p -= 2;
			} else {
				// the first dash is zero; bad luck, gdk cannot start pattern with non-stroke, so we put a 1-pixel stub here
				// (it may turn out not shown, though, see special cases below)
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

	if (longest_dash > 1e-18 && longest_dash < 0.5) {
		// fake "shortening" of one-pixel marks by painting them lighter-than-black
		gint rgb = 255 - (gint) (255 * longest_dash / 0.5);
		gdk_rgb_gc_set_foreground (gc, SP_RGBA32_U_COMPOSE (rgb, rgb, rgb, rgb));
	} else {
		gdk_rgb_gc_set_foreground (gc, 0x00000000);
	}

	if (n_source_dashes > 0) {
		// special cases:
		if (all_even_are_zero (pattern, n_source_dashes)) {
			; // do not draw anything, only gaps are non-zero
		} else if (all_odd_are_zero (pattern, n_source_dashes)) {
			// draw solid line, only dashes are non-zero
			gdk_gc_set_line_attributes (gc, DASH_PREVIEW_WIDTH,
										GDK_LINE_SOLID, GDK_CAP_BUTT,
										GDK_JOIN_MITER);
			gdk_draw_line (pixmap, gc, 4, 8, DASH_PREVIEW_LENGTH, 8);
		} else {
			// regular pattern with both gaps and dashes non-zero
			gdk_gc_set_line_attributes (gc, DASH_PREVIEW_WIDTH,
										GDK_LINE_ON_OFF_DASH, GDK_CAP_BUTT,
										GDK_JOIN_MITER);
			gdk_gc_set_dashes (gc, 0, pixels_i, n_pixel_dashes);
			gdk_draw_line (pixmap, gc, 4, 8, DASH_PREVIEW_LENGTH, 8);
		}
	} else {
		// no pattern, draw solid line
		gdk_gc_set_line_attributes (gc, DASH_PREVIEW_WIDTH,
									GDK_LINE_SOLID, GDK_CAP_BUTT,
									GDK_JOIN_MITER);
		gdk_draw_line (pixmap, gc, 4, 8, DASH_PREVIEW_LENGTH, 8);
	}

	gdk_gc_unref (gc);

	gtk_image_set_from_pixmap(GTK_IMAGE(px), pixmap, NULL);
	gdk_pixmap_unref(pixmap);
}

static void
sp_dash_selector_dash_activate (GtkObject *object, SPDashSelector *dsel)
{
	double *pattern = (double*)gtk_object_get_data (object, "pattern");
	gtk_object_set_data (GTK_OBJECT (dsel), "pattern", pattern);

	gtk_signal_emit (GTK_OBJECT (dsel), signals[CHANGED]);
}

static void
sp_dash_selector_offset_value_changed (GtkAdjustment */*adj*/, SPDashSelector *dsel)
{
	gtk_signal_emit (GTK_OBJECT (dsel), signals[CHANGED]);
}
