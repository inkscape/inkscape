#define __SP_GNOME_C__

/*
 * Gnome stuff
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Ted Gould <ted@gould.cx>
 *
 *
 * Copyright (C) 2005 Authors
 *
 * Lauris: This code is in public domain
 * Ted: This code is under the GNU GPL
 */

/* Gnome Print */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>

#include "libnr/n-art-bpath.h"
#include "libnr/nr-rect.h"
#include "libnr/nr-matrix.h"
#include "libnr/nr-matrix-fns.h"
#include "libnr/nr-path.h"
#include "libnr/nr-pixblock.h"
#include "display/canvas-bpath.h"

#include <glib.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkbox.h>
#include <gtk/gtkstock.h>

#if OLDGnome
#include <libgnomeprint/gnome-print-master.h>
#include <libgnomeprintui/gnome-print-master-preview.h>
#endif

#include <glibmm/i18n.h>
#include "enums.h"
#include "document.h"
#include "style.h"
#include "sp-paint-server.h"

#include "gnome.h"

#include "extension/extension.h"
#include "extension/system.h"

static ArtBpath *nr_artpath_to_art_bpath(NArtBpath const *s);

namespace Inkscape {
namespace Extension {
namespace Internal {

PrintGNOME::PrintGNOME (void)
{
    /* Nothing here */
}

PrintGNOME::~PrintGNOME (void)
{
	return;
}

unsigned int
PrintGNOME::setup (Inkscape::Extension::Print *mod)
{
    GnomePrintConfig *config;
#if OLDGnome
    GtkWidget *dlg, *vbox, *sel;
#endif

    config = gnome_print_config_default ();
#if OLDGnome
    dlg = gtk_dialog_new_with_buttons (_("Select printer"), NULL,
				       GTK_DIALOG_MODAL,
				       GTK_STOCK_PRINT,
				       GTK_RESPONSE_OK,
				       GTK_STOCK_CANCEL,
				       GTK_RESPONSE_CANCEL,
				       NULL);

    vbox = GTK_DIALOG (dlg)->vbox;
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 4);

    sel = gnome_printer_selector_new (config);
    gtk_widget_show (sel);
    gtk_box_pack_start (GTK_BOX (vbox), sel, TRUE, TRUE, 0);

    btn = gtk_dialog_run (GTK_DIALOG (dlg));
    gtk_widget_destroy (dlg);
    if (btn != GTK_RESPONSE_OK) return FALSE;
#endif
    _gpc = gnome_print_context_new (config);
    gnome_print_config_unref (config);

    return TRUE;
}

unsigned int
PrintGNOME::set_preview (Inkscape::Extension::Print *mod)
{
#if OLDGnome
    SPPrintContext ctx;
    GnomePrintContext *gpc;
    GnomePrintMaster *gpm;
    GtkWidget *gpmp;
    gchar *title;


    gpm = gnome_print_master_new();
    _gpc = gnome_print_master_get_context (gpm);

    g_return_if_fail (gpm != NULL);
    g_return_if_fail (gpc != NULL);

    /* Print document */
    gnome_print_beginpage (gpc, SP_DOCUMENT_NAME (doc));
    gnome_print_translate (gpc, 0.0, sp_document_height (doc));
    /* From desktop points to document pixels */
    gnome_print_scale (gpc, 0.8, -0.8);
    sp_item_invoke_print (SP_ITEM (sp_document_root (doc)), &ctx);
    gnome_print_showpage (gpc);
    gnome_print_context_close (gpc);

    title = g_strdup_printf (_("Inkscape: Print Preview"));
    gpmp = gnome_print_master_preview_new (gpm, title);

    gtk_widget_show (GTK_WIDGET(gpmp));

    gnome_print_master_close (gpm);

    g_free (title);
#endif
    return 0;
}

unsigned int
PrintGNOME::begin (Inkscape::Extension::Print *mod, SPDocument *doc)
{
    gnome_print_beginpage (_gpc, (const guchar *)SP_DOCUMENT_NAME (doc));
    gnome_print_translate (_gpc, 0.0, sp_document_height (doc));
    /* From desktop points to document pixels */
    gnome_print_scale (_gpc, 0.8, -0.8);

    return 0;
}

unsigned int
PrintGNOME::finish (Inkscape::Extension::Print *mod)
{
    gnome_print_showpage (_gpc);
    gnome_print_context_close (_gpc);

    return 0;
}

unsigned int
PrintGNOME::bind (Inkscape::Extension::Print *mod, const NRMatrix *transform, float opacity)
{
    gdouble t[6];

	gnome_print_gsave(_gpc);

    t[0] = transform->c[0];
    t[1] = transform->c[1];
    t[2] = transform->c[2];
    t[3] = transform->c[3];
    t[4] = transform->c[4];
    t[5] = transform->c[5];

    gnome_print_concat (_gpc, t);

    /* fixme: Opacity? (lauris) */

    return 0;
}

unsigned int
PrintGNOME::release (Inkscape::Extension::Print *mod)
{
    gnome_print_grestore (_gpc);
    return 0;
}

unsigned int PrintGNOME::comment (Inkscape::Extension::Print * module,
		                  const char * comment)
{
	// ignore comment
	return 0;
}

unsigned int
PrintGNOME::fill(Inkscape::Extension::Print *mod,
		 NRBPath const *bpath, NRMatrix const *ctm, SPStyle const *style,
		 NRRect const *pbox, NRRect const *dbox, NRRect const *bbox)
{
    gdouble t[6];

    /* CTM is for information purposes only */
    /* We expect user coordinate system to be set up already */

    t[0] = ctm->c[0];
    t[1] = ctm->c[1];
    t[2] = ctm->c[2];
    t[3] = ctm->c[3];
    t[4] = ctm->c[4];
    t[5] = ctm->c[5];

    if (style->fill.type == SP_PAINT_TYPE_COLOR) {
	float rgb[3], opacity;
	sp_color_get_rgb_floatv (&style->fill.value.color, rgb);
	gnome_print_setrgbcolor (_gpc, rgb[0], rgb[1], rgb[2]);

	/* fixme: */
	opacity = SP_SCALE24_TO_FLOAT (style->fill_opacity.value) * SP_SCALE24_TO_FLOAT (style->opacity.value);
	gnome_print_setopacity (_gpc, opacity);

	ArtBpath * apath = nr_artpath_to_art_bpath(bpath->path);
	gnome_print_bpath (_gpc, apath, FALSE);
	g_free(apath);

	if (style->fill_rule.value == SP_WIND_RULE_EVENODD) {
	    gnome_print_eofill (_gpc);
	} else {
	    gnome_print_fill (_gpc);
	}
    } else if (style->fill.type == SP_PAINT_TYPE_PAINTSERVER) {
	SPPainter *painter;
	NRRect dpbox;

	/* fixme: */
	dpbox.x0 = pbox->x0;
	dpbox.y0 = pbox->y0;
	dpbox.x1 = pbox->x1;
	dpbox.y1 = pbox->y1;
	painter = sp_paint_server_painter_new(SP_STYLE_FILL_SERVER(style),
						// FIXME: the second matrix below must be the parent (context) transform.
						// I don't know what it must be for gnome-print. --bb
						ctm, NR::identity(),
					      &dpbox);
	if (painter) {
	    NRRect cbox;
	    NRRectL ibox;
	    NRMatrix d2i;
	    double dd2i[6];
	    int x, y;

	    nr_rect_d_intersect (&cbox, dbox, bbox);
	    ibox.x0 = (long) cbox.x0;
	    ibox.y0 = (long) cbox.y0;
	    ibox.x1 = (long) (cbox.x1 + 0.9999);
	    ibox.y1 = (long) (cbox.y1 + 0.9999);

	    nr_matrix_invert (&d2i, ctm);

	    gnome_print_gsave (_gpc);

		ArtBpath * apath = nr_artpath_to_art_bpath(bpath->path);
		gnome_print_bpath (_gpc, apath, FALSE);
		g_free(apath);

	    if (style->fill_rule.value == SP_WIND_RULE_EVENODD) {
		gnome_print_eoclip (_gpc);
	    } else {
		gnome_print_clip (_gpc);
	    }
	    dd2i[0] = d2i.c[0];
	    dd2i[1] = d2i.c[1];
	    dd2i[2] = d2i.c[2];
	    dd2i[3] = d2i.c[3];
	    dd2i[4] = d2i.c[4];
	    dd2i[5] = d2i.c[5];
	    gnome_print_concat (_gpc, dd2i);
	    /* Now we are in desktop coordinates */
	    for (y = ibox.y0; y < ibox.y1; y+= 64) {
		for (x = ibox.x0; x < ibox.x1; x+= 64) {
		    NRPixBlock pb;
		    nr_pixblock_setup_fast (&pb, NR_PIXBLOCK_MODE_R8G8B8A8N, x, y, x + 64, y + 64, TRUE);
		    painter->fill (painter, &pb);
		    gnome_print_gsave (_gpc);
		    gnome_print_translate (_gpc, x, y + 64);
		    gnome_print_scale (_gpc, 64, -64);
		    gnome_print_rgbaimage (_gpc, NR_PIXBLOCK_PX (&pb), 64, 64, pb.rs);
		    gnome_print_grestore (_gpc);
		    nr_pixblock_release (&pb);
		}
	    }
	    gnome_print_grestore (_gpc);
	    sp_painter_free (painter);
	}
    }

    return 0;
}

unsigned int
PrintGNOME::stroke (Inkscape::Extension::Print *mod, const NRBPath *bpath, const NRMatrix *ctm, const SPStyle *style,
			      const NRRect *pbox, const NRRect *dbox, const NRRect *bbox)
{
    gdouble t[6];

    /* CTM is for information purposes only */
    /* We expect user coordinate system to be set up already */

    t[0] = ctm->c[0];
    t[1] = ctm->c[1];
    t[2] = ctm->c[2];
    t[3] = ctm->c[3];
    t[4] = ctm->c[4];
    t[5] = ctm->c[5];

    if (style->stroke.type == SP_PAINT_TYPE_COLOR) {
	float rgb[3], opacity;
	sp_color_get_rgb_floatv (&style->stroke.value.color, rgb);
	gnome_print_setrgbcolor (_gpc, rgb[0], rgb[1], rgb[2]);

	/* fixme: */
	opacity = SP_SCALE24_TO_FLOAT (style->stroke_opacity.value) * SP_SCALE24_TO_FLOAT (style->opacity.value);
	gnome_print_setopacity (_gpc, opacity);

	if (style->stroke_dash.n_dash > 0) {
	    gnome_print_setdash (_gpc, style->stroke_dash.n_dash, style->stroke_dash.dash, style->stroke_dash.offset);
	} else {
	    gnome_print_setdash (_gpc, 0, NULL, 0.0);
	}

	gnome_print_setlinewidth (_gpc, style->stroke_width.computed);
	gnome_print_setlinejoin (_gpc, style->stroke_linejoin.computed);
	gnome_print_setlinecap (_gpc, style->stroke_linecap.computed);

	ArtBpath * apath = nr_artpath_to_art_bpath(bpath->path);
	gnome_print_bpath (_gpc, apath, FALSE);
	g_free(apath);

	gnome_print_stroke (_gpc);
    }

    return 0;
}

unsigned int
PrintGNOME::image (Inkscape::Extension::Print *mod, unsigned char *px, unsigned int w, unsigned int h, unsigned int rs,
			     const NRMatrix *transform, const SPStyle *style)
{
    gdouble t[6];

    t[0] = transform->c[0];
    t[1] = transform->c[1];
    t[2] = transform->c[2];
    t[3] = transform->c[3];
    t[4] = transform->c[4];
    t[5] = transform->c[5];

    gnome_print_gsave (_gpc);

    gnome_print_concat (_gpc, t);

    if (style->opacity.value != SP_SCALE24_MAX) {
	guchar *dpx, *d, *s;
	guint x, y;
	guint32 alpha;
	alpha = (guint32) floor (SP_SCALE24_TO_FLOAT (style->opacity.value) * 255.9999);
	dpx = g_new (guchar, w * h * 4);
	for (y = 0; y < h; y++) {
	    s = px + y * rs;
	    d = dpx + y * w * 4;
	    memcpy (d, s, w * 4);
	    for (x = 0; x < w; x++) {
		d[3] = (s[3] * alpha) / 255;
		s += 4;
		d += 4;
	    }
	}
	gnome_print_rgbaimage (_gpc, dpx, w, h, w * 4);
	g_free (dpx);
    } else {
	gnome_print_rgbaimage (_gpc, px, w, h, rs);
    }

    gnome_print_grestore (_gpc);

    return 0;
}

#include "clear-n_.h"

void
PrintGNOME::init (void)
{
	Inkscape::Extension::Extension * ext;

	/* SVG in */
    ext = Inkscape::Extension::build_from_mem(
		"<inkscape-extension>\n"
			"<name>" N_("GNOME Print") "</name>\n"
			"<id>" SP_MODULE_KEY_PRINT_GNOME "</id>\n"
			"<print/>\n"
		"</inkscape-extension>", new PrintGNOME());

	return;
}

}  /* namespace Internal */
}  /* namespace Extension */
}  /* namespace Inkscape */


// Remember to free the result!
static ArtBpath *
nr_artpath_to_art_bpath(NArtBpath const *s)
{
	int i;
	if (!s) {
		return NULL;
	}

	i = 0;
	while (s[i].code != NR_END) i += 1;

	ArtBpath* d = g_new (ArtBpath, i + 1);

	i = 0;
	while (s[i].code != NR_END) {
		d[i].code = (ArtPathcode)s[i].code;
		if (s[i].code == NR_CURVETO) {
			d[i].x1 = s[i].x1;
			d[i].y1 = s[i].y1;
			d[i].x2 = s[i].x2;
			d[i].y2 = s[i].y2;
		}
		d[i].x3 = s[i].x3;
		d[i].y3 = s[i].y3;
		i += 1;
	}
	d[i].code = ART_END;

	return d;
}

