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

#ifndef PATH_UTIL_H
#define PATH_UTIL_H

struct NArtBpath;

struct GnomeCanvasBpathDef {
	int ref_count;
	NArtBpath *bpath;
	int n_bpath;
	int n_bpath_max;
	int moveto_idx;
};


GnomeCanvasBpathDef *gnome_canvas_bpath_def_new (void);
GnomeCanvasBpathDef *gnome_canvas_bpath_def_new_from (NArtBpath *bpath);
GnomeCanvasBpathDef *gnome_canvas_bpath_def_ref (GnomeCanvasBpathDef *bpd);

#define gnome_canvas_bpath_def_unref gnome_canvas_bpath_def_free
void gnome_canvas_bpath_def_free       (GnomeCanvasBpathDef *bpd);

void gnome_canvas_bpath_def_moveto     (GnomeCanvasBpathDef *bpd,
					double x, double y);
void gnome_canvas_bpath_def_lineto     (GnomeCanvasBpathDef *bpd,
					double x, double y);
void gnome_canvas_bpath_def_curveto    (GnomeCanvasBpathDef *bpd,
					double x1, double y1,
					double x2, double y2,
					double x3, double y3);
void gnome_canvas_bpath_def_closepath  (GnomeCanvasBpathDef *bpd);

void gnome_canvas_bpath_def_art_finish (GnomeCanvasBpathDef *bpd);



#endif
