/*
 * SVG <rect> implementation
 *
 * Authors:
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


#include "display/curve.h"
#include <2geom/rect.h>

#include "inkscape.h"
#include "document.h"
#include "attributes.h"
#include "style.h"
#include "sp-rect.h"
#include <glibmm/i18n.h>
#include "xml/repr.h"
#include "sp-guide.h"
#include "preferences.h"

#define noRECT_VERBOSE


SPRect::SPRect() : SPShape() {
}

SPRect::~SPRect() {
}

void SPRect::build(SPDocument* doc, Inkscape::XML::Node* repr) {
    SPShape::build(doc, repr);

    this->readAttr("x");
    this->readAttr("y");
    this->readAttr("width");
    this->readAttr("height");
    this->readAttr("rx");
    this->readAttr("ry");
}

void SPRect::set(unsigned key, gchar const *value) {
    /* fixme: We need real error processing some time */

    // We must update the SVGLengths immediately or nodes may be misplaced after they are moved.
    double const w = viewport.width();
    double const h = viewport.height();
    double const em = style->font_size.computed;
    double const ex = em * 0.5;

    switch (key) {
        case SP_ATTR_X:
            this->x.readOrUnset(value);
            this->x.update( em, ex, w );
            this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_Y:
            this->y.readOrUnset(value);
            this->y.update( em, ex, h );
            this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_WIDTH:
            if (!this->width.read(value) || this->width.value < 0.0) {
            	this->width.unset();
            }
            this->width.update( em, ex, w );
            this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_HEIGHT:
            if (!this->height.read(value) || this->height.value < 0.0) {
            	this->height.unset();
            }
            this->height.update( em, ex, h );
            this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_RX:
            if (!this->rx.read(value) || this->rx.value <= 0.0) {
            	this->rx.unset();
            }
            this->rx.update( em, ex, w );
            this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_RY:
            if (!this->ry.read(value) || this->ry.value <= 0.0) {
            	this->ry.unset();
            }
            this->ry.update( em, ex, h );
            this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            break;

        default:
            SPShape::set(key, value);
            break;
    }
}

void SPRect::update(SPCtx* ctx, unsigned int flags) {
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {
        SPItemCtx const *ictx = reinterpret_cast<SPItemCtx const *>(ctx);

        double const w = ictx->viewport.width();
        double const h = ictx->viewport.height();
        double const em = style->font_size.computed;
        double const ex = 0.5 * em;  // fixme: get x height from pango or libnrtype.

        this->x.update(em, ex, w);
        this->y.update(em, ex, h);
        this->width.update(em, ex, w);
        this->height.update(em, ex, h);
        this->rx.update(em, ex, w);
        this->ry.update(em, ex, h);
        this->set_shape();

        flags &= ~SP_OBJECT_USER_MODIFIED_FLAG_B; // since we change the description, it's not a "just translation" anymore
    }

    SPShape::update(ctx, flags);
}

Inkscape::XML::Node * SPRect::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:rect");
    }

    sp_repr_set_svg_length(repr, "width",  this->width);
    sp_repr_set_svg_length(repr, "height", this->height);

    if (this->rx._set) {
    	sp_repr_set_svg_length(repr, "rx", this->rx);
    }

    if (this->ry._set) {
    	sp_repr_set_svg_length(repr, "ry", this->ry);
    }

    sp_repr_set_svg_length(repr, "x", this->x);
    sp_repr_set_svg_length(repr, "y", this->y);

    this->set_shape(); // evaluate SPCurve
    SPShape::write(xml_doc, repr, flags);

    return repr;
}

const char* SPRect::displayName() const {
    return _("Rectangle");
}

#define C1 0.554

void SPRect::set_shape() {
    if ((this->height.computed < 1e-18) || (this->width.computed < 1e-18)) {
    	this->setCurveInsync( NULL, TRUE);
    	this->setCurveBeforeLPE( NULL );
        return;
    }

    SPCurve *c = new SPCurve();

    double const x = this->x.computed;
    double const y = this->y.computed;
    double const w = this->width.computed;
    double const h = this->height.computed;
    double const w2 = w / 2;
    double const h2 = h / 2;
    double const rx = std::min(( this->rx._set
                                 ? this->rx.computed
                                 : ( this->ry._set
                                     ? this->ry.computed
                                     : 0.0 ) ),
                               .5 * this->width.computed);
    double const ry = std::min(( this->ry._set
                                 ? this->ry.computed
                                 : ( this->rx._set
                                     ? this->rx.computed
                                     : 0.0 ) ),
                               .5 * this->height.computed);
    /* TODO: Handle negative rx or ry as per
     * http://www.w3.org/TR/SVG11/shapes.html#RectElementRXAttribute once Inkscape has proper error
     * handling (see http://www.w3.org/TR/SVG11/implnote.html#ErrorProcessing).
     */

    /* We don't use proper circular/elliptical arcs, but bezier curves can approximate a 90-degree
     * arc fairly well.
     */
    if ((rx > 1e-18) && (ry > 1e-18)) {
        c->moveto(x + rx, y);

        if (rx < w2) {
        	c->lineto(x + w - rx, y);
        }

        c->curveto(x + w - rx * (1 - C1), y, x + w, y + ry * (1 - C1), x + w, y + ry);

        if (ry < h2) {
        	c->lineto(x + w, y + h - ry);
        }

        c->curveto(x + w, y + h - ry * (1 - C1), x + w - rx * (1 - C1), y + h, x + w - rx, y + h);

        if (rx < w2) {
        	c->lineto(x + rx, y + h);
        }

        c->curveto(x + rx * (1 - C1), y + h, x, y + h - ry * (1 - C1), x, y + h - ry);

        if (ry < h2) {
        	c->lineto(x, y + ry);
        }

        c->curveto(x, y + ry * (1 - C1), x + rx * (1 - C1), y, x + rx, y);
    } else {
        c->moveto(x + 0.0, y + 0.0);
        c->lineto(x + w, y + 0.0);
        c->lineto(x + w, y + h);
        c->lineto(x + 0.0, y + h);
    }

    c->closepath();
    this->setCurveInsync(c, true);
    this->setCurveBeforeLPE(c);

    // LPE is not applied because result can generally not be represented as SPRect

    c->unref();
}

/* fixme: Think (Lauris) */

void SPRect::setPosition(gdouble x, gdouble y, gdouble width, gdouble height) {
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;

    this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

void SPRect::setRx(bool set, gdouble value) {
    this->rx._set = set;

    if (set) {
    	this->rx = value;
    }

    this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

void SPRect::setRy(bool set, gdouble value) {
    this->ry._set = set;

    if (set) {
    	this->ry = value;
    }

    this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

Geom::Affine SPRect::set_transform(Geom::Affine const& xform) {
    /* Calculate rect start in parent coords. */
    Geom::Point pos(Geom::Point(this->x.computed, this->y.computed) * xform);

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

    /* Preserve units */
    this->width.scale( sw );
    this->height.scale( sh );

    if (this->rx._set) {
    	this->rx.scale( sw );
    }

    if (this->ry._set) {
    	this->ry.scale( sh );
    }

    /* Find start in item coords */
    pos = pos * ret.inverse();
    this->x = pos[Geom::X];
    this->y = pos[Geom::Y];

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


/**
Returns the ratio in which the vector from p0 to p1 is stretched by transform
 */
gdouble SPRect::vectorStretch(Geom::Point p0, Geom::Point p1, Geom::Affine xform) {
    if (p0 == p1) {
        return 0;
    }

    return (Geom::distance(p0 * xform, p1 * xform) / Geom::distance(p0, p1));
}

void SPRect::setVisibleRx(gdouble rx) {
    if (rx == 0) {
        this->rx.unset();
    } else {
    	this->rx = rx / SPRect::vectorStretch(
            Geom::Point(this->x.computed + 1, this->y.computed),
            Geom::Point(this->x.computed, this->y.computed),
            this->i2doc_affine());
    }

    this->updateRepr();
}

void SPRect::setVisibleRy(gdouble ry) {
    if (ry == 0) {
    	this->ry.unset();
    } else {
    	this->ry = ry / SPRect::vectorStretch(
            Geom::Point(this->x.computed, this->y.computed + 1),
            Geom::Point(this->x.computed, this->y.computed),
            this->i2doc_affine());
    }

    this->updateRepr();
}

gdouble SPRect::getVisibleRx() const {
    if (!this->rx._set) {
        return 0;
    }

    return this->rx.computed * SPRect::vectorStretch(
        Geom::Point(this->x.computed + 1, this->y.computed),
        Geom::Point(this->x.computed, this->y.computed),
        this->i2doc_affine());
}

gdouble SPRect::getVisibleRy() const {
    if (!this->ry._set) {
        return 0;
    }

    return this->ry.computed * SPRect::vectorStretch(
        Geom::Point(this->x.computed, this->y.computed + 1),
        Geom::Point(this->x.computed, this->y.computed),
        this->i2doc_affine());
}

Geom::Rect SPRect::getRect() const {
    Geom::Point p0 = Geom::Point(this->x.computed, this->y.computed);
    Geom::Point p2 = Geom::Point(this->x.computed + this->width.computed, this->y.computed + this->height.computed);

    return Geom::Rect(p0, p2);
}

void SPRect::compensateRxRy(Geom::Affine xform) {
    if (this->rx.computed == 0 && this->ry.computed == 0) {
        return; // nothing to compensate
    }

    // test unit vectors to find out compensation:
    Geom::Point c(this->x.computed, this->y.computed);
    Geom::Point cx = c + Geom::Point(1, 0);
    Geom::Point cy = c + Geom::Point(0, 1);

    // apply previous transform if any
    c *= this->transform;
    cx *= this->transform;
    cy *= this->transform;

    // find out stretches that we need to compensate
    gdouble eX = SPRect::vectorStretch(cx, c, xform);
    gdouble eY = SPRect::vectorStretch(cy, c, xform);

    // If only one of the radii is set, set both radii so they have the same visible length
    // This is needed because if we just set them the same length in SVG, they might end up unequal because of transform
    if ((this->rx._set && !this->ry._set) || (this->ry._set && !this->rx._set)) {
        gdouble r = MAX(this->rx.computed, this->ry.computed);
        this->rx = r / eX;
        this->ry = r / eY;
    } else {
    	this->rx = this->rx.computed / eX;
    	this->ry = this->ry.computed / eY;
    }

    // Note that a radius may end up larger than half-side if the rect is scaled down;
    // that's ok because this preserves the intended radii in case the rect is enlarged again,
    // and set_shape will take care of trimming too large radii when generating d=
}

void SPRect::setVisibleWidth(gdouble width) {
    this->width = width / SPRect::vectorStretch(
        Geom::Point(this->x.computed + 1, this->y.computed),
        Geom::Point(this->x.computed, this->y.computed),
        this->i2doc_affine());

    this->updateRepr();
}

void SPRect::setVisibleHeight(gdouble height) {
    this->height = height / SPRect::vectorStretch(
        Geom::Point(this->x.computed, this->y.computed + 1),
        Geom::Point(this->x.computed, this->y.computed),
        this->i2doc_affine());

	this->updateRepr();
}

gdouble SPRect::getVisibleWidth() const {
    if (!this->width._set) {
        return 0;
    }

    return this->width.computed * SPRect::vectorStretch(
        Geom::Point(this->x.computed + 1, this->y.computed),
        Geom::Point(this->x.computed, this->y.computed),
        this->i2doc_affine());
}

gdouble SPRect::getVisibleHeight() const {
    if (!this->height._set) {
        return 0;
    }

    return this->height.computed * SPRect::vectorStretch(
        Geom::Point(this->x.computed, this->y.computed + 1),
        Geom::Point(this->x.computed, this->y.computed),
        this->i2doc_affine());
}

void SPRect::snappoints(std::vector<Inkscape::SnapCandidatePoint> &p, Inkscape::SnapPreferences const *snapprefs) const {
    /* This method overrides sp_shape_snappoints, which is the default for any shape. The default method
    returns all eight points along the path of a rounded rectangle, but not the real corners. Snapping
    the startpoint and endpoint of each rounded corner is not very useful and really confusing. Instead
    we could snap either the real corners, or not snap at all. Bulia Byak opted to snap the real corners,
    but it should be noted that this might be confusing in some cases with relatively large radii. With
    small radii though the user will easily understand which point is snapping. */

    Geom::Affine const i2dt (this->i2dt_affine ());

    Geom::Point p0 = Geom::Point(this->x.computed, this->y.computed) * i2dt;
    Geom::Point p1 = Geom::Point(this->x.computed, this->y.computed + this->height.computed) * i2dt;
    Geom::Point p2 = Geom::Point(this->x.computed + this->width.computed, this->y.computed + this->height.computed) * i2dt;
    Geom::Point p3 = Geom::Point(this->x.computed + this->width.computed, this->y.computed) * i2dt;

    if (snapprefs->isTargetSnappable(Inkscape::SNAPTARGET_RECT_CORNER)) {
        p.push_back(Inkscape::SnapCandidatePoint(p0, Inkscape::SNAPSOURCE_RECT_CORNER, Inkscape::SNAPTARGET_RECT_CORNER));
        p.push_back(Inkscape::SnapCandidatePoint(p1, Inkscape::SNAPSOURCE_RECT_CORNER, Inkscape::SNAPTARGET_RECT_CORNER));
        p.push_back(Inkscape::SnapCandidatePoint(p2, Inkscape::SNAPSOURCE_RECT_CORNER, Inkscape::SNAPTARGET_RECT_CORNER));
        p.push_back(Inkscape::SnapCandidatePoint(p3, Inkscape::SNAPSOURCE_RECT_CORNER, Inkscape::SNAPTARGET_RECT_CORNER));
    }

    if (snapprefs->isTargetSnappable(Inkscape::SNAPTARGET_LINE_MIDPOINT)) {
        p.push_back(Inkscape::SnapCandidatePoint((p0 + p1)/2, Inkscape::SNAPSOURCE_LINE_MIDPOINT, Inkscape::SNAPTARGET_LINE_MIDPOINT));
        p.push_back(Inkscape::SnapCandidatePoint((p1 + p2)/2, Inkscape::SNAPSOURCE_LINE_MIDPOINT, Inkscape::SNAPTARGET_LINE_MIDPOINT));
        p.push_back(Inkscape::SnapCandidatePoint((p2 + p3)/2, Inkscape::SNAPSOURCE_LINE_MIDPOINT, Inkscape::SNAPTARGET_LINE_MIDPOINT));
        p.push_back(Inkscape::SnapCandidatePoint((p3 + p0)/2, Inkscape::SNAPSOURCE_LINE_MIDPOINT, Inkscape::SNAPTARGET_LINE_MIDPOINT));
    }

    if (snapprefs->isTargetSnappable(Inkscape::SNAPTARGET_OBJECT_MIDPOINT)) {
        p.push_back(Inkscape::SnapCandidatePoint((p0 + p2)/2, Inkscape::SNAPSOURCE_OBJECT_MIDPOINT, Inkscape::SNAPTARGET_OBJECT_MIDPOINT));
    }
}

void SPRect::convert_to_guides() const {
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    if (!prefs->getBool("/tools/shapes/rect/convertguides", true)) {
    	// Use bounding box instead of edges
    	SPShape::convert_to_guides();
        return;
    }

    std::list<std::pair<Geom::Point, Geom::Point> > pts;

    Geom::Affine const i2dt(this->i2dt_affine());

    Geom::Point A1(Geom::Point(this->x.computed, this->y.computed) * i2dt);
    Geom::Point A2(Geom::Point(this->x.computed, this->y.computed + this->height.computed) * i2dt);
    Geom::Point A3(Geom::Point(this->x.computed + this->width.computed, this->y.computed + this->height.computed) * i2dt);
    Geom::Point A4(Geom::Point(this->x.computed + this->width.computed, this->y.computed) * i2dt);

    pts.push_back(std::make_pair(A1, A2));
    pts.push_back(std::make_pair(A2, A3));
    pts.push_back(std::make_pair(A3, A4));
    pts.push_back(std::make_pair(A4, A1));

    sp_guide_pt_pairs_to_guides(this->document, pts);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
