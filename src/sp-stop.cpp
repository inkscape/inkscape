/** @file
 * @gradient stop class.
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 1999,2005 authors
 * Copyright (C) 2010 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include "sp-stop.h"
#include "style.h"

#include "attributes.h"
#include "streq.h"
#include "svg/svg.h"
#include "svg/svg-color.h"
#include "svg/css-ostringstream.h"
#include "xml/repr.h"

SPStop::SPStop() : SPObject() {
	this->path_string = NULL;

    this->offset = 0.0;
    this->currentColor = false;
    this->specified_color.set( 0x000000ff );
    this->opacity = 1.0;
}

SPStop::~SPStop() {
}

void SPStop::build(SPDocument* doc, Inkscape::XML::Node* repr) {
    SPObject::build(doc, repr);

    this->readAttr( "offset" );
    this->readAttr( "stop-color" );
    this->readAttr( "stop-opacity" );
    this->readAttr( "style" );
    this->readAttr( "path" ); // For mesh
}

/**
 * Virtual build: set stop attributes from its associated XML node.
 */

void SPStop::set(unsigned int key, const gchar* value) {
    switch (key) {
        case SP_ATTR_STYLE: {
        /** \todo
         * fixme: We are reading simple values 3 times during build (Lauris).
         * \par
         * We need presentation attributes etc.
         * \par
         * remove the hackish "style reading" from here: see comments in
         * sp_object_get_style_property about the bugs in our current
         * approach.  However, note that SPStyle doesn't currently have
         * stop-color and stop-opacity properties.
         */
            {
                gchar const *p = this->getStyleProperty( "stop-color", "black");
                if (streq(p, "currentColor")) {
                    this->currentColor = true;
                } else {
                    this->specified_color = SPStop::readStopColor( p );
                }
            }
            {
                gchar const *p = this->getStyleProperty( "stop-opacity", "1");
                gdouble opacity = sp_svg_read_percentage(p, this->opacity);
                this->opacity = opacity;
            }
            this->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
            break;
        }
        case SP_PROP_STOP_COLOR: {
            {
                gchar const *p = this->getStyleProperty( "stop-color", "black");
                if (streq(p, "currentColor")) {
                    this->currentColor = true;
                } else {
                    this->currentColor = false;
                    this->specified_color = SPStop::readStopColor( p );
                }
            }
            this->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
            break;
        }
        case SP_PROP_STOP_OPACITY: {
            {
                gchar const *p = this->getStyleProperty( "stop-opacity", "1");
                gdouble opacity = sp_svg_read_percentage(p, this->opacity);
                this->opacity = opacity;
            }
            this->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
            break;
        }
        case SP_ATTR_OFFSET: {
            this->offset = sp_svg_read_percentage(value, 0.0);
            this->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
            break;
        }
        case SP_PROP_STOP_PATH: {
            if (value) {
                this->path_string = new Glib::ustring( value );
                //Geom::PathVector pv = sp_svg_read_pathv(value);
                //SPCurve *curve = new SPCurve(pv);
                //if( curve ) {
                    // std::cout << "Got Curve" << std::endl;
                    //curve->unref();
                //}
            }
            break;
        }
        default: {
            SPObject::set(key, value);
            break;
        }
    }
}

/**
 * Virtual set: set attribute to value.
 */

Inkscape::XML::Node* SPStop::write(Inkscape::XML::Document* xml_doc, Inkscape::XML::Node* repr, guint flags) {
    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = xml_doc->createElement("svg:stop");
    }

    Glib::ustring colorStr = this->specified_color.toString();
    gfloat opacity = this->opacity;

    SPObject::write(xml_doc, repr, flags);

    // Since we do a hackish style setting here (because SPStyle does not support stop-color and
    // stop-opacity), we must do it AFTER calling the parent write method; otherwise
    // sp_object_write would clear our style= attribute (bug 1695287)

    Inkscape::CSSOStringStream os;
    os << "stop-color:";
    if (this->currentColor) {
        os << "currentColor";
    } else {
        os << colorStr;
    }
    os << ";stop-opacity:" << opacity;
    repr->setAttribute("style", os.str().c_str());
    repr->setAttribute("stop-color", NULL);
    repr->setAttribute("stop-opacity", NULL);
    sp_repr_set_css_double(repr, "offset", this->offset);
    /* strictly speaking, offset an SVG <number> rather than a CSS one, but exponents make no sense
     * for offset proportions. */

    return repr;
}

/**
 * Virtual write: write object attributes to repr.
 */

// A stop might have some non-stop siblings
SPStop* SPStop::getNextStop() {
    SPStop *result = 0;

    for (SPObject* obj = getNext(); obj && !result; obj = obj->getNext()) {
        if (SP_IS_STOP(obj)) {
            result = SP_STOP(obj);
        }
    }

    return result;
}

SPStop* SPStop::getPrevStop() {
    SPStop *result = 0;

    for (SPObject* obj = getPrev(); obj; obj = obj->getPrev()) {
        // The closest previous SPObject that is an SPStop *should* be ourself.
        if (SP_IS_STOP(obj)) {
            SPStop* stop = SP_STOP(obj);
            // Sanity check to ensure we have a proper sibling structure.
            if (stop->getNextStop() == this) {
                result = stop;
            } else {
                g_warning("SPStop previous/next relationship broken");
            }
            break;
        }
    }

    return result;
}

SPColor SPStop::readStopColor(Glib::ustring const &styleStr, guint32 dfl) {
    SPColor color(dfl);
    SPIPaint paint;

    paint.read( styleStr.c_str() );

    if ( paint.isColor() ) {
        color = paint.value.color;
    }

    return color;
}

SPColor SPStop::getEffectiveColor() const {
    SPColor ret;

    if (currentColor) {
        char const *str = getStyleProperty("color", NULL);
        /* Default value: arbitrarily black.  (SVG1.1 and CSS2 both say that the initial
         * value depends on user agent, and don't give any further restrictions that I can
         * see.) */
        ret = readStopColor( str, 0 );
    } else {
        ret = specified_color;
    }

    return ret;
}

/**
 * Return stop's color as 32bit value.
 */
guint32 SPStop::get_rgba32() const {
    guint32 rgb0 = 0;

    /* Default value: arbitrarily black.  (SVG1.1 and CSS2 both say that the initial
     * value depends on user agent, and don't give any further restrictions that I can
     * see.) */
    if (this->currentColor) {
        char const *str = this->getStyleProperty("color", NULL);

        if (str) {
            rgb0 = sp_svg_read_color(str, rgb0);
        }

        unsigned const alpha = static_cast<unsigned>(this->opacity * 0xff + 0.5);

        g_return_val_if_fail((alpha & ~0xff) == 0, rgb0 | 0xff);

        return rgb0 | alpha;
    } else {
        return this->specified_color.toRGBA32(this->opacity);
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
