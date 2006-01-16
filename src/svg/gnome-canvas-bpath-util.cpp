/* GnomeCanvas Bezier polyline paths & segments
 *
 * GnomeCanvas is basically a port of the Tk toolkit's most excellent canvas widget.  Tk is
 * copyrighted by the Regents of the University of California, Sun Microsystems, and other parties.
 *
 * Copyright (C) 1998,1999 The Free Software Foundation
 *
 * Authors: Federico Mena <federico@nuclecu.unam.mx>
 *          Lauris Kaplinski <lauris@ariman.ee>
 *          Raph Levien <raph@acm.org>
 */

#include <glib/gmem.h>
#include <glib/gmessages.h>

#include "libnr/n-art-bpath.h"
#include "gnome-canvas-bpath-util.h"

GnomeCanvasBpathDef *
gnome_canvas_bpath_def_new (void)
{
	GnomeCanvasBpathDef *bpd;

	bpd = g_new (GnomeCanvasBpathDef, 1);
	bpd->n_bpath = 0;
	bpd->n_bpath_max = 16;
	bpd->moveto_idx = -1;
	bpd->bpath = g_new (NArtBpath, bpd->n_bpath_max);
	bpd->ref_count = 1;

	return bpd;
}

GnomeCanvasBpathDef *
gnome_canvas_bpath_def_new_from (NArtBpath *path)
{
	GnomeCanvasBpathDef *bpd;
	int i;

	g_return_val_if_fail (path != NULL, NULL);

	bpd = g_new (GnomeCanvasBpathDef, 1);
	
	for (i = 0; path [i].code != NR_END; i++)
		;
	bpd->n_bpath = i;
	bpd->n_bpath_max = i;
	bpd->moveto_idx = -1;
	bpd->ref_count = 1;
	bpd->bpath = g_new (NArtBpath, i);

	memcpy (bpd->bpath, path, i * sizeof (NArtBpath));
	return bpd;
}

GnomeCanvasBpathDef *
gnome_canvas_bpath_def_ref (GnomeCanvasBpathDef *bpd)
{
	g_return_val_if_fail (bpd != NULL, NULL);

	bpd->ref_count += 1;
	return bpd;
}

void
gnome_canvas_bpath_def_free (GnomeCanvasBpathDef *bpd)
{
	g_return_if_fail (bpd != NULL);

	bpd->ref_count -= 1;
	if (bpd->ref_count == 0) {
		g_free (bpd->bpath);
		g_free (bpd);
	}
}

void
gnome_canvas_bpath_def_moveto (GnomeCanvasBpathDef *bpd, double x, double y)
{
	NArtBpath *bpath;
	int n_bpath;

	g_return_if_fail (bpd != NULL);

	n_bpath = bpd->n_bpath++;

	if (n_bpath == bpd->n_bpath_max)
		bpd->bpath = (NArtBpath*)g_realloc (bpd->bpath,
					(bpd->n_bpath_max <<= 1) * sizeof (NArtBpath));
	bpath = bpd->bpath;
	bpath[n_bpath].code = NR_MOVETO_OPEN;
	bpath[n_bpath].x3 = x;
	bpath[n_bpath].y3 = y;
	bpd->moveto_idx = n_bpath;
}

void
gnome_canvas_bpath_def_lineto (GnomeCanvasBpathDef *bpd, double x, double y)
{
	NArtBpath *bpath;
	int n_bpath;

	g_return_if_fail (bpd != NULL);
	g_return_if_fail (bpd->moveto_idx >= 0);

	n_bpath = bpd->n_bpath++;

	if (n_bpath == bpd->n_bpath_max)
		bpd->bpath = (NArtBpath*)g_realloc (bpd->bpath,
					(bpd->n_bpath_max <<= 1) * sizeof (NArtBpath));
	bpath = bpd->bpath;
	bpath[n_bpath].code = NR_LINETO;
	bpath[n_bpath].x3 = x;
	bpath[n_bpath].y3 = y;
}

void
gnome_canvas_bpath_def_curveto (GnomeCanvasBpathDef *bpd, double x1, double y1, double x2, double y2, double x3, double y3)
{
	NArtBpath *bpath;
	int n_bpath;

	g_return_if_fail (bpd != NULL);
	g_return_if_fail (bpd->moveto_idx >= 0);

	n_bpath = bpd->n_bpath++;

	if (n_bpath == bpd->n_bpath_max)
		bpd->bpath = (NArtBpath*)g_realloc (bpd->bpath,
					(bpd->n_bpath_max <<= 1) * sizeof (NArtBpath));
	bpath = bpd->bpath;
	bpath[n_bpath].code = NR_CURVETO;
	bpath[n_bpath].x1 = x1;
	bpath[n_bpath].y1 = y1;
	bpath[n_bpath].x2 = x2;
	bpath[n_bpath].y2 = y2;
	bpath[n_bpath].x3 = x3;
	bpath[n_bpath].y3 = y3;
}

void
gnome_canvas_bpath_def_closepath (GnomeCanvasBpathDef *bpd)
{
	NArtBpath *bpath;
	int n_bpath;

	g_return_if_fail (bpd != NULL);
	g_return_if_fail (bpd->moveto_idx >= 0);
	g_return_if_fail (bpd->n_bpath > 0);
	
	bpath = bpd->bpath;
	n_bpath = bpd->n_bpath;

	/* Add closing vector if we need it. */
	if (bpath[n_bpath - 1].x3 != bpath[bpd->moveto_idx].x3 ||
	    bpath[n_bpath - 1].y3 != bpath[bpd->moveto_idx].y3) {
		gnome_canvas_bpath_def_lineto (bpd, bpath[bpd->moveto_idx].x3,
					       bpath[bpd->moveto_idx].y3);
		bpath = bpd->bpath;
	}
	bpath[bpd->moveto_idx].code = NR_MOVETO;
	bpd->moveto_idx = -1;
}

void
gnome_canvas_bpath_def_art_finish (GnomeCanvasBpathDef *bpd)
{
	int n_bpath;

	g_return_if_fail (bpd != NULL);
	
	n_bpath = bpd->n_bpath++;

	if (n_bpath == bpd->n_bpath_max)
		bpd->bpath = (NArtBpath*)g_realloc (bpd->bpath,
					(bpd->n_bpath_max <<= 1) * sizeof (NArtBpath));
	bpd->bpath [n_bpath].code = NR_END;
}

