/** \file
 * SVG \<svg\> implementation.
 */
/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
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

#include <string>
#include <2geom/transforms.h>

#include "attributes.h"
#include "print.h"
#include "document.h"
#include "inkscape-version.h"
#include "sp-defs.h"
#include "sp-root.h"
#include "display/drawing-group.h"
#include "svg/stringstream.h"
#include "svg/svg.h"
#include "xml/repr.h"

#include "sp-factory.h"

namespace {
SPObject *createRoot()
{
    return new SPRoot();
}

bool rootRegistered = SPFactory::instance().registerObject("svg:svg", createRoot);
}

SPRoot::SPRoot() : SPGroup()
{
    this->aspect_set = 0;
    this->aspect_align = 0;
    this->onload = NULL;
    this->aspect_clip = 0;

    static Inkscape::Version const zero_version(0, 0);

    sp_version_from_string(SVG_VERSION, &this->original.svg);
    this->version.svg = zero_version;
    this->original.svg = zero_version;
    this->version.inkscape = zero_version;
    this->original.inkscape = zero_version;

    this->x.unset();
    this->y.unset();
    this->width.unset(SVGLength::PERCENT, 1.0, 1.0);
    this->height.unset(SVGLength::PERCENT, 1.0, 1.0);

    this->viewBox_set = false;

    this->c2p.setIdentity();

    this->defs = NULL;
}

SPRoot::~SPRoot()
{
}

void SPRoot::build(SPDocument *document, Inkscape::XML::Node *repr)
{
    //XML Tree being used directly here while it shouldn't be.
    if (!this->getRepr()->attribute("version")) {
        repr->setAttribute("version", SVG_VERSION);
    }

    this->readAttr("version");
    this->readAttr("inkscape:version");
    /* It is important to parse these here, so objects will have viewport build-time */
    this->readAttr("x");
    this->readAttr("y");
    this->readAttr("width");
    this->readAttr("height");
    this->readAttr("viewBox");
    this->readAttr("preserveAspectRatio");
    this->readAttr("onload");

    SPGroup::build(document, repr);

    // Search for first <defs> node
    for (SPObject *o = this->firstChild() ; o ; o = o->getNext()) {
        if (SP_IS_DEFS(o)) {
            this->defs = SP_DEFS(o);
            break;
        }
    }

    // clear transform, if any was read in - SVG does not allow transform= on <svg>
    SP_ITEM(this)->transform = Geom::identity();
}

void SPRoot::release()
{
    this->defs = NULL;

    SPGroup::release();
}


void SPRoot::set(unsigned int key, const gchar *value)
{
    switch (key) {
    case SP_ATTR_VERSION:
        if (!sp_version_from_string(value, &this->version.svg)) {
            this->version.svg = this->original.svg;
        }
        break;

    case SP_ATTR_INKSCAPE_VERSION:
        if (!sp_version_from_string(value, &this->version.inkscape)) {
            this->version.inkscape = this->original.inkscape;
        }
        break;

    case SP_ATTR_X:
        if (!this->x.readAbsolute(value)) {
            /* fixme: em, ex, % are probably valid, but require special treatment (Lauris) */
            this->x.unset();
        }

        /* fixme: I am almost sure these do not require viewport flag (Lauris) */
        this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
        break;

    case SP_ATTR_Y:
        if (!this->y.readAbsolute(value)) {
            /* fixme: em, ex, % are probably valid, but require special treatment (Lauris) */
            this->y.unset();
        }

        /* fixme: I am almost sure these do not require viewport flag (Lauris) */
        this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
        break;

    case SP_ATTR_WIDTH:
        if (!this->width.readAbsolute(value) || !(this->width.computed > 0.0)) {
            /* fixme: em, ex, % are probably valid, but require special treatment (Lauris) */
            this->width.unset(SVGLength::PERCENT, 1.0, 1.0);
        }

        this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
        break;

    case SP_ATTR_HEIGHT:
        if (!this->height.readAbsolute(value) || !(this->height.computed > 0.0)) {
            /* fixme: em, ex, % are probably valid, but require special treatment (Lauris) */
            this->height.unset(SVGLength::PERCENT, 1.0, 1.0);
        }

        this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
        break;

    case SP_ATTR_VIEWBOX:
        if (value) {
            double x, y, width, height;
            char *eptr;

            /* fixme: We have to take original item affine into account */
            /* fixme: Think (Lauris) */
            eptr = (gchar *) value;
            x = g_ascii_strtod(eptr, &eptr);

            while (*eptr && ((*eptr == ',') || (*eptr == ' '))) {
                eptr++;
            }

            y = g_ascii_strtod(eptr, &eptr);

            while (*eptr && ((*eptr == ',') || (*eptr == ' '))) {
                eptr++;
            }

            width = g_ascii_strtod(eptr, &eptr);

            while (*eptr && ((*eptr == ',') || (*eptr == ' '))) {
                eptr++;
            }

            height = g_ascii_strtod(eptr, &eptr);

            while (*eptr && ((*eptr == ',') || (*eptr == ' '))) {
                eptr++;
            }

            if ((width > 0) && (height > 0)) {
                /* Set viewbox */
                this->viewBox = Geom::Rect::from_xywh(x, y, width, height);
                this->viewBox_set = TRUE;
            } else {
                this->viewBox_set = FALSE;
            }
        } else {
            this->viewBox_set = FALSE;
        }

        this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
        break;

    case SP_ATTR_PRESERVEASPECTRATIO:
        /* Do setup before, so we can use break to escape */
        this->aspect_set = FALSE;
        this->aspect_align = SP_ASPECT_XMID_YMID;
        this->aspect_clip = SP_ASPECT_MEET;

        this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);

        if (value) {
            int len;
            gchar c[256];
            gchar const *p, *e;
            unsigned int align, clip;
            p = value;

            while (*p && *p == 32) {
                p += 1;
            }

            if (!*p) {
                break;
            }

            e = p;

            while (*e && *e != 32) {
                e += 1;
            }

            len = e - p;

            if (len > 8) {
                break;
            }

            memcpy(c, value, len);

            c[len] = 0;

            /* Now the actual part */
            if (!strcmp(c, "none")) {
                align = SP_ASPECT_NONE;
            } else if (!strcmp(c, "xMinYMin")) {
                align = SP_ASPECT_XMIN_YMIN;
            } else if (!strcmp(c, "xMidYMin")) {
                align = SP_ASPECT_XMID_YMIN;
            } else if (!strcmp(c, "xMaxYMin")) {
                align = SP_ASPECT_XMAX_YMIN;
            } else if (!strcmp(c, "xMinYMid")) {
                align = SP_ASPECT_XMIN_YMID;
            } else if (!strcmp(c, "xMidYMid")) {
                align = SP_ASPECT_XMID_YMID;
            } else if (!strcmp(c, "xMaxYMid")) {
                align = SP_ASPECT_XMAX_YMID;
            } else if (!strcmp(c, "xMinYMax")) {
                align = SP_ASPECT_XMIN_YMAX;
            } else if (!strcmp(c, "xMidYMax")) {
                align = SP_ASPECT_XMID_YMAX;
            } else if (!strcmp(c, "xMaxYMax")) {
                align = SP_ASPECT_XMAX_YMAX;
            } else {
                break;
            }

            clip = SP_ASPECT_MEET;

            while (*e && *e == 32) {
                e += 1;
            }

            if (*e) {
                if (!strcmp(e, "meet")) {
                    clip = SP_ASPECT_MEET;
                } else if (!strcmp(e, "slice")) {
                    clip = SP_ASPECT_SLICE;
                } else {
                    break;
                }
            }

            this->aspect_set = TRUE;
            this->aspect_align = align;
            this->aspect_clip = clip;
        }
        break;

    case SP_ATTR_ONLOAD:
        this->onload = (char *) value;
        break;

    default:
        /* Pass the set event to the parent */
        SPGroup::set(key, value);
        break;
    }
}

void SPRoot::child_added(Inkscape::XML::Node *child, Inkscape::XML::Node *ref)
{
    SPGroup::child_added(child, ref);

    SPObject *co = this->document->getObjectByRepr(child);
    // NOTE: some XML nodes do not have corresponding SP objects,
    // for instance inkscape:clipboard used in the clipboard code.
    // See LP bug #1227827
    //g_assert (co != NULL || !strcmp("comment", child->name())); // comment repr node has no object

    if (co && SP_IS_DEFS(co)) {
        // We search for first <defs> node - it is not beautiful, but works
        for (SPObject *c = this->firstChild() ; c ; c = c->getNext()) {
            if (SP_IS_DEFS(c)) {
                this->defs = SP_DEFS(c);
                break;
            }
        }
    }
}

void SPRoot::remove_child(Inkscape::XML::Node *child)
{
    if (this->defs && (this->defs->getRepr() == child)) {
        SPObject *iter = 0;

        // We search for first remaining <defs> node - it is not beautiful, but works
        for (iter = this->firstChild() ; iter ; iter = iter->getNext()) {
            if (SP_IS_DEFS(iter) && (SPDefs *)iter != this->defs) {
                this->defs = (SPDefs *)iter;
                break;
            }
        }

        if (!iter) {
            /* we should probably create a new <defs> here? */
            this->defs = NULL;
        }
    }

    SPGroup::remove_child(child);
}

void SPRoot::update(SPCtx *ctx, guint flags)
{
    SPItemCtx *ictx = (SPItemCtx *) ctx;

    /* fixme: This will be invoked too often (Lauris) */
    /* fixme: We should calculate only if parent viewport has changed (Lauris) */
    /* If position is specified as percentage, calculate actual values */
    if (this->x.unit == SVGLength::PERCENT) {
        this->x.computed = this->x.value * ictx->viewport.width();
    }

    if (this->y.unit == SVGLength::PERCENT) {
        this->y.computed = this->y.value * ictx->viewport.height();
    }

    if (this->width.unit == SVGLength::PERCENT) {
        this->width.computed = this->width.value * ictx->viewport.width();
    }

    if (this->height.unit == SVGLength::PERCENT) {
        this->height.computed = this->height.value * ictx->viewport.height();
    }

    /* Create copy of item context */
    SPItemCtx rctx = *ictx;

    /* Calculate child to parent transformation */
    this->c2p.setIdentity();

    if (this->parent) {
        /*
         * fixme: I am not sure whether setting x and y does or does not
         * fixme: translate the content of inner SVG.
         * fixme: Still applying translation and setting viewport to width and
         * fixme: height seems natural, as this makes the inner svg element
         * fixme: self-contained. The spec is vague here.
         */
        this->c2p = Geom::Affine(Geom::Translate(this->x.computed, this->y.computed));
    }

    if (this->viewBox_set) {
        double x, y, width, height;
        /* Determine actual viewbox in viewport coordinates */
        if (this->aspect_align == SP_ASPECT_NONE) {
            x = 0.0;
            y = 0.0;
            width = this->width.computed;
            height = this->height.computed;
        } else {
            double scalex, scaley, scale;
            /* Things are getting interesting */
            scalex = this->width.computed / this->viewBox.width();
            scaley = this->height.computed / this->viewBox.height();
            scale = (this->aspect_clip == SP_ASPECT_MEET) ? MIN(scalex, scaley) : MAX(scalex, scaley);
            width = this->viewBox.width() * scale;
            height = this->viewBox.height() * scale;

            /* Now place viewbox to requested position */
            /* todo: Use an array lookup to find the 0.0/0.5/1.0 coefficients,
               as is done for dialogs/align.cpp. */
            switch (this->aspect_align) {
            case SP_ASPECT_XMIN_YMIN:
                x = 0.0;
                y = 0.0;
                break;

            case SP_ASPECT_XMID_YMIN:
                x = 0.5 * (this->width.computed - width);
                y = 0.0;
                break;

            case SP_ASPECT_XMAX_YMIN:
                x = 1.0 * (this->width.computed - width);
                y = 0.0;
                break;

            case SP_ASPECT_XMIN_YMID:
                x = 0.0;
                y = 0.5 * (this->height.computed - height);
                break;

            case SP_ASPECT_XMID_YMID:
                x = 0.5 * (this->width.computed - width);
                y = 0.5 * (this->height.computed - height);
                break;

            case SP_ASPECT_XMAX_YMID:
                x = 1.0 * (this->width.computed - width);
                y = 0.5 * (this->height.computed - height);
                break;

            case SP_ASPECT_XMIN_YMAX:
                x = 0.0;
                y = 1.0 * (this->height.computed - height);
                break;

            case SP_ASPECT_XMID_YMAX:
                x = 0.5 * (this->width.computed - width);
                y = 1.0 * (this->height.computed - height);
                break;

            case SP_ASPECT_XMAX_YMAX:
                x = 1.0 * (this->width.computed - width);
                y = 1.0 * (this->height.computed - height);
                break;

            default:
                x = 0.0;
                y = 0.0;
                break;
            }
        }

        /* Compose additional transformation from scale and position */
        Geom::Scale const viewBox_length(this->viewBox.dimensions());
        Geom::Scale const new_length(width, height);

        /* Append viewbox transformation */
        /* TODO: The below looks suspicious to me (pjrm): I wonder whether the RHS
           expression should have c2p at the beginning rather than at the end.  Test it. */
        this->c2p = Geom::Translate(-this->viewBox.min()) * (new_length * viewBox_length.inverse()) * Geom::Translate(x, y) * this->c2p;
    }

    rctx.i2doc = this->c2p * rctx.i2doc;

    /* Initialize child viewport */
    if (this->viewBox_set) {
        rctx.viewport = this->viewBox;
    } else {
        /* fixme: I wonder whether this logic is correct (Lauris) */
        Geom::Point minp(0, 0);
        if (this->parent) {
            minp = Geom::Point(this->x.computed, this->y.computed);
        }

        rctx.viewport = Geom::Rect::from_xywh(minp[Geom::X], minp[Geom::Y], this->width.computed, this->height.computed);
    }

    rctx.i2vp = Geom::identity();

    /* And invoke parent method */
    SPGroup::update((SPCtx *) &rctx, flags);

    /* As last step set additional transform of drawing group */
    for (SPItemView *v = this->display; v != NULL; v = v->next) {
        Inkscape::DrawingGroup *g = dynamic_cast<Inkscape::DrawingGroup *>(v->arenaitem);
        g->setChildTransform(this->c2p);
    }
}

void SPRoot::modified(unsigned int flags)
{
    SPGroup::modified(flags);

    /* fixme: (Lauris) */
    if (!this->parent && (flags & SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {
        this->document->emitResizedSignal(this->width.computed, this->height.computed);
    }
}


Inkscape::XML::Node *SPRoot::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:svg");
    }

    if (flags & SP_OBJECT_WRITE_EXT) {
        repr->setAttribute("inkscape:version", Inkscape::version_string);
    }

    if (!repr->attribute("version")) {
        gchar *myversion = sp_version_to_string(this->version.svg);
        repr->setAttribute("version", myversion);
        g_free(myversion);
    }

    if (fabs(this->x.computed) > 1e-9) {
        sp_repr_set_svg_double(repr, "x", this->x.computed);
    }

    if (fabs(this->y.computed) > 1e-9) {
        sp_repr_set_svg_double(repr, "y", this->y.computed);
    }

    /* Unlike all other SPObject, here we want to preserve absolute units too (and only here,
     * according to the recommendation in http://www.w3.org/TR/SVG11/coords.html#Units).
     */
    repr->setAttribute("width", sp_svg_length_write_with_units(this->width).c_str());
    repr->setAttribute("height", sp_svg_length_write_with_units(this->height).c_str());

    if (this->viewBox_set) {
        Inkscape::SVGOStringStream os;
        os << this->viewBox.left() << " " << this->viewBox.top() << " "
           << this->viewBox.width() << " " << this->viewBox.height();

        repr->setAttribute("viewBox", os.str().c_str());
    }

    SPGroup::write(xml_doc, repr, flags);

    return repr;
}

Inkscape::DrawingItem *SPRoot::show(Inkscape::Drawing &drawing, unsigned int key, unsigned int flags)
{
    Inkscape::DrawingItem *ai = 0;

    ai = SPGroup::show(drawing, key, flags);

    if (ai) {
        Inkscape::DrawingGroup *g = dynamic_cast<Inkscape::DrawingGroup *>(ai);
        g->setChildTransform(this->c2p);
    }

    return ai;
}

void SPRoot::print(SPPrintContext *ctx)
{
    sp_print_bind(ctx, this->c2p, 1.0);

    SPGroup::print(ctx);

    sp_print_release(ctx);
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
