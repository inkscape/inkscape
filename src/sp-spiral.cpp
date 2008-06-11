#define __SP_SPIRAL_C__

/** \file
 * <sodipodi:spiral> implementation
 */
/*
 * Authors:
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "config.h"


#include "svg/svg.h"
#include "attributes.h"
#include "display/bezier-utils.h"
#include "display/curve.h"
#include <glibmm/i18n.h>
#include "xml/repr.h"
#include "document.h"

#include "sp-spiral.h"

static void sp_spiral_class_init (SPSpiralClass *klass);
static void sp_spiral_init (SPSpiral *spiral);

static void sp_spiral_build (SPObject * object, SPDocument * document, Inkscape::XML::Node * repr);
static Inkscape::XML::Node *sp_spiral_write (SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);
static void sp_spiral_set (SPObject *object, unsigned int key, const gchar *value);
static void sp_spiral_update (SPObject *object, SPCtx *ctx, guint flags);

static gchar * sp_spiral_description (SPItem * item);
static void sp_spiral_snappoints(SPItem const *item, SnapPointsIter p);

static void sp_spiral_set_shape (SPShape *shape);
static void sp_spiral_update_patheffect (SPLPEItem *lpeitem, bool write);

static NR::Point sp_spiral_get_tangent (SPSpiral const *spiral, gdouble t);

static SPShapeClass *parent_class;

/**
 * Register SPSpiral class and return its type number.
 */
GType
sp_spiral_get_type (void)
{
	static GType spiral_type = 0;

	if (!spiral_type) {
		GTypeInfo spiral_info = {
			sizeof (SPSpiralClass),
			NULL,	/* base_init */
			NULL,	/* base_finalize */
			(GClassInitFunc) sp_spiral_class_init,
			NULL,	/* class_finalize */
			NULL,	/* class_data */
			sizeof (SPSpiral),
			16,	/* n_preallocs */
			(GInstanceInitFunc) sp_spiral_init,
			NULL,	/* value_table */
		};
		spiral_type = g_type_register_static (SP_TYPE_SHAPE, "SPSpiral", &spiral_info, (GTypeFlags)0);
	}
	return spiral_type;
}

/**
 * SPSpiral vtable initialization.
 */
static void
sp_spiral_class_init (SPSpiralClass *klass)
{
	GObjectClass * gobject_class;
	SPObjectClass * sp_object_class;
	SPItemClass * item_class;
	SPLPEItemClass * lpe_item_class;
	SPShapeClass *shape_class;

	gobject_class = (GObjectClass *) klass;
	sp_object_class = (SPObjectClass *) klass;
	item_class = (SPItemClass *) klass;
	lpe_item_class = (SPLPEItemClass *) klass;
	shape_class = (SPShapeClass *) klass;

	parent_class = (SPShapeClass *)g_type_class_ref (SP_TYPE_SHAPE);

	sp_object_class->build = sp_spiral_build;
	sp_object_class->write = sp_spiral_write;
	sp_object_class->set = sp_spiral_set;
	sp_object_class->update = sp_spiral_update;

	item_class->description = sp_spiral_description;
	item_class->snappoints = sp_spiral_snappoints;

    lpe_item_class->update_patheffect = sp_spiral_update_patheffect;

    shape_class->set_shape = sp_spiral_set_shape;
}

/**
 * Callback for SPSpiral object initialization.
 */
static void
sp_spiral_init (SPSpiral * spiral)
{
	spiral->cx         = 0.0;
	spiral->cy         = 0.0;
	spiral->exp        = 1.0;
	spiral->revo       = 3.0;
	spiral->rad        = 1.0;
	spiral->arg        = 0.0;
	spiral->t0         = 0.0;
}

/**
 * Virtual build: set spiral properties from corresponding repr.
 */
static void
sp_spiral_build (SPObject * object, SPDocument * document, Inkscape::XML::Node * repr)
{
	if (((SPObjectClass *) parent_class)->build)
		((SPObjectClass *) parent_class)->build (object, document, repr);

	sp_object_read_attr (object, "sodipodi:cx");
	sp_object_read_attr (object, "sodipodi:cy");
	sp_object_read_attr (object, "sodipodi:expansion");
	sp_object_read_attr (object, "sodipodi:revolution");
	sp_object_read_attr (object, "sodipodi:radius");
	sp_object_read_attr (object, "sodipodi:argument");
	sp_object_read_attr (object, "sodipodi:t0");
}

/**
 * Virtual write: write spiral attributes to corresponding repr.
 */
static Inkscape::XML::Node *
sp_spiral_write (SPObject *object, Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
	SPSpiral *spiral = SP_SPIRAL (object);

	if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
		repr = xml_doc->createElement("svg:path");
	}

	if (flags & SP_OBJECT_WRITE_EXT) {
		/* Fixme: we may replace these attributes by
		 * sodipodi:spiral="cx cy exp revo rad arg t0"
		 */
		repr->setAttribute("sodipodi:type", "spiral");
		sp_repr_set_svg_double(repr, "sodipodi:cx", spiral->cx);
		sp_repr_set_svg_double(repr, "sodipodi:cy", spiral->cy);
		sp_repr_set_svg_double(repr, "sodipodi:expansion", spiral->exp);
		sp_repr_set_svg_double(repr, "sodipodi:revolution", spiral->revo);
		sp_repr_set_svg_double(repr, "sodipodi:radius", spiral->rad);
		sp_repr_set_svg_double(repr, "sodipodi:argument", spiral->arg);
		sp_repr_set_svg_double(repr, "sodipodi:t0", spiral->t0);
	}

     // make sure the curve is rebuilt with all up-to-date parameters
     sp_spiral_set_shape ((SPShape *) spiral);

    //Duplicate the path
    SPCurve *curve = ((SPShape *) spiral)->curve;
    //Nulls might be possible if this called iteratively
    if ( !curve ) {
            //g_warning("sp_spiral_write(): No path to copy\n");
            return NULL;
    }
    char *d = sp_svg_write_path ( curve->get_pathvector() );
    repr->setAttribute("d", d);
    g_free (d);

    if (((SPObjectClass *) (parent_class))->write)
        ((SPObjectClass *) (parent_class))->write (object, xml_doc, repr, flags | SP_SHAPE_WRITE_PATH);

    return repr;
}

/**
 * Virtual set: change spiral object attribute.
 */
static void
sp_spiral_set (SPObject *object, unsigned int key, const gchar *value)
{
	SPSpiral *spiral;
	SPShape  *shape;

	spiral = SP_SPIRAL (object);
	shape  = SP_SHAPE (object);

	/// \todo fixme: we should really collect updates
	switch (key) {
	case SP_ATTR_SODIPODI_CX:
		if (!sp_svg_length_read_computed_absolute (value, &spiral->cx)) {
			spiral->cx = 0.0;
		}
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_CY:
		if (!sp_svg_length_read_computed_absolute (value, &spiral->cy)) {
			spiral->cy = 0.0;
		}
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_EXPANSION:
		if (value) {
			/** \todo
                         * FIXME: check that value looks like a (finite)
                         * number. Create a routine that uses strtod, and
                         * accepts a default value (if strtod finds an error).
                         * N.B. atof/sscanf/strtod consider "nan" and "inf"
                         * to be valid numbers.
                         */
			spiral->exp = g_ascii_strtod (value, NULL);
			spiral->exp = CLAMP (spiral->exp, 0.0, 1000.0);
		} else {
			spiral->exp = 1.0;
		}
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_REVOLUTION:
		if (value) {
			spiral->revo = g_ascii_strtod (value, NULL);
			spiral->revo = CLAMP (spiral->revo, 0.05, 1024.0);
		} else {
			spiral->revo = 3.0;
		}
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_RADIUS:
		if (!sp_svg_length_read_computed_absolute (value, &spiral->rad)) {
			spiral->rad = MAX (spiral->rad, 0.001);
		}
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_ARGUMENT:
		if (value) {
			spiral->arg = g_ascii_strtod (value, NULL);
			/** \todo
                         * FIXME: We still need some bounds on arg, for
                         * numerical reasons. E.g., we don't want inf or NaN,
                         * nor near-infinite numbers. I'm inclined to take
                         * modulo 2*pi.  If so, then change the knot editors,
                         * which use atan2 - revo*2*pi, which typically
                         * results in very negative arg.
                         */
		} else {
			spiral->arg = 0.0;
		}
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	case SP_ATTR_SODIPODI_T0:
		if (value) {
			spiral->t0 = g_ascii_strtod (value, NULL);
			spiral->t0 = CLAMP (spiral->t0, 0.0, 0.999);
			/** \todo
                         * Have shared constants for the allowable bounds for
                         * attributes. There was a bug here where we used -1.0
                         * as the minimum (which leads to NaN via, e.g.,
                         * pow(-1.0, 0.5); see sp_spiral_get_xy for
                         * requirements.
                         */
		} else {
			spiral->t0 = 0.0;
		}
		object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
		break;
	default:
		if (((SPObjectClass *) parent_class)->set)
			((SPObjectClass *) parent_class)->set (object, key, value);
		break;
	}
}

/**
 * Virtual update callback.
 */
static void
sp_spiral_update (SPObject *object, SPCtx *ctx, guint flags)
{
	if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {
		sp_shape_set_shape ((SPShape *) object);
	}

	if (((SPObjectClass *) parent_class)->update)
		((SPObjectClass *) parent_class)->update (object, ctx, flags);
}

static void
sp_spiral_update_patheffect(SPLPEItem *lpeitem, bool write)
{
    SPShape *shape = (SPShape *) lpeitem;
    sp_spiral_set_shape(shape);

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

/**
 * Return textual description of spiral.
 */
static gchar *
sp_spiral_description (SPItem * item)
{
	// TRANSLATORS: since turn count isn't an integer, please adjust the
	// string as needed to deal with an localized plural forms.
	return g_strdup_printf (_("<b>Spiral</b> with %3f turns"), SP_SPIRAL(item)->revo);
}


/**
 * Fit beziers together to spiral and draw it.
 *
 * \pre dstep \> 0.
 * \pre is_unit_vector(*hat1).
 * \post is_unit_vector(*hat2).
 **/
static void
sp_spiral_fit_and_draw (SPSpiral const *spiral,
			SPCurve	 *c,
			double dstep,
			NR::Point darray[],
			NR::Point const &hat1,
			NR::Point &hat2,
			double *t)
{
#define BEZIER_SIZE   4
#define FITTING_MAX_BEZIERS 4
#define BEZIER_LENGTH (BEZIER_SIZE * FITTING_MAX_BEZIERS)
	g_assert (dstep > 0);
	g_assert (is_unit_vector (hat1));

	NR::Point bezier[BEZIER_LENGTH];
	double d;
	int depth, i;

	for (d = *t, i = 0; i <= SAMPLE_SIZE; d += dstep, i++) {
		darray[i] = sp_spiral_get_xy(spiral, d);

		/* Avoid useless adjacent dups.  (Otherwise we can have all of darray filled with
		   the same value, which upsets chord_length_parameterize.) */
		if ((i != 0)
		    && (darray[i] == darray[i - 1])
		    && (d < 1.0)) {
			i--;
			d += dstep;
			/** We mustn't increase dstep for subsequent values of
                         * i: for large spiral.exp values, rate of growth
                         * increases very rapidly.
                         */
                        /** \todo
                         * Get the function itself to decide what value of d
                         * to use next: ensure that we move at least 0.25 *
                         * stroke width, for example.  The derivative (as used
                         * for get_tangent before normalization) would be
                         * useful for estimating the appropriate d value.  Or
                         * perhaps just start with a small dstep and scale by
                         * some small number until we move >= 0.25 *
                         * stroke_width.  Must revert to the original dstep
                         * value for next iteration to avoid the problem
                         * mentioned above.
                         */
		}
	}

	double const next_t = d - 2 * dstep;
	/* == t + (SAMPLE_SIZE - 1) * dstep, in absence of dups. */

	hat2 = -sp_spiral_get_tangent (spiral, next_t);

	/** \todo
         * We should use better algorithm to specify maximum error.
         */
	depth = sp_bezier_fit_cubic_full (bezier, NULL, darray, SAMPLE_SIZE,
					  hat1, hat2,
					  SPIRAL_TOLERANCE*SPIRAL_TOLERANCE,
					  FITTING_MAX_BEZIERS);
	g_assert(depth * BEZIER_SIZE <= gint(G_N_ELEMENTS(bezier)));
#ifdef SPIRAL_DEBUG
	if (*t == spiral->t0 || *t == 1.0)
		g_print ("[%s] depth=%d, dstep=%g, t0=%g, t=%g, arg=%g\n",
			 debug_state, depth, dstep, spiral->t0, *t, spiral->arg);
#endif
	if (depth != -1) {
		for (i = 0; i < 4*depth; i += 4) {
			c->curveto(bezier[i + 1],
					  bezier[i + 2],
					  bezier[i + 3]);
		}
	} else {
#ifdef SPIRAL_VERBOSE
		g_print ("cant_fit_cubic: t=%g\n", *t);
#endif
		for (i = 1; i < SAMPLE_SIZE; i++)
			c->lineto(darray[i]);
	}
	*t = next_t;
	g_assert (is_unit_vector (hat2));
}

static void
sp_spiral_set_shape (SPShape *shape)
{
	NR::Point darray[SAMPLE_SIZE + 1];
	double t;

	SPSpiral *spiral = SP_SPIRAL(shape);

	SP_OBJECT (spiral)->requestModified(SP_OBJECT_MODIFIED_FLAG);

	SPCurve *c = new SPCurve ();
	
#ifdef SPIRAL_VERBOSE
	g_print ("cx=%g, cy=%g, exp=%g, revo=%g, rad=%g, arg=%g, t0=%g\n",
		 spiral->cx,
		 spiral->cy,
		 spiral->exp,
		 spiral->revo,
		 spiral->rad,
		 spiral->arg,
		 spiral->t0);
#endif

	/* Initial moveto. */
	c->moveto(sp_spiral_get_xy(spiral, spiral->t0));

	double const tstep = SAMPLE_STEP / spiral->revo;
	double const dstep = tstep / (SAMPLE_SIZE - 1);

	NR::Point hat1 = sp_spiral_get_tangent (spiral, spiral->t0);
	NR::Point hat2;
	for (t = spiral->t0; t < (1.0 - tstep);) {
		sp_spiral_fit_and_draw (spiral, c, dstep, darray, hat1, hat2, &t);

		hat1 = -hat2;
	}
	if ((1.0 - t) > SP_EPSILON)
		sp_spiral_fit_and_draw (spiral, c, (1.0 - t)/(SAMPLE_SIZE - 1.0),
					darray, hat1, hat2, &t);

    sp_lpe_item_perform_path_effect(SP_LPE_ITEM (spiral), c);
    sp_shape_set_curve_insync ((SPShape *) spiral, c, TRUE);
    c->unref();
}

/**
 * Set spiral properties and update display.
 */
void
sp_spiral_position_set       (SPSpiral          *spiral,
		     gdouble            cx,
		     gdouble            cy,
		     gdouble            exp,
		     gdouble            revo,
		     gdouble            rad,
		     gdouble            arg,
		     gdouble            t0)
{
	g_return_if_fail (spiral != NULL);
	g_return_if_fail (SP_IS_SPIRAL (spiral));

	/** \todo
         * Consider applying CLAMP or adding in-bounds assertions for
         * some of these parameters.
         */
	spiral->cx         = cx;
	spiral->cy         = cy;
	spiral->exp        = exp;
	spiral->revo       = revo;
	spiral->rad        = MAX (rad, 0.001);
	spiral->arg        = arg;
	spiral->t0         = CLAMP(t0, 0.0, 0.999);
	
	((SPObject *)spiral)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

/**
 * Virtual snappoints callback.
 */
static void sp_spiral_snappoints(SPItem const *item, SnapPointsIter p)
{
	if (((SPItemClass *) parent_class)->snappoints) {
		((SPItemClass *) parent_class)->snappoints (item, p);
	}
}

/**
 * Return one of the points on the spiral.
 *
 * \param t specifies how far along the spiral.
 * \pre \a t in [0.0, 2.03].  (It doesn't make sense for t to be much more
 * than 1.0, though some callers go slightly beyond 1.0 for curve-fitting
 * purposes.)
 */
NR::Point sp_spiral_get_xy (SPSpiral const *spiral, gdouble t)
{
	g_assert (spiral != NULL);
	g_assert (SP_IS_SPIRAL(spiral));
	g_assert (spiral->exp >= 0.0);
	/* Otherwise we get NaN for t==0. */
	g_assert (spiral->exp <= 1000.0);
	/* Anything much more results in infinities.  Even allowing 1000 is somewhat overkill. */
	g_assert (t >= 0.0);
	/* Any callers passing -ve t will have a bug for non-integral values of exp. */

	double const rad = spiral->rad * pow(t, (double) spiral->exp);
	double const arg = 2.0 * M_PI * spiral->revo * t + spiral->arg;

	return NR::Point(rad * cos (arg) + spiral->cx,
			 rad * sin (arg) + spiral->cy);
}


/**
 * Returns the derivative of sp_spiral_get_xy with respect to t,
 *  scaled to a unit vector.
 *
 *  \pre spiral != 0.
 *  \pre 0 \<= t.
 *  \pre p != NULL.
 *  \post is_unit_vector(*p).
 */
static NR::Point
sp_spiral_get_tangent (SPSpiral const *spiral, gdouble t)
{
	NR::Point ret(1.0, 0.0);
	g_return_val_if_fail (( ( spiral != NULL )
				&& SP_IS_SPIRAL(spiral) ),
			      ret);
	g_assert (t >= 0.0);
	g_assert (spiral->exp >= 0.0);
	/* See above for comments on these assertions. */

	double const t_scaled = 2.0 * M_PI * spiral->revo * t;
	double const arg = t_scaled + spiral->arg;
	double const s = sin (arg);
	double const c = cos (arg);

	if (spiral->exp == 0.0) {
		ret = NR::Point(-s, c);
	} else if (t_scaled == 0.0) {
		ret = NR::Point(c, s);
	} else {
		NR::Point unrotated(spiral->exp, t_scaled);
		double const s_len = L2 (unrotated);
		g_assert (s_len != 0);
		/** \todo
                 * Check that this isn't being too hopeful of the hypot
                 * function.  E.g. test with numbers around 2**-1070
                 * (denormalized numbers), preferably on a few different
                 * platforms.  However, njh says that the usual implementation
                 * does handle both very big and very small numbers.
                 */
		unrotated /= s_len;

		/* ret = spiral->exp * (c, s) + t_scaled * (-s, c);
		   alternatively ret = (spiral->exp, t_scaled) * (( c, s),
								  (-s, c)).*/
		ret = NR::Point(dot(unrotated, NR::Point(c, -s)),
				dot(unrotated, NR::Point(s, c)));
		/* ret should already be approximately normalized: the
		   matrix ((c, -s), (s, c)) is orthogonal (it just
		   rotates by arg), and unrotated has been normalized,
		   so ret is already of unit length other than numerical
		   error in the above matrix multiplication. */

		/** \todo
                 * I haven't checked how important it is for ret to be very
                 * near unit length; we could get rid of the below.
                 */

		ret.normalize();
		/* Proof that ret length is non-zero: see above.  (Should be near 1.) */
	}

	g_assert (is_unit_vector (ret));
	return ret;
}

/**
 * Compute rad and/or arg for point on spiral.
 */
void
sp_spiral_get_polar (SPSpiral const *spiral, gdouble t, gdouble *rad, gdouble *arg)
{
	g_return_if_fail (spiral != NULL);
	g_return_if_fail (SP_IS_SPIRAL(spiral));

	if (rad)
		*rad = spiral->rad * pow(t, (double) spiral->exp);
	if (arg)
		*arg = 2.0 * M_PI * spiral->revo * t + spiral->arg;
}

/**
 * Return true if spiral has properties that make it invalid.
 */
bool
sp_spiral_is_invalid (SPSpiral const *spiral)
{
	gdouble rad;

	sp_spiral_get_polar (spiral, 0.0, &rad, NULL);
	if (rad < 0.0 || rad > SP_HUGE) {
		g_print ("rad(t=0)=%g\n", rad);
		return TRUE;
	}
	sp_spiral_get_polar (spiral, 1.0, &rad, NULL);
	if (rad < 0.0 || rad > SP_HUGE) {
		g_print ("rad(t=1)=%g\n", rad);
		return TRUE;
	}
	return FALSE;
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
