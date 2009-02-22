#ifndef __NR_ARENA_GLYPHS_H__
#define __NR_ARENA_GLYPHS_H__

/*
 * RGBA display list system for inkscape
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL
 *
 */

#define NR_TYPE_ARENA_GLYPHS (nr_arena_glyphs_get_type ())
#define NR_ARENA_GLYPHS(obj) (NR_CHECK_INSTANCE_CAST ((obj), NR_TYPE_ARENA_GLYPHS, NRArenaGlyphs))
#define NR_IS_ARENA_GLYPHS(obj) (NR_CHECK_INSTANCE_TYPE ((obj), NR_TYPE_ARENA_GLYPHS))

#include <libnrtype/nrtype-forward.h>

#include <display/display-forward.h>
#include <forward.h>
#include <sp-paint-server.h>
#include <display/nr-arena-item.h>

#define test_glyph_liv

class Shape;

NRType nr_arena_glyphs_get_type (void);

struct NRArenaGlyphs : public NRArenaItem {
	/* Glyphs data */
	SPStyle *style;
	Geom::Matrix g_transform;
	font_instance *font;
	gint glyph;

	raster_font *rfont;
	raster_font *sfont;
	float x, y;

//	Geom::Matrix cached_tr;
//	Shape  *cached_shp;
//	bool   cached_shp_dirty;
//	bool   cached_style_dirty;
	
//	Shape  *stroke_shp;

	static NRArenaGlyphs *create(NRArena *arena) {
		NRArenaGlyphs *obj=reinterpret_cast<NRArenaGlyphs *>(nr_object_new(NR_TYPE_ARENA_GLYPHS));
		obj->init(arena);
		return obj;
	}
};

struct NRArenaGlyphsClass {
	NRArenaItemClass parent_class;
};

void nr_arena_glyphs_set_path ( NRArenaGlyphs *glyphs,
                                SPCurve *curve, unsigned int lieutenant,
                                font_instance *font, int glyph,
                                Geom::Matrix const *transform   );
void nr_arena_glyphs_set_style (NRArenaGlyphs *glyphs, SPStyle *style);

/* Integrated group of component glyphss */

typedef struct NRArenaGlyphsGroup NRArenaGlyphsGroup;
typedef struct NRArenaGlyphsGroupClass NRArenaGlyphsGroupClass;

#include "nr-arena-group.h"

#define NR_TYPE_ARENA_GLYPHS_GROUP (nr_arena_glyphs_group_get_type ())
#define NR_ARENA_GLYPHS_GROUP(obj) (NR_CHECK_INSTANCE_CAST ((obj), NR_TYPE_ARENA_GLYPHS_GROUP, NRArenaGlyphsGroup))
#define NR_IS_ARENA_GLYPHS_GROUP(obj) (NR_CHECK_INSTANCE_TYPE ((obj), NR_TYPE_ARENA_GLYPHS_GROUP))

NRType nr_arena_glyphs_group_get_type (void);

struct NRArenaGlyphsGroup : public NRArenaGroup {
  //SPStyle *style;
	NRRect paintbox;
	/* State data */
	SPPainter *fill_painter;
	SPPainter *stroke_painter;

	static NRArenaGlyphsGroup *create(NRArena *arena) {
		NRArenaGlyphsGroup *obj=reinterpret_cast<NRArenaGlyphsGroup *>(nr_object_new(NR_TYPE_ARENA_GLYPHS_GROUP));
		obj->init(arena);
		return obj;
	}
};

struct NRArenaGlyphsGroupClass {
	NRArenaGroupClass parent_class;
};

/* Utility functions */

void nr_arena_glyphs_group_clear (NRArenaGlyphsGroup *group);

void nr_arena_glyphs_group_add_component (NRArenaGlyphsGroup *group, font_instance *font, int glyph, Geom::Matrix const &transform);

void nr_arena_glyphs_group_set_style (NRArenaGlyphsGroup *group, SPStyle *style);

void nr_arena_glyphs_group_set_paintbox (NRArenaGlyphsGroup *group, const NRRect *pbox);

#endif
