/*
 * SVG <symbol> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Abhishek Sharma
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 1999-2003 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cstring>
#include <string>

#include <2geom/transforms.h>
#include "display/drawing-group.h"
#include "xml/repr.h"
#include "attributes.h"
#include "print.h"
#include "sp-symbol.h"
#include "document.h"

#include "sp-factory.h"

namespace {
	SPObject* createSymbol() {
		return new SPSymbol();
	}

	bool symbolRegistered = SPFactory::instance().registerObject("svg:symbol", createSymbol);
}

SPSymbol::SPSymbol() : SPGroup() {
	this->aspect_align = 0;
	this->aspect_clip = 0;
	this->aspect_set = 0;

    this->viewBox_set = FALSE;
    this->c2p = Geom::identity();
}

SPSymbol::~SPSymbol() {
}

void SPSymbol::build(SPDocument *document, Inkscape::XML::Node *repr) {
    this->readAttr( "viewBox" );
    this->readAttr( "preserveAspectRatio" );

    SPGroup::build(document, repr);
}

void SPSymbol::release() {
	SPGroup::release();
}

void SPSymbol::set(unsigned int key, const gchar* value) {
    switch (key) {
    case SP_ATTR_VIEWBOX:
        if (value) {
            double x, y, width, height;
            char *eptr;

            /* fixme: We have to take original item affine into account */
            /* fixme: Think (Lauris) */
            eptr = (gchar *) value;
            x = g_ascii_strtod (eptr, &eptr);

            while (*eptr && ((*eptr == ',') || (*eptr == ' '))) {
            	eptr++;
            }

            y = g_ascii_strtod (eptr, &eptr);

            while (*eptr && ((*eptr == ',') || (*eptr == ' '))) {
            	eptr++;
            }

            width = g_ascii_strtod (eptr, &eptr);

            while (*eptr && ((*eptr == ',') || (*eptr == ' '))) {
            	eptr++;
            }

            height = g_ascii_strtod (eptr, &eptr);

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
        this->aspect_align = SP_ASPECT_NONE;
        this->aspect_clip = SP_ASPECT_MEET;
        this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);

        if (value) {
            int len;
            gchar c[256];
            const gchar *p, *e;
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

            memcpy (c, value, len);

            c[len] = 0;

            /* Now the actual part */
            if (!strcmp (c, "none")) {
                align = SP_ASPECT_NONE;
            } else if (!strcmp (c, "xMinYMin")) {
                align = SP_ASPECT_XMIN_YMIN;
            } else if (!strcmp (c, "xMidYMin")) {
                align = SP_ASPECT_XMID_YMIN;
            } else if (!strcmp (c, "xMaxYMin")) {
                align = SP_ASPECT_XMAX_YMIN;
            } else if (!strcmp (c, "xMinYMid")) {
                align = SP_ASPECT_XMIN_YMID;
            } else if (!strcmp (c, "xMidYMid")) {
                align = SP_ASPECT_XMID_YMID;
            } else if (!strcmp (c, "xMaxYMid")) {
                align = SP_ASPECT_XMAX_YMID;
            } else if (!strcmp (c, "xMinYMax")) {
                align = SP_ASPECT_XMIN_YMAX;
            } else if (!strcmp (c, "xMidYMax")) {
                align = SP_ASPECT_XMID_YMAX;
            } else if (!strcmp (c, "xMaxYMax")) {
                align = SP_ASPECT_XMAX_YMAX;
            } else {
                break;
            }

            clip = SP_ASPECT_MEET;

            while (*e && *e == 32) {
            	e += 1;
            }

            if (*e) {
                if (!strcmp (e, "meet")) {
                    clip = SP_ASPECT_MEET;
                } else if (!strcmp (e, "slice")) {
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

    default:
        SPGroup::set(key, value);
        break;
    }
}

void SPSymbol::child_added(Inkscape::XML::Node *child, Inkscape::XML::Node *ref) {
	SPGroup::child_added(child, ref);
}


void SPSymbol::update(SPCtx *ctx, guint flags) {
    SPItemCtx *ictx = (SPItemCtx *) ctx;
    SPItemCtx rctx;

    if (this->cloned) {
        /* Cloned <symbol> is actually renderable */

        /* fixme: We have to set up clip here too */

        /* Create copy of item context */
        rctx = *ictx;

        /* Calculate child to parent transformation */
        /* Apply parent <use> translation (set up as vewport) */
        this->c2p = Geom::Translate(rctx.viewport.min());

        if (this->viewBox_set) {
            double x, y, width, height;

            /* Determine actual viewbox in viewport coordinates */
            if (this->aspect_align == SP_ASPECT_NONE) {
                x = 0.0;
                y = 0.0;
                width = rctx.viewport.width();
                height = rctx.viewport.height();
            } else {
                double scalex, scaley, scale;
                /* Things are getting interesting */
                scalex = rctx.viewport.width() / this->viewBox.width();
                scaley = rctx.viewport.height() / this->viewBox.height();
                scale = (this->aspect_clip == SP_ASPECT_MEET) ? MIN (scalex, scaley) : MAX (scalex, scaley);
                width = this->viewBox.width() * scale;
                height = this->viewBox.height() * scale;

                /* Now place viewbox to requested position */
                switch (this->aspect_align) {
                case SP_ASPECT_XMIN_YMIN:
                    x = 0.0;
                    y = 0.0;
                    break;
                case SP_ASPECT_XMID_YMIN:
                    x = 0.5 * (rctx.viewport.width() - width);
                    y = 0.0;
                    break;
                case SP_ASPECT_XMAX_YMIN:
                    x = 1.0 * (rctx.viewport.width() - width);
                    y = 0.0;
                    break;
                case SP_ASPECT_XMIN_YMID:
                    x = 0.0;
                    y = 0.5 * (rctx.viewport.height() - height);
                    break;
                case SP_ASPECT_XMID_YMID:
                    x = 0.5 * (rctx.viewport.width() - width);
                    y = 0.5 * (rctx.viewport.height() - height);
                    break;
                case SP_ASPECT_XMAX_YMID:
                    x = 1.0 * (rctx.viewport.width() - width);
                    y = 0.5 * (rctx.viewport.height() - height);
                    break;
                case SP_ASPECT_XMIN_YMAX:
                    x = 0.0;
                    y = 1.0 * (rctx.viewport.height() - height);
                    break;
                case SP_ASPECT_XMID_YMAX:
                    x = 0.5 * (rctx.viewport.width() - width);
                    y = 1.0 * (rctx.viewport.height() - height);
                    break;
                case SP_ASPECT_XMAX_YMAX:
                    x = 1.0 * (rctx.viewport.width() - width);
                    y = 1.0 * (rctx.viewport.height() - height);
                    break;
                default:
                    x = 0.0;
                    y = 0.0;
                    break;
                }
            }

            /* Compose additional transformation from scale and position */
            Geom::Affine q;
            q[0] = width / this->viewBox.width();
            q[1] = 0.0;
            q[2] = 0.0;
            q[3] = height / this->viewBox.height();
            q[4] = -this->viewBox.left() * q[0] + x;
            q[5] = -this->viewBox.top() * q[3] + y;

            /* Append viewbox transformation */
            this->c2p = q * this->c2p;
        }

        rctx.i2doc = this->c2p * (Geom::Affine)rctx.i2doc;

        /* If viewBox is set initialize child viewport */
        /* Otherwise <use> has set it up already */
        if (this->viewBox_set) {
            rctx.viewport = this->viewBox;
            rctx.i2vp = Geom::identity();
        }

        // And invoke parent method
        SPGroup::update((SPCtx *) &rctx, flags);

        // As last step set additional transform of drawing group
        for (SPItemView *v = this->display; v != NULL; v = v->next) {
        	Inkscape::DrawingGroup *g = dynamic_cast<Inkscape::DrawingGroup *>(v->arenaitem);
        	g->setChildTransform(this->c2p);
        }
    } else {
        // No-op
        SPGroup::update(ctx, flags);
    }
}

void SPSymbol::modified(unsigned int flags) {
	SPGroup::modified(flags);
}


Inkscape::XML::Node* SPSymbol::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:symbol");
    }

    //XML Tree being used directly here while it shouldn't be.
    repr->setAttribute("viewBox", this->getRepr()->attribute("viewBox"));
	
    //XML Tree being used directly here while it shouldn't be.
    repr->setAttribute("preserveAspectRatio", this->getRepr()->attribute("preserveAspectRatio"));

    SPGroup::write(xml_doc, repr, flags);

    return repr;
}

Inkscape::DrawingItem* SPSymbol::show(Inkscape::Drawing &drawing, unsigned int key, unsigned int flags) {
    Inkscape::DrawingItem *ai = 0;

    if (this->cloned) {
        // Cloned <symbol> is actually renderable
        ai = SPGroup::show(drawing, key, flags);
        Inkscape::DrawingGroup *g = dynamic_cast<Inkscape::DrawingGroup *>(ai);

		if (g) {
			g->setChildTransform(this->c2p);
		}
    }

    return ai;
}

void SPSymbol::hide(unsigned int key) {
    if (this->cloned) {
        /* Cloned <symbol> is actually renderable */
        SPGroup::hide(key);
    }
}


Geom::OptRect SPSymbol::bbox(Geom::Affine const &transform, SPItem::BBoxType type) const {
    Geom::OptRect bbox;

    // We don't need a bounding box for Symbols dialog when selecting
    // symbols. They have no canvas location. But cloned symbols are.
    if (this->cloned) {
    	Geom::Affine const a( this->c2p * transform );
    	bbox = SPGroup::bbox(a, type);
    }

    return bbox;
}

void SPSymbol::print(SPPrintContext* ctx) {
    if (this->cloned) {
        // Cloned <symbol> is actually renderable

        sp_print_bind(ctx, this->c2p, 1.0);

        SPGroup::print(ctx);

        sp_print_release (ctx);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
