#ifndef __SP_UNIT_H__
#define __SP_UNIT_H__

/*
 * SPUnit
 *
 * Ported from libgnomeprint
 *
 * Authors:
 *   Dirk Luetjens <dirk@luedi.oche.de>
 *   Yves Arrouye <Yves.Arrouye@marin.fdn.fr>
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright 1999-2001 Ximian, Inc. and authors
 *
 */

#include <glib/gmessages.h>
#include <glib/gslist.h>
#include <glib/gtypes.h>
#include "sp-metric.h"


/*
 * Units and conversion methods used by libgnomeprint.
 *
 * You need those for certain config keys (like paper size), if you are
 * interested in using these (look at gnome-print-config.h for discussion,
 * why you may NOT be interested in paper size).
 *
 * Unit bases define set of mutually unrelated measuring systems (numbers,
 * paper, screen and dimesionless user coordinates). Still, you can convert
 * between those, specifying scaling factors explicitly.
 *
 * Paper (i.e. output) coordinates are taken as absolute real world units.
 * It has some justification, because screen unit (pixel) size changes,
 * if you change screen resolution, while you cannot change output on paper
 * as easily (unless you have thermally contracting paper, of course).
 *
 */

struct SPUnit;
struct SPDistance;

/*
 * The base linear ("absolute") unit is 1/72th of an inch, i.e. the base unit of postscript.
 */

/*
 * Unit bases
 */
enum SPUnitBase {
	SP_UNIT_DIMENSIONLESS = (1 << 0), /* For percentages and like */
	SP_UNIT_ABSOLUTE = (1 << 1), /* Real world distances - i.e. mm, cm... */
	SP_UNIT_DEVICE = (1 << 2), /* Pixels in the SVG/CSS sense. */
	SP_UNIT_VOLATILE = (1 << 3) /* em and ex */
};

/*
 * Units: indexes into sp_units.
 */
enum SPUnitId {
	SP_UNIT_SCALE,	// 1.0 == 100%
	SP_UNIT_PT,	// Postscript points: exactly 72 per inch
	SP_UNIT_PC,	// Pica; there are 12 points per pica
	SP_UNIT_PX,	// "Pixels" in the CSS sense; though Inkscape assumes a constant 90 per inch.
	SP_UNIT_PERCENT,  /* Note: In Inkscape this often means "relative to current value" (for
			     users to edit a value), rather than the SVG/CSS use of percentages. */
	SP_UNIT_MM,	// millimetres
	SP_UNIT_CM,	// centimetres
	SP_UNIT_M,	// metres
	SP_UNIT_IN,	// inches
	SP_UNIT_FT,	// foot
	SP_UNIT_EM,	// font-size of relevant font
	SP_UNIT_EX,	// x-height of relevant font
	sp_max_unit_id = SP_UNIT_EX	// For bounds-checking in sp_unit_get_by_id.
};

/*
 * Notice, that for correct menus etc. you have to use
 * ngettext method family yourself. For that reason we
 * do not provide translations in unit names.
 * I also do not know, whether to allow user-created units,
 * because this would certainly confuse textdomain.
 */

struct SPUnit {
	SPUnitId unit_id; /* used as sanity check */
	SPUnitBase base;
	gdouble unittobase; /* how many base units in this unit */
	SPMetric metric; // the corresponding SPMetric from sp-metrics.h
	guint svg_unit; // the corresponding SVGLengthUnit

	/* When using, you must call "gettext" on them so they're translated */
	gchar const *name;
	gchar const *abbr;
	gchar const *plural;
	gchar const *abbr_plural;
};

const SPUnit *sp_unit_get_by_abbreviation (const gchar *abbreviation);
/* When using, you must call "gettext" on them so they're translated */
const gchar *sp_unit_get_abbreviation (const SPUnit *unit);
gchar const *sp_unit_get_plural (SPUnit const *unit);

SPMetric sp_unit_get_metric(SPUnit const *unit);
guint sp_unit_get_svg_unit(SPUnit const *unit);

extern SPUnit const sp_units[];

inline SPUnit const &
sp_unit_get_by_id(SPUnitId const id)
{
	/* inline because the compiler should optimize away the g_return_val_if_fail test in the
	   usual case that the argument value is known at compile-time, leaving just
	   "return sp_units[constant]". */
	unsigned const ix = unsigned(id);
	g_return_val_if_fail(ix <= sp_max_unit_id, sp_units[SP_UNIT_PX]);
	return sp_units[ix];
}

#define SP_PS_UNIT (&sp_unit_get_by_id(SP_UNIT_PT))


/** Used solely by units-test.cpp. */
bool sp_units_table_sane();

#define SP_UNITS_ALL (SP_UNIT_DIMENSIONLESS | SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE | SP_UNIT_VOLATILE)

GSList *sp_unit_get_list (guint bases);
void sp_unit_free_list (GSList *units);

/* These are pure utility */
/* Return TRUE if conversion is possible, FALSE if unit bases differ */
gboolean sp_convert_distance (gdouble *distance, const SPUnit *from, const SPUnit *to);

/* If either one is NULL, transconverting to/from that base fails */
/* Generic conversion between volatile units would be useless anyways */
gdouble sp_convert_distance_full(gdouble const from_dist, SPUnit const &from, SPUnit const &to);

/* Some more convenience */
gdouble sp_units_get_pixels(gdouble const units, SPUnit const &unit);
gdouble sp_pixels_get_units(gdouble const pixels, SPUnit const &unit);

double angle_to_compass(double angle);
double angle_from_compass(double angle);

#endif 
