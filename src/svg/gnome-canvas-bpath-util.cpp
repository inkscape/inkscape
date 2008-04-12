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
 *          Jasper van de Gronde <th.v.d.gronde@hccnet.nl>
 */

#include <cstring>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <glib/gmem.h>
#include <glib/gmessages.h>
#include <algorithm>

#include "libnr/n-art-bpath.h"
#include "libnr/nr-point-ops.h"
#include "gnome-canvas-bpath-util.h"
#include "prefs-utils.h"

static inline NR::Point distTo(GnomeCanvasBpathDef *bpd, size_t idx1, size_t idx2, unsigned int coord1=3, unsigned int coord2=3) {
    NR::Point diff(bpd->bpath[idx1].c(coord1) - bpd->bpath[idx2].c(coord2));
    return NR::Point(std::abs(diff[NR::X]), std::abs(diff[NR::Y]));
}

static bool isApproximatelyClosed(GnomeCanvasBpathDef *bpd) {
    int const np = prefs_get_int_attribute("options.svgoutput", "numericprecision", 8);
    double const precision = pow(10.0, -np); // This roughly corresponds to a difference below the last significant digit
	int const initial = bpd->moveto_idx;
	int const current = bpd->n_bpath - 1;
	int const previous = bpd->n_bpath - 2;
    NR::Point distToPrev(distTo(bpd, current, previous));
    NR::Point distToInit(distTo(bpd, current, initial));
    // NOTE: It would be better to determine the uncertainty while parsing, in rsvg_parse_path_data, but this seems to perform reasonably well
    return
        distToInit[NR::X] <= distToPrev[NR::X]*precision &&
        distToInit[NR::Y] <= distToPrev[NR::Y]*precision;
}

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
    if (!isApproximatelyClosed(bpd)) {
		gnome_canvas_bpath_def_lineto (bpd, bpath[bpd->moveto_idx].x3,
					       bpath[bpd->moveto_idx].y3);
		bpath = bpd->bpath;
    } else {
        // If it is approximately closed we close it here to prevent internal logic to fail.
        // In addition it is probably better to continue working with this end point, as it
        // is probably more precise than the original.
        // NOTE: At the very least sp_bpath_check_subpath will fail, but it is not unreasonable
        // to assume that there might be more places where similar problems would occur.
        bpath[n_bpath-1].x3 = bpath[bpd->moveto_idx].x3;
        bpath[n_bpath-1].y3 = bpath[bpd->moveto_idx].y3;
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

