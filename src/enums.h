#ifndef __SP_ENUMS_H__
#define __SP_ENUMS_H__

/*
 * Main program enumerated types
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2003 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/* Anchor types (imported from Gtk) */
typedef enum
{
  SP_ANCHOR_CENTER,
  SP_ANCHOR_NORTH,
  SP_ANCHOR_NORTH_WEST,
  SP_ANCHOR_NORTH_EAST,
  SP_ANCHOR_SOUTH,
  SP_ANCHOR_SOUTH_WEST,
  SP_ANCHOR_SOUTH_EAST,
  SP_ANCHOR_WEST,
  SP_ANCHOR_EAST,
  SP_ANCHOR_N		= SP_ANCHOR_NORTH,
  SP_ANCHOR_NW		= SP_ANCHOR_NORTH_WEST,
  SP_ANCHOR_NE		= SP_ANCHOR_NORTH_EAST,
  SP_ANCHOR_S		= SP_ANCHOR_SOUTH,
  SP_ANCHOR_SW		= SP_ANCHOR_SOUTH_WEST,
  SP_ANCHOR_SE		= SP_ANCHOR_SOUTH_EAST,
  SP_ANCHOR_W		= SP_ANCHOR_WEST,
  SP_ANCHOR_E		= SP_ANCHOR_EAST
} SPAnchorType;

/* preserveAspectRatio */

enum {
	SP_ASPECT_NONE,
	SP_ASPECT_XMIN_YMIN,
	SP_ASPECT_XMID_YMIN,
	SP_ASPECT_XMAX_YMIN,
	SP_ASPECT_XMIN_YMID,
	SP_ASPECT_XMID_YMID,
	SP_ASPECT_XMAX_YMID,
	SP_ASPECT_XMIN_YMAX,
	SP_ASPECT_XMID_YMAX,
	SP_ASPECT_XMAX_YMAX
};

enum {
	SP_ASPECT_MEET,
	SP_ASPECT_SLICE
};

/* maskUnits */
/* maskContentUnits */

enum {
	SP_CONTENT_UNITS_USERSPACEONUSE,
	SP_CONTENT_UNITS_OBJECTBOUNDINGBOX
};

/* markerUnits */

enum {
	SP_MARKER_UNITS_STROKEWIDTH,
	SP_MARKER_UNITS_USERSPACEONUSE
};

/* stroke-linejoin */
/* stroke-linecap */

/* markers */

enum {
  SP_MARKER_NONE,
  SP_MARKER_TRIANGLE,
  SP_MARKER_ARROW
};

/* fill-rule */
/* clip-rule */

enum {
	SP_CLONE_COMPENSATION_PARALLEL,
	SP_CLONE_COMPENSATION_UNMOVED,
	SP_CLONE_COMPENSATION_NONE
};

enum {
	SP_CLONE_ORPHANS_UNLINK,
	SP_CLONE_ORPHANS_DELETE,
	SP_CLONE_ORPHANS_ASKME
};

/* "inlayer" preference values */

enum PrefsSelectionContext {
    PREFS_SELECTION_ALL = 0,
    PREFS_SELECTION_LAYER = 1,
    PREFS_SELECTION_LAYER_RECURSIVE = 2,
};

/* clip/mask group enclosing behavior preference values */

enum PrefsMaskobjectGrouping {
    PREFS_MASKOBJECT_GROUPING_NONE = 0,
    PREFS_MASKOBJECT_GROUPING_SEPARATE = 1,
    PREFS_MASKOBJECT_GROUPING_ALL = 2,
};

#endif

