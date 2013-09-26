/*
 * SVG <ellipse> and related implementations
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Mitsuru Oka
 *   bulia byak <buliabyak@users.sf.net>
 *   Abhishek Sharma
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "svg/svg.h"
#include "svg/path-string.h"
#include "xml/repr.h"
#include "attributes.h"
#include "style.h"
#include "display/curve.h"
#include <glibmm/i18n.h>
#include <2geom/transforms.h>
#include <2geom/pathvector.h>
#include <2geom/svg-path.h>
#include "document.h"
#include "sp-ellipse.h"
#include "preferences.h"
#include "snap-candidate.h"

/* Common parent class */

#define noELLIPSE_VERBOSE


#include "sp-factory.h"

namespace {
	SPObject* createEllipse() {
		return new SPEllipse();
	}

	SPObject* createCircle() {
		return new SPCircle();
	}

	SPObject* createArc() {
		return new SPArc();
	}

	bool ellipseRegistered = SPFactory::instance().registerObject("svg:ellipse", createEllipse);
	bool circleRegistered = SPFactory::instance().registerObject("svg:circle", createCircle);
	bool arcRegistered = SPFactory::instance().registerObject("arc", createArc);
}


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SP_2PI (2 * M_PI)

#if 1
/* Hmmm... shouldn't this also qualify */
/* Whether it is faster or not, well, nobody knows */
#define sp_round(v,m) (((v) < 0.0) ? ((ceil((v) / (m) - 0.5)) * (m)) : ((floor((v) / (m) + 0.5)) * (m)))
#else
/* we do not use C99 round(3) function yet */
static double sp_round(double x, double y)
{
    double remain;

    g_assert(y > 0.0);

    /* return round(x/y) * y; */

    remain = fmod(x, y);

    if (remain >= 0.5*y)
        return x - remain + y;
    else
        return x - remain;
}
#endif

static gboolean sp_arc_set_elliptical_path_attribute(SPArc *arc, Inkscape::XML::Node *repr);

SPGenericEllipse::SPGenericEllipse() : SPShape() {
    this->cx.unset();
    this->cy.unset();
    this->rx.unset();
    this->ry.unset();

    this->start = 0.0;
    this->end = SP_2PI;
    this->closed = TRUE;
}

SPGenericEllipse::~SPGenericEllipse() {
}

void SPGenericEllipse::update(SPCtx *ctx, guint flags) {
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {
        Geom::Rect const &viewbox = ((SPItemCtx const *) ctx)->viewport;

        double const dx = viewbox.width();
        double const dy = viewbox.height();
        double const dr = sqrt(dx*dx + dy*dy)/sqrt(2);
        double const em = this->style->font_size.computed;
        double const ex = em * 0.5; // fixme: get from pango or libnrtype

        this->cx.update(em, ex, dx);
        this->cy.update(em, ex, dy);
        this->rx.update(em, ex, dr);
        this->ry.update(em, ex, dr);

        this->set_shape();
    }

    SPShape::update(ctx, flags);
}

void SPGenericEllipse::update_patheffect(bool write) {
    this->set_shape();

    if (write) {
        Inkscape::XML::Node *repr = this->getRepr();

        if ( this->_curve != NULL ) {
            gchar *str = sp_svg_write_path(this->_curve->get_pathvector());
            repr->setAttribute("d", str);
            g_free(str);
        } else {
            repr->setAttribute("d", NULL);
        }
    }

    this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

#include <2geom/ellipse.h>

/* fixme: Think (Lauris) */
/* Can't we use arcto in this method? */
void SPGenericEllipse::set_shape() {
    if (hasBrokenPathEffect()) {
        g_warning ("The ellipse shape has unknown LPE on it! Convert to path to make it editable preserving the appearance; editing it as ellipse will remove the bad LPE");

        if (this->getRepr()->attribute("d")) {
            // unconditionally read the curve from d, if any, to preserve appearance
            Geom::PathVector pv = sp_svg_read_pathv(this->getRepr()->attribute("d"));
            SPCurve *cold = new SPCurve(pv);
            this->setCurveInsync( cold, TRUE);
            cold->unref();
        }

        return;
    }

    if ((this->rx.computed < 1e-18) || (this->ry.computed < 1e-18)) {
    	return;
    }

    if (fabs(this->end - this->start) < 1e-9) {
    	return;
    }

    sp_genericellipse_normalize(this);

    // figure out if we have a slice, guarding against rounding errors
    double len = fmod(this->end - this->start, SP_2PI);

    if (len < 0.0) {
    	len += SP_2PI;
    }

    bool slice = false;

    if (fabs(len) < 1e-8 || fabs(len - SP_2PI) < 1e-8) {
        slice = false;
        this->end = this->start + SP_2PI;
    } else {
        slice = true;
    }

    SPCurve *curve = NULL;

    // For simplicity, we use a circle with center (0, 0) and radius 1 for our calculations.

    if (slice) {
        Geom::Point startPoint(cos(start), sin(start));
        Geom::Point endPoint(cos(end), sin(end));
        Geom::Point middlePoint = make_angle_bisector_ray(Geom::Ray(Geom::Point(), start), Geom::Ray(Geom::Point(), end)).versor();

        Geom::Ellipse ellipse(0, 0, 1, 1, 0);
        Geom::EllipticalArc *arc = ellipse.arc(startPoint, middlePoint, endPoint);

        Geom::Path path(startPoint);
        path.append(*arc);

        delete arc;

        Geom::PathBuilder pb;
        pb.append(path);

        if (this->closed) {
            // "pizza slice"
            pb.lineTo(Geom::Point(0, 0));
            pb.closePath();
        } else {
            // arc only
            pb.finish();
        }

        curve = new SPCurve(pb.peek());
    } else {
        // Full ellipse
        Geom::Circle circle(0, 0, 1);
        Geom::PathVector path;

        circle.getPath(path);

        curve = new SPCurve(path);
        curve->closepath();
    }

    // Stretching / moving the calculated shape to fit the actual dimensions.
    Geom::Affine aff = Geom::Scale(rx.computed, ry.computed) * Geom::Translate(cx.computed, cy.computed);
    curve->transform(aff);

    /* Reset the shape's curve to the "original_curve"
     * This is very important for LPEs to work properly! (the bbox might be recalculated depending on the curve in shape)*/
    this->setCurveInsync( curve, TRUE);
    this->setCurveBeforeLPE(curve);

    if (hasPathEffect() && sp_lpe_item_path_effects_enabled(this)) {
        SPCurve *c_lpe = curve->copy();
        bool success = sp_lpe_item_perform_path_effect(this, c_lpe);

        if (success) {
        	this->setCurveInsync( c_lpe, TRUE);
        }

        c_lpe->unref();
    }

    curve->unref();
}

void SPGenericEllipse::snappoints(std::vector<Inkscape::SnapCandidatePoint> &p, Inkscape::SnapPreferences const *snapprefs) {
    sp_genericellipse_normalize(this);
    Geom::Affine const i2dt = this->i2dt_affine();

    // figure out if we have a slice, while guarding against rounding errors
    bool slice = false;
    double len = fmod(this->end - this->start, SP_2PI);

    if (len < 0.0) {
    	len += SP_2PI;
    }

    if (fabs(len) < 1e-8 || fabs(len - SP_2PI) < 1e-8) {
        slice = false;
        this->end = this->start + SP_2PI;
    } else {
        slice = true;
    }

    double rx = this->rx.computed;
    double ry = this->ry.computed;
    double cx = this->cx.computed;
    double cy = this->cy.computed;

    Geom::Point pt;

    // Snap to the 4 quadrant points of the this, but only if the arc
    // spans far enough to include them
    if (snapprefs->isTargetSnappable(Inkscape::SNAPTARGET_ELLIPSE_QUADRANT_POINT)) {
        double angle = 0;

        for (angle = 0; angle < SP_2PI; angle += M_PI_2) {
            if (angle >= this->start && angle <= this->end) {
                pt = Geom::Point(cx + cos(angle)*rx, cy + sin(angle)*ry) * i2dt;
                p.push_back(Inkscape::SnapCandidatePoint(pt, Inkscape::SNAPSOURCE_ELLIPSE_QUADRANT_POINT, Inkscape::SNAPTARGET_ELLIPSE_QUADRANT_POINT));
            }
        }
    }

    // Add the centre, if we have a closed slice or when explicitly asked for
    bool c1 = snapprefs->isTargetSnappable(Inkscape::SNAPTARGET_NODE_CUSP) && slice && this->closed;
    bool c2 = snapprefs->isTargetSnappable(Inkscape::SNAPTARGET_OBJECT_MIDPOINT);

    if (c1 || c2) {
        pt = Geom::Point(cx, cy) * i2dt;

        if (c1) {
            p.push_back(Inkscape::SnapCandidatePoint(pt, Inkscape::SNAPSOURCE_NODE_CUSP, Inkscape::SNAPTARGET_NODE_CUSP));
        }

        if (c2) {
            p.push_back(Inkscape::SnapCandidatePoint(pt, Inkscape::SNAPSOURCE_OBJECT_MIDPOINT, Inkscape::SNAPTARGET_OBJECT_MIDPOINT));
        }
    }

    // And if we have a slice, also snap to the endpoints
    if (snapprefs->isTargetSnappable(Inkscape::SNAPTARGET_NODE_CUSP) && slice) {
        // Add the start point, if it's not coincident with a quadrant point
        if (fmod(this->start, M_PI_2) != 0.0 ) {
            pt = Geom::Point(cx + cos(this->start)*rx, cy + sin(this->start)*ry) * i2dt;
            p.push_back(Inkscape::SnapCandidatePoint(pt, Inkscape::SNAPSOURCE_NODE_CUSP, Inkscape::SNAPTARGET_NODE_CUSP));
        }

        // Add the end point, if it's not coincident with a quadrant point
        if (fmod(this->end, M_PI_2) != 0.0 ) {
            pt = Geom::Point(cx + cos(this->end)*rx, cy + sin(this->end)*ry) * i2dt;
            p.push_back(Inkscape::SnapCandidatePoint(pt, Inkscape::SNAPSOURCE_NODE_CUSP, Inkscape::SNAPTARGET_NODE_CUSP));
        }
    }
}

Geom::Affine SPGenericEllipse::set_transform(Geom::Affine const &xform)
{
    /* Calculate ellipse start in parent coords. */
    Geom::Point pos( Geom::Point(this->cx.computed, this->cy.computed) * xform );

    /* This function takes care of translation and scaling, we return whatever parts we can't
       handle. */
    Geom::Affine ret(Geom::Affine(xform).withoutTranslation());
    gdouble const sw = hypot(ret[0], ret[1]);
    gdouble const sh = hypot(ret[2], ret[3]);
    if (sw > 1e-9) {
        ret[0] /= sw;
        ret[1] /= sw;
    } else {
        ret[0] = 1.0;
        ret[1] = 0.0;
    }
    if (sh > 1e-9) {
        ret[2] /= sh;
        ret[3] /= sh;
    } else {
        ret[2] = 0.0;
        ret[3] = 1.0;
    }

    if (this->rx._set) {
        this->rx = this->rx.computed * sw;
    }
    if (this->ry._set) {
        this->ry = this->ry.computed * sh;
    }

    /* Find start in item coords */
    pos = pos * ret.inverse();
    this->cx = pos[Geom::X];
    this->cy = pos[Geom::Y];

    this->set_shape();

    // Adjust stroke width
    this->adjust_stroke(sqrt(fabs(sw * sh)));

    // Adjust pattern fill
    this->adjust_pattern(xform * ret.inverse());

    // Adjust gradient fill
    this->adjust_gradient(xform * ret.inverse());

    this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);

    return ret;
}

void
sp_genericellipse_normalize(SPGenericEllipse *ellipse)
{
    ellipse->start = fmod(ellipse->start, SP_2PI);
    ellipse->end = fmod(ellipse->end, SP_2PI);

    if (ellipse->start < 0.0) {
        ellipse->start += SP_2PI;
    }

    double diff = ellipse->start - ellipse->end;

    if (diff >= 0.0) {
        ellipse->end += diff - fmod(diff, SP_2PI) + SP_2PI;
    }

    /* Now we keep: 0 <= start < end <= 2*PI */
}

Inkscape::XML::Node* SPGenericEllipse::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
    if (flags & SP_OBJECT_WRITE_EXT) {
        if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
            repr = xml_doc->createElement("svg:path");
        }

        sp_repr_set_svg_double(repr, "sodipodi:cx", this->cx.computed);
        sp_repr_set_svg_double(repr, "sodipodi:cy", this->cy.computed);
        sp_repr_set_svg_double(repr, "sodipodi:rx", this->rx.computed);
        sp_repr_set_svg_double(repr, "sodipodi:ry", this->ry.computed);

        if (SP_IS_ARC(this)) {
            sp_arc_set_elliptical_path_attribute(SP_ARC(this), this->getRepr());
        }
    }

    this->set_shape(); // evaluate SPCurve

    SPShape::write(xml_doc, repr, flags);

    return repr;
}

/* SVG <ellipse> element */
SPEllipse::SPEllipse() : SPGenericEllipse() {
}

SPEllipse::~SPEllipse() {
}

void SPEllipse::build(SPDocument *document, Inkscape::XML::Node *repr) {
	SPGenericEllipse::build(document, repr);

    this->readAttr( "cx" );
    this->readAttr( "cy" );
    this->readAttr( "rx" );
    this->readAttr( "ry" );
}


Inkscape::XML::Node* SPEllipse::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:ellipse");
    }

    sp_repr_set_svg_double(repr, "cx", this->cx.computed);
    sp_repr_set_svg_double(repr, "cy", this->cy.computed);
    sp_repr_set_svg_double(repr, "rx", this->rx.computed);
    sp_repr_set_svg_double(repr, "ry", this->ry.computed);

    SPGenericEllipse::write(xml_doc, repr, flags);

    return repr;
}


void SPEllipse::set(unsigned int key, gchar const* value) {
    switch (key) {
        case SP_ATTR_CX:
            this->cx.readOrUnset(value);
            this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
            
        case SP_ATTR_CY:
            this->cy.readOrUnset(value);
            this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
            
        case SP_ATTR_RX:
            if (!this->rx.read(value) || (this->rx.value <= 0.0)) {
                this->rx.unset();
            }
            
            this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
            
        case SP_ATTR_RY:
            if (!this->ry.read(value) || (this->ry.value <= 0.0)) {
                this->ry.unset();
            }
            
            this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
            
        default:
            SPGenericEllipse::set(key, value);
            break;
    }
}

const char* SPEllipse::displayName() {
    return _("Ellipse");
}


void
sp_ellipse_position_set(SPEllipse *ellipse, gdouble x, gdouble y, gdouble rx, gdouble ry)
{
    SPGenericEllipse *ge;

    g_return_if_fail(ellipse != NULL);
    g_return_if_fail(SP_IS_ELLIPSE(ellipse));

    ge = SP_GENERICELLIPSE(ellipse);

    ge->cx.computed = x;
    ge->cy.computed = y;
    ge->rx.computed = rx;
    ge->ry.computed = ry;

    ((SPObject *)ge)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

/* SVG <circle> element */
SPCircle::SPCircle() : SPGenericEllipse() {
}

SPCircle::~SPCircle() {
}

void SPCircle::build(SPDocument *document, Inkscape::XML::Node *repr) {
    SPGenericEllipse::build(document, repr);

    this->readAttr( "cx" );
    this->readAttr( "cy" );
    this->readAttr( "r" );
}


Inkscape::XML::Node* SPCircle::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:circle");
    }

    sp_repr_set_svg_double(repr, "cx", this->cx.computed);
    sp_repr_set_svg_double(repr, "cy", this->cy.computed);
    sp_repr_set_svg_double(repr, "r", this->rx.computed);

    SPGenericEllipse::write(xml_doc, repr, flags);

    return repr;
}

void SPCircle::set(unsigned int key, gchar const* value) {
    switch (key) {
        case SP_ATTR_CX:
            this->cx.readOrUnset(value);
            this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
            
        case SP_ATTR_CY:
            this->cy.readOrUnset(value);
            this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
            
        case SP_ATTR_R:
            if (!this->rx.read(value) || this->rx.value <= 0.0) {
                this->rx.unset();
            }
            
            this->ry = this->rx;
            this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;
            
        default:
            SPGenericEllipse::set(key, value);
            break;
    }
}

const char* SPCircle::displayName() {
	return _("Circle");
}

/* <path sodipodi:type="arc"> element */
SPArc::SPArc() : SPGenericEllipse() {
}

SPArc::~SPArc() {
}

void SPArc::build(SPDocument *document, Inkscape::XML::Node *repr) {
	SPGenericEllipse::build(document, repr);

    this->readAttr( "sodipodi:cx" );
    this->readAttr( "sodipodi:cy" );
    this->readAttr( "sodipodi:rx" );
    this->readAttr( "sodipodi:ry" );

    this->readAttr( "sodipodi:start" );
    this->readAttr( "sodipodi:end" );
    this->readAttr( "sodipodi:open" );
}

/*
 * sp_arc_set_elliptical_path_attribute:
 *
 * Convert center to endpoint parameterization and set it to repr.
 *
 * See SVG 1.0 Specification W3C Recommendation
 * ``F.6 Ellptical arc implementation notes'' for more detail.
 */
static gboolean
sp_arc_set_elliptical_path_attribute(SPArc *arc, Inkscape::XML::Node *repr)
{
    SPGenericEllipse *ge = SP_GENERICELLIPSE(arc);

    Inkscape::SVG::PathString str;

    Geom::Point p1 = sp_arc_get_xy(arc, ge->start);
    Geom::Point p2 = sp_arc_get_xy(arc, ge->end);
    double rx = ge->rx.computed;
    double ry = ge->ry.computed;

    str.moveTo(p1);

    double dt = fmod(ge->end - ge->start, SP_2PI);
    if (fabs(dt) < 1e-6) {
        Geom::Point ph = sp_arc_get_xy(arc, (ge->start + ge->end) / 2.0);
        str.arcTo(rx, ry, 0, true, true, ph)
           .arcTo(rx, ry, 0, true, true, p2)
           .closePath();
    } else {
        bool fa = (fabs(dt) > M_PI);
        bool fs = (dt > 0);
        str.arcTo(rx, ry, 0, fa, fs, p2);
        if (ge->closed) {
            Geom::Point center = Geom::Point(ge->cx.computed, ge->cy.computed);
            str.lineTo(center).closePath();
        }
    }

    repr->setAttribute("d", str.c_str());
    return true;
}

Inkscape::XML::Node* SPArc::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:path");
    }

    if (flags & SP_OBJECT_WRITE_EXT) {
        repr->setAttribute("sodipodi:type", "arc");

        sp_repr_set_svg_double(repr, "sodipodi:cx", this->cx.computed);
        sp_repr_set_svg_double(repr, "sodipodi:cy", this->cy.computed);
        sp_repr_set_svg_double(repr, "sodipodi:rx", this->rx.computed);
        sp_repr_set_svg_double(repr, "sodipodi:ry", this->ry.computed);

        // write start and end only if they are non-trivial; otherwise remove
        gdouble len = fmod(this->end - this->start, SP_2PI);

        if (len < 0.0) {
        	len += SP_2PI;
        }

        if (!(fabs(len) < 1e-8 || fabs(len - SP_2PI) < 1e-8)) {
            sp_repr_set_svg_double(repr, "sodipodi:start", this->start);
            sp_repr_set_svg_double(repr, "sodipodi:end", this->end);

            repr->setAttribute("sodipodi:open", (!this->closed) ? "true" : NULL);
        } else {
            repr->setAttribute("sodipodi:end", NULL);
            repr->setAttribute("sodipodi:start", NULL);
            repr->setAttribute("sodipodi:open", NULL);
        }
    }

    // write d=
    sp_arc_set_elliptical_path_attribute(this, repr);

    SPGenericEllipse::write(xml_doc, repr, flags);

    return repr;
}

void SPArc::set(unsigned int key, gchar const* value) {
    switch (key) {
        case SP_ATTR_SODIPODI_CX:
            this->cx.readOrUnset(value);
            this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_SODIPODI_CY:
            this->cy.readOrUnset(value);
            this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_SODIPODI_RX:
            if (!this->rx.read(value) || this->rx.computed <= 0.0) {
                this->rx.unset();
            }

            this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_SODIPODI_RY:
            if (!this->ry.read(value) || this->ry.computed <= 0.0) {
                this->ry.unset();
            }

            this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_SODIPODI_START:
            if (value) {
                sp_svg_number_read_d(value, &this->start);
            } else {
                this->start = 0;
            }

            this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_SODIPODI_END:
            if (value) {
                sp_svg_number_read_d(value, &this->end);
            } else {
                this->end = 2 * M_PI;
            }

            this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_SODIPODI_OPEN:
            this->closed = (!value);
            this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;

        default:
            SPGenericEllipse::set(key, value);
            break;
    }
}

void SPArc::modified(guint flags) {
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {
        this->set_shape();
    }

    SPGenericEllipse::modified(flags);
}


const char* SPArc::displayName() {
    gdouble len = fmod(this->end - this->start, SP_2PI);

    if (len < 0.0) {
    	len += SP_2PI;
    }

    if (!(fabs(len) < 1e-8 || fabs(len - SP_2PI) < 1e-8)) {
        if (this->closed) {
            return _("Segment");
        } else {
            return _("Arc");
        }
    } else {
        return _("Ellipse");
    }
}

void
sp_arc_position_set(SPArc *arc, gdouble x, gdouble y, gdouble rx, gdouble ry)
{
    g_return_if_fail(arc != NULL);
    g_return_if_fail(SP_IS_ARC(arc));

    SPGenericEllipse *ge = SP_GENERICELLIPSE(arc);

    ge->cx.computed = x;
    ge->cy.computed = y;
    ge->rx.computed = rx;
    ge->ry.computed = ry;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    // those pref values are in degrees, while we want radians
    if (prefs->getDouble("/tools/shapes/arc/start", 0.0) != 0)
        ge->start = prefs->getDouble("/tools/shapes/arc/start", 0.0) * M_PI / 180;
    if (prefs->getDouble("/tools/shapes/arc/end", 0.0) != 0)
        ge->end = prefs->getDouble("/tools/shapes/arc/end", 0.0) * M_PI / 180;
    if (!prefs->getBool("/tools/shapes/arc/open"))
        ge->closed = 1;
    else
        ge->closed = 0;

    ((SPObject *)arc)->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

Geom::Point sp_arc_get_xy(SPArc *arc, gdouble arg)
{
    SPGenericEllipse *ge = SP_GENERICELLIPSE(arc);

    return Geom::Point(ge->rx.computed * cos(arg) + ge->cx.computed,
                     ge->ry.computed * sin(arg) + ge->cy.computed);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
