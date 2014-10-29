#ifndef SEEN_SP_STAR_H
#define SEEN_SP_STAR_H

/*
 * <sodipodi:star> implementation
 *
 * Authors:
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-polygon.h"


#define SP_STAR(obj) (dynamic_cast<SPStar*>((SPObject*)obj))
#define SP_IS_STAR(obj) (dynamic_cast<const SPStar*>((SPObject*)obj) != NULL)

typedef enum {
	SP_STAR_POINT_KNOT1,
	SP_STAR_POINT_KNOT2
} SPStarPoint;

class SPStar : public SPPolygon {
public:
	SPStar();
	virtual ~SPStar();

	int sides;

	Geom::Point center;
	double r[2];
	double arg[2];
	bool flatsided;

	double rounded;
	double randomized;

// CPPIFY: This derivation is a bit weird.
// parent_class = reinterpret_cast<SPShapeClass *>(g_type_class_ref(SP_TYPE_SHAPE));
// So shouldn't star be derived from shape instead of polygon?
// What does polygon have that shape doesn't?

	virtual void build(SPDocument *document, Inkscape::XML::Node *repr);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, unsigned int flags);
	virtual void set(unsigned int key, char const* value);
	virtual void update(SPCtx* ctx, unsigned int flags);

    virtual const char* displayName() const;
	virtual char* description() const;
	virtual void snappoints(std::vector<Inkscape::SnapCandidatePoint> &p, Inkscape::SnapPreferences const *snapprefs) const;

	virtual void update_patheffect(bool write);
	virtual void set_shape();
	virtual Geom::Affine set_transform(Geom::Affine const& xform);
};

void sp_star_position_set (SPStar *star, int sides, Geom::Point center, double r1, double r2, double arg1, double arg2, bool isflat, double rounded, double randomized);

Geom::Point sp_star_get_xy (SPStar const *star, SPStarPoint point, int index, bool randomized = false);



#endif
