#define __SP_STAR_C__

/*
 * <sodipodi:star> implementation
 *
 * Authors:
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cstring>
#include <string>
#include <glibmm/i18n.h>

#include "svg/svg.h"
#include "attributes.h"
#include "display/curve.h"
#include "xml/repr.h"
#include "document.h"

#include <2geom/pathvector.h>

#include "sp-star.h"

static void sp_star_class_init (SPStarClass *klass);
static void sp_star_init (SPStar *star);

static void sp_star_build (SPObject * object, SPDocument * document, Inkscape::XML::Node * repr);
static Inkscape::XML::Node *sp_star_write (SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_star_set (SPObject *object, unsigned int key, const gchar *value);
static void sp_star_update (SPObject *object, SPCtx *ctx, guint flags);

static gchar * sp_star_description (SPItem * item);
static void sp_star_snappoints(SPItem const *item, bool const target, SnapPointsWithType &p, Inkscape::SnapPreferences const *snapprefs);

static void sp_star_set_shape (SPShape *shape);
static void sp_star_update_patheffect (SPLPEItem *lpeitem, bool write);

static SPShapeClass *parent_class;

GType
sp_star_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (SPStarClass),
			NULL, NULL,
			(GClassInitFunc) sp_star_class_init,
			NULL, NULL,
			sizeof (SPStar),
			16,
			(GInstanceInitFunc) sp_star_init,
			NULL,	/* value_table */
		};
		type = g_type_register_static (SP_TYPE_SHAPE, "SPStar", &info, (GTypeFlags)0);
	}
	return type;
}

static void
sp_star_class_init (SPStarClass *klass)
{
	GObjectClass * gobject_class;
	SPObjectClass * sp_object_class;
	SPItemClass * item_class;
	SPLPEItemClass * lpe_item_class;
	SPShapeClass * shape_class;

	gobject_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;
	item_class = (SPItemClass *) klass;
	lpe_item_class = (SPLPEItemClass *) klass;
	shape_class = (SPShapeClass *) klass;

	parent_class = (SPShapeClass *)g_type_class_ref (SP_TYPE_SHAPE);

	sp_object_class->build = sp_star_build;
	sp_object_class->write = sp_star_write;
	sp_object_class->set = sp_star_set;
	sp_object_class->update = sp_star_update;

	item_class->description = sp_star_description;
	item_class->snappoints = sp_star_snappoints;

    lpe_item_class->update_patheffect = sp_star_update_patheffect;

	shape_class->set_shape = sp_star_set_shape;
}

static void
sp_star_init (SPStar * star)
{
	star->sides = 5;
	star->center = Geom::Point(0, 0);
	star->r[0] = 1.0;
	star->r[1] = 0.001;
	star->arg[0] = star->arg[1] = 0.0;
	star->flatsided = 0;
	star->rounded = 0.0;
	star->randomized = 0.0;
}

static void
sp_star_build (SPObject * object, SPDocument * document, Inkscape::XML::Node * repr)
{
	if (((SPObjectClass *) parent_class)->build)
		((SPObjectClass *) parent_class)->build (object, document, repr);

	sp_object_read_attr (object, "sodipodi:cx");
	sp_object_read_attr (object, "sodipodi:cy");
	sp_object_read_attr (object, "sodipodi:sides");
	sp_object_read_attr (object, "sodipodi:r1");
	sp_object_read_attr (object, "sodipodi:r2");
	sp_object_read_attr (object, "sodipodi:arg1");
	sp_object_read_attr (object, "sodipodi:arg2");
	sp_object_read_attr (object, "inkscape:flatsided");
	sp_object_read_attr (object, "inkscape:rounded");
	sp_object_read_attr (object, "inkscape:randomized");
}

static Inkscape::XML::Node *
sp_star_write (SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
	SPStar *star = SP_STAR (object);

	if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
		repr = xml_doc->createElement("svg:path");
	}

	if (flags & SP_OBJECT_WRITE_EXT) {
		repr->setAttribute("sodipodi:type", "star");
		sp_repr_set_int (repr, "sodipodi:sides", star->sides);
		sp_repr_set_svg_double(repr, "sodipodi:cx", star->center[Geom::X]);
		sp_repr_set_svg_double(repr, "sodipodi:cy", star->center[Geom::Y]);
		sp_repr_set_svg_double(repr, "sodipodi:r1", star->r[0]);
		sp_repr_set_svg_double(repr, "sodipodi:r2", star->r[1]);
		sp_repr_set_svg_double(repr, "sodipodi:arg1", star->arg[0]);
		sp_repr_set_svg_double(repr, "sodipodi:arg2", star->arg[1]);
		sp_repr_set_boolean (repr, "inkscape:flatsided", star->flatsided);
		sp_repr_set_svg_double(repr, "inkscape:rounded", star->rounded);
		sp_repr_set_svg_double(repr, "inkscape:randomized", star->randomized);
	}

    sp_star_set_shape ((SPShape *) star);
    char *d = sp_svg_write_path (((SPShape *) star)->curve->get_pathvector());
    repr->setAttribute("d", d);
    g_free (d);

	if (((SPObjectClass *) (parent_class))->write)
		((SPObjectClass *) (parent_class))->write (object, xml_doc, repr, flags);

	return repr;
}

static void
sp_star_set (SPObject *object, unsigned int key, const gchar *value)
{
	SVGLength::Unit unit;

	SPStar *star = SP_STAR (object);

	/* fixme: we should really collect updates */
	switch (key) {
	case SP_ATTR_SODIPODI_SIDES:
		if (value) {
			star->sides = atoi (value);
			star->sides = NR_CLAMP(star->sides, 3, 1024);
		} else {
			star->sides = 5;
		}
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_CX:
		if (!sp_svg_length_read_ldd (value, &unit, NULL, &star->center[Geom::X]) ||
		    (unit == SVGLength::EM) ||
		    (unit == SVGLength::EX) ||
		    (unit == SVGLength::PERCENT)) {
			star->center[Geom::X] = 0.0;
		}
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_CY:
		if (!sp_svg_length_read_ldd (value, &unit, NULL, &star->center[Geom::Y]) ||
		    (unit == SVGLength::EM) ||
		    (unit == SVGLength::EX) ||
		    (unit == SVGLength::PERCENT)) {
			star->center[Geom::Y] = 0.0;
		}
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_R1:
		if (!sp_svg_length_read_ldd (value, &unit, NULL, &star->r[0]) ||
		    (unit == SVGLength::EM) ||
		    (unit == SVGLength::EX) ||
		    (unit == SVGLength::PERCENT)) {
			star->r[0] = 1.0;
		}
		/* fixme: Need CLAMP (Lauris) */
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_R2:
		if (!sp_svg_length_read_ldd (value, &unit, NULL, &star->r[1]) ||
		    (unit == SVGLength::EM) ||
		    (unit == SVGLength::EX) ||
		    (unit == SVGLength::PERCENT)) {
			star->r[1] = 0.0;
		}
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		return;
	case SP_ATTR_SODIPODI_ARG1:
		if (value) {
			star->arg[0] = g_ascii_strtod (value, NULL);
		} else {
			star->arg[0] = 0.0;
		}
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_ARG2:
		if (value) {
			star->arg[1] = g_ascii_strtod (value, NULL);
		} else {
			star->arg[1] = 0.0;
		}
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_INKSCAPE_FLATSIDED:
		if (value && !strcmp (value, "true"))
			star->flatsided = true;
		else star->flatsided = false;
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_INKSCAPE_ROUNDED:
		if (value) {
			star->rounded = g_ascii_strtod (value, NULL);
		} else {
			star->rounded = 0.0;
		}
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_INKSCAPE_RANDOMIZED:
		if (value) {
			star->randomized = g_ascii_strtod (value, NULL);
		} else {
			star->randomized = 0.0;
		}
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	default:
		if (((SPObjectClass *) parent_class)->set)
			((SPObjectClass *) parent_class)->set (object, key, value);
		break;
	}
}

static void
sp_star_update (SPObject *object, SPCtx *ctx, guint flags)
{
	if (flags & (SP_OBJECT_MODIFIED_FLAG |
		     SP_OBJECT_STYLE_MODIFIED_FLAG |
		     SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {
		sp_shape_set_shape ((SPShape *) object);
	}

	if (((SPObjectClass *) parent_class)->update)
		((SPObjectClass *) parent_class)->update (object, ctx, flags);
}

static void
sp_star_update_patheffect(SPLPEItem *lpeitem, bool write)
{
    SPShape *shape = (SPShape *) lpeitem;
    sp_star_set_shape(shape);

    if (write) {
        Inkscape::XML::Node *repr = SP_OBJECT_REPR(shape);
        if ( shape->curve != NULL ) {
            gchar *str = sp_svg_write_path(shape->curve->get_pathvector());
            repr->setAttribute("d", str);
            g_free(str);
        } else {
            repr->setAttribute("d", NULL);
        }
    }

    ((SPObject *)shape)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

static gchar *
sp_star_description (SPItem *item)
{
    SPStar *star = SP_STAR (item);

    // while there will never be less than 3 vertices, we still need to
    // make calls to ngettext because the pluralization may be different
    // for various numbers >=3.  The singular form is used as the index.
    if (star->flatsided == false )
	return g_strdup_printf (ngettext("<b>Star</b> with %d vertex",
				         "<b>Star</b> with %d vertices",
					 star->sides), star->sides);
    else
        return g_strdup_printf (ngettext("<b>Polygon</b> with %d vertex",
				         "<b>Polygon</b> with %d vertices",
					 star->sides), star->sides);
}

/**
Returns a unit-length vector at 90 degrees to the direction from o to n
 */
static Geom::Point
rot90_rel (Geom::Point o, Geom::Point n)
{
	return ((1/Geom::L2(n - o)) * Geom::Point ((n - o)[Geom::Y],  (o - n)[Geom::X]));
}

/**
Returns a unique 32 bit int for a given point.
Obvious (but acceptable for my purposes) limits to uniqueness:
- returned value for x,y repeats for x+n*1024,y+n*1024
- returned value is unchanged when the point is moved by less than 1/1024 of px
*/
static guint32
point_unique_int (Geom::Point o)
{
	return ((guint32)
	65536 *
		(((int) floor (o[Geom::X] * 64)) % 1024 + ((int) floor (o[Geom::X] * 1024)) % 64)
	+
     		(((int) floor (o[Geom::Y] * 64)) % 1024 + ((int) floor (o[Geom::Y] * 1024)) % 64)
	);
}

/**
Returns the next pseudorandom value using the Linear Congruential Generator algorithm (LCG)
with the parameters (m = 2^32, a = 69069, b = 1). These parameters give a full-period generator,
i.e. it is guaranteed to go through all integers < 2^32 (see http://random.mat.sbg.ac.at/~charly/server/server.html)
*/
static inline guint32
lcg_next(guint32 const prev)
{
	return (guint32) ( 69069 * prev + 1 );
}

/**
Returns a random number in the range [-0.5, 0.5) from the given seed, stepping the given number of steps from the seed.
*/
static double
rnd (guint32 const seed, unsigned steps) {
	guint32 lcg = seed;
	for (; steps > 0; steps --)
		lcg = lcg_next (lcg);

	return ( lcg / 4294967296. ) - 0.5;
}

static Geom::Point
sp_star_get_curvepoint (SPStar *star, SPStarPoint point, gint index, bool previ)
{
	// the point whose neighboring curve handle we're calculating
	Geom::Point o = sp_star_get_xy (star, point, index);

	// indices of previous and next points
	gint pi = (index > 0)? (index - 1) : (star->sides - 1);
	gint ni = (index < star->sides - 1)? (index + 1) : 0;

	// the other point type
	SPStarPoint other = (point == SP_STAR_POINT_KNOT2? SP_STAR_POINT_KNOT1 : SP_STAR_POINT_KNOT2);

	// the neighbors of o; depending on flatsided, they're either the same type (polygon) or the other type (star)
	Geom::Point prev = (star->flatsided? sp_star_get_xy (star, point, pi) : sp_star_get_xy (star, other, point == SP_STAR_POINT_KNOT2? index : pi));
	Geom::Point next = (star->flatsided? sp_star_get_xy (star, point, ni) : sp_star_get_xy (star, other, point == SP_STAR_POINT_KNOT1? index : ni));

	// prev-next midpoint
	Geom::Point mid =  0.5 * (prev + next);

	// point to which we direct the bissector of the curve handles;
	// it's far enough outside the star on the perpendicular to prev-next through mid
	Geom::Point biss =  mid + 100000 * rot90_rel (mid, next);

	// lengths of vectors to prev and next
	gdouble prev_len = Geom::L2 (prev - o);
	gdouble next_len = Geom::L2 (next - o);

	// unit-length vector perpendicular to o-biss
	Geom::Point rot = rot90_rel (o, biss);

	// multiply rot by star->rounded coefficient and the distance to the star point; flip for next
	Geom::Point ret;
	if (previ) {
		ret = (star->rounded * prev_len) * rot;
	} else {
		ret = (star->rounded * next_len * -1) * rot;
	}

	if (star->randomized == 0) {
		// add the vector to o to get the final curvepoint
		return o + ret;
	} else {
		// the seed corresponding to the exact point
		guint32 seed = point_unique_int (o);

		// randomly rotate (by step 3 from the seed) and scale (by step 4) the vector
		ret = ret * Geom::Matrix (Geom::Rotate (star->randomized * M_PI * rnd (seed, 3)));
		ret *= ( 1 + star->randomized * rnd (seed, 4));

		// the randomized corner point
		Geom::Point o_randomized = sp_star_get_xy (star, point, index, true);

		return o_randomized + ret;
	}
}


#define NEXT false
#define PREV true

static void
sp_star_set_shape (SPShape *shape)
{
	SPStar *star = SP_STAR (shape);

    // perhaps we should convert all our shapes into LPEs without source path
    // and with knotholders for parameters, then this situation will be handled automatically
    // by disabling the entire stack (including the shape LPE)
    if (sp_lpe_item_has_broken_path_effect(SP_LPE_ITEM(shape))) {
        g_warning ("The star shape has unknown LPE on it! Convert to path to make it editable preserving the appearance; editing it as star will remove the bad LPE");
        if (SP_OBJECT_REPR(shape)->attribute("d")) {
            // unconditionally read the curve from d, if any, to preserve appearance
            Geom::PathVector pv = sp_svg_read_pathv(SP_OBJECT_REPR(shape)->attribute("d"));
            SPCurve *cold = new SPCurve(pv);
            sp_shape_set_curve_insync (shape, cold, TRUE);
            cold->unref();
        }
        return;
    }

	SPCurve *c = new SPCurve ();

	gint sides = star->sides;
	bool not_rounded = (fabs (star->rounded) < 1e-4);

	// note that we pass randomized=true to sp_star_get_xy, because the curve must be randomized;
	// other places that call that function (e.g. the knotholder) need the exact point

	// draw 1st segment
	c->moveto(sp_star_get_xy (star, SP_STAR_POINT_KNOT1, 0, true));
	if (star->flatsided == false) {
		if (not_rounded) {
			c->lineto(sp_star_get_xy (star, SP_STAR_POINT_KNOT2, 0, true));
		} else {
			c->curveto(sp_star_get_curvepoint (star, SP_STAR_POINT_KNOT1, 0, NEXT),
				sp_star_get_curvepoint (star, SP_STAR_POINT_KNOT2, 0, PREV),
				sp_star_get_xy (star, SP_STAR_POINT_KNOT2, 0, true));
		}
	}

	// draw all middle segments
	for (gint i = 1; i < sides; i++) {
		if (not_rounded) {
			c->lineto(sp_star_get_xy (star, SP_STAR_POINT_KNOT1, i, true));
		} else {
			if (star->flatsided == false) {
				c->curveto(sp_star_get_curvepoint (star, SP_STAR_POINT_KNOT2, i - 1, NEXT),
						sp_star_get_curvepoint (star, SP_STAR_POINT_KNOT1, i, PREV),
						sp_star_get_xy (star, SP_STAR_POINT_KNOT1, i, true));
			} else {
				c->curveto(sp_star_get_curvepoint (star, SP_STAR_POINT_KNOT1, i - 1, NEXT),
						sp_star_get_curvepoint (star, SP_STAR_POINT_KNOT1, i, PREV),
						sp_star_get_xy (star, SP_STAR_POINT_KNOT1, i, true));
			}
		}
		if (star->flatsided == false) {

			if (not_rounded) {
                       c->lineto(sp_star_get_xy (star, SP_STAR_POINT_KNOT2, i, true));
			} else {
				c->curveto(sp_star_get_curvepoint (star, SP_STAR_POINT_KNOT1, i, NEXT),
					sp_star_get_curvepoint (star, SP_STAR_POINT_KNOT2, i, PREV),
					sp_star_get_xy (star, SP_STAR_POINT_KNOT2, i, true));
			}
		}
	}

	// draw last segment
		if (not_rounded) {
			c->lineto(sp_star_get_xy (star, SP_STAR_POINT_KNOT1, 0, true));
		} else {
			if (star->flatsided == false) {
			c->curveto(sp_star_get_curvepoint (star, SP_STAR_POINT_KNOT2, sides - 1, NEXT),
				sp_star_get_curvepoint (star, SP_STAR_POINT_KNOT1, 0, PREV),
				sp_star_get_xy (star, SP_STAR_POINT_KNOT1, 0, true));
			} else {
			c->curveto(sp_star_get_curvepoint (star, SP_STAR_POINT_KNOT1, sides - 1, NEXT),
				sp_star_get_curvepoint (star, SP_STAR_POINT_KNOT1, 0, PREV),
				sp_star_get_xy (star, SP_STAR_POINT_KNOT1, 0, true));
			}
		}

    c->closepath();

    /* Reset the shape'scurve to the "original_curve"
     * This is very important for LPEs to work properly! (the bbox might be recalculated depending on the curve in shape)*/
    sp_shape_set_curve_insync (shape, c, TRUE);
    if (sp_lpe_item_has_path_effect(SP_LPE_ITEM(shape)) && sp_lpe_item_path_effects_enabled(SP_LPE_ITEM(shape))) {
        SPCurve *c_lpe = c->copy();
        bool success = sp_lpe_item_perform_path_effect(SP_LPE_ITEM (shape), c_lpe);
        if (success) {
            sp_shape_set_curve_insync (shape, c_lpe, TRUE);
        } 
        c_lpe->unref();
    }
    c->unref();
}

void
sp_star_position_set (SPStar *star, gint sides, Geom::Point center, gdouble r1, gdouble r2, gdouble arg1, gdouble arg2, bool isflat, double rounded, double randomized)
{
	g_return_if_fail (star != NULL);
	g_return_if_fail (SP_IS_STAR (star));

	star->sides = NR_CLAMP(sides, 3, 1024);
	star->center = center;
	star->r[0] = MAX (r1, 0.001);
	if (isflat == false) {
		star->r[1] = NR_CLAMP(r2, 0.0, star->r[0]);
	} else {
		star->r[1] = NR_CLAMP( r1*cos(M_PI/sides) ,0.0, star->r[0] );
	}
	star->arg[0] = arg1;
	star->arg[1] = arg2;
	star->flatsided = isflat;
	star->rounded = rounded;
	star->randomized = randomized;
	SP_OBJECT(star)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

static void sp_star_snappoints(SPItem const *item, bool const target, SnapPointsWithType &p, Inkscape::SnapPreferences const *snapprefs)
{
	// We will determine the star's midpoint ourselves, instead of trusting on the base class
	// Therefore setSnapObjectMidpoints() is set to false temporarily
	Inkscape::SnapPreferences local_snapprefs = *snapprefs;
	local_snapprefs.setSnapObjectMidpoints(false);

	if (((SPItemClass *) parent_class)->snappoints) {
		((SPItemClass *) parent_class)->snappoints (item, target, p, &local_snapprefs);
	}

	// Help enforcing strict snapping, i.e. only return nodes when we're snapping nodes to nodes or a guide to nodes
	if (!(snapprefs->getSnapModeNode() || snapprefs->getSnapModeGuide())) {
		return;
	}

	if (snapprefs->getSnapObjectMidpoints()) {
		Geom::Matrix const i2d (sp_item_i2d_affine (item));
		int type = target ? int(Inkscape::SNAPTARGET_OBJECT_MIDPOINT) : int(Inkscape::SNAPSOURCE_OBJECT_MIDPOINT);
		p.push_back(std::make_pair(SP_STAR(item)->center * i2d, type));
	}
}

/**
 * sp_star_get_xy: Get X-Y value as item coordinate system
 * @star: star item
 * @point: point type to obtain X-Y value
 * @index: index of vertex
 * @p: pointer to store X-Y value
 * @randomized: false (default) if you want to get exact, not randomized point
 *
 * Initial item coordinate system is same as document coordinate system.
 */

Geom::Point
sp_star_get_xy (SPStar *star, SPStarPoint point, gint index, bool randomized)
{
	gdouble darg = 2.0 * M_PI / (double) star->sides;

	double arg = star->arg[point];
	arg += index * darg;

	Geom::Point xy = star->r[point] * Geom::Point(cos(arg), sin(arg)) + star->center;

	if (!randomized || star->randomized == 0) {
		// return the exact point
		return xy;
	} else { // randomize the point
		// find out the seed, unique for this point so that randomization is the same so long as the original point is stationary
		guint32 seed = point_unique_int (xy);
		// the full range (corresponding to star->randomized == 1.0) is equal to the star's diameter
		double range = 2 * MAX (star->r[0], star->r[1]);
		// find out the random displacement; x is controlled by step 1 from the seed, y by the step 2
		Geom::Point shift (star->randomized * range * rnd (seed, 1), star->randomized * range * rnd (seed, 2));
		// add the shift to the exact point
		return xy + shift;
	}
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
