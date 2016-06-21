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
#include "util/units.h"

SPRoot::SPRoot() : SPGroup(), SPViewBox()
{
    this->onload = NULL;

    static Inkscape::Version const zero_version(0, 0);

    sp_version_from_string(SVG_VERSION, &this->original.svg);
    this->version.svg = zero_version;
    this->original.svg = zero_version;
    this->version.inkscape = zero_version;
    this->original.inkscape = zero_version;

    this->x.unset(SVGLength::PERCENT, 0.0, 0.0); // Ignored for root SVG element
    this->y.unset(SVGLength::PERCENT, 0.0, 0.0);
    this->width.unset(SVGLength::PERCENT, 1.0, 1.0);
    this->height.unset(SVGLength::PERCENT, 1.0, 1.0);

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
        /* Valid for non-root SVG elements; ex, em not handled correctly. */
        if (!this->x.read(value)) {
            this->x.unset(SVGLength::PERCENT, 0.0, 0.0);
        }

        /* fixme: I am almost sure these do not require viewport flag (Lauris) */
        this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
        break;

    case SP_ATTR_Y:
        /* Valid for non-root SVG elements; ex, em not handled correctly. */
        if (!this->y.read(value)) {
            this->y.unset(SVGLength::PERCENT, 0.0, 0.0);
        }

        /* fixme: I am almost sure these do not require viewport flag (Lauris) */
        this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
        break;

    case SP_ATTR_WIDTH:
        if (!this->width.read(value) || !(this->width.computed > 0.0)) {
            this->width.unset(SVGLength::PERCENT, 1.0, 1.0);
        }
        this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
        break;

    case SP_ATTR_HEIGHT:
        if (!this->height.read(value) || !(this->height.computed > 0.0)) {
            this->height.unset(SVGLength::PERCENT, 1.0, 1.0);
        }
        this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
        break;

    case SP_ATTR_VIEWBOX:
        set_viewBox( value );
        this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
        break;

    case SP_ATTR_PRESERVEASPECTRATIO:
        set_preserveAspectRatio( value );
        this->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_VIEWPORT_MODIFIED_FLAG);
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
    SPItemCtx const *ictx = (SPItemCtx const *) ctx;

    if( !this->parent ) {

        /*
         * This is the root SVG element:
         *
         * x, y, width, and height apply to positioning the SVG element inside a parent.
         * For the root SVG in Inkscape there is no parent, thus special rules apply:
         *   If width, height not set, width = 100%, height = 100% (as always).
         *   If width and height are in percent, they are percent of viewBox width/height.
         *   If width, height, and viewBox are not set... pick "random" width/height.
         *   x, y are ignored.
         *   initial viewport = (0 0 width height)
         */
        if( this->viewBox_set ) {

            if( this->width._set ) {
                // Check if this is necessary
                if (this->width.unit == SVGLength::PERCENT) {
                    this->width.computed  = this->width.value  * this->viewBox.width();
                }
            } else {
                this->width.set( SVGLength::PX, this->viewBox.width(),  this->viewBox.width()  );
            }

            if( this->height._set ) {
                if (this->height.unit == SVGLength::PERCENT) {
                    this->height.computed = this->height.value * this->viewBox.height();
                }
            } else {
                this->height.set(SVGLength::PX, this->viewBox.height(), this->viewBox.height() );
            }

        } else {

            if( !this->width._set ) {
                this->width.set(  SVGLength::PX, 100,  100 ); // Random default
            }

            if( !this->height._set ) {
                this->height.set( SVGLength::PX, 100,  100 ); // Random default
            }
        }

        // Ignore x, y values for root element
        this->x.unset(SVGLength::PERCENT, 0.0, 0.0);
        this->y.unset(SVGLength::PERCENT, 0.0, 0.0);
    }

    // Calculate x, y, width, height from parent/initial viewport
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

    // std::cout << "SPRoot::update: final:"
    //           << " x: " << x.computed
    //           << " y: " << y.computed
    //           << " width: " << width.computed
    //           << " height: " << height.computed << std::endl;

    // Calculate new viewport
    SPItemCtx rctx = *ictx;
    rctx.viewport = Geom::Rect::from_xywh( this->x.computed, this->y.computed,
                                           this->width.computed, this->height.computed );
    rctx = get_rctx( &rctx, Inkscape::Util::Quantity::convert(1, this->document->getDisplayUnit(), "px") );

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

    /* Only update version string on successful write to file. This is handled by 'file_save()'.
     * if (flags & SP_OBJECT_WRITE_EXT) {
     *   repr->setAttribute("inkscape:version", Inkscape::version_string);
     * }
     */

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

    // Uncomment to print out XML tree
    // getRepr()->recursivePrintTree(0);
    
    // Uncomment to print out SP Object tree
    // recursivePrintTree(0);
    
    // Uncomment to print out Display Item tree
    // ai->recursivePrintTree(0);

    return ai;
}

void SPRoot::print(SPPrintContext *ctx)
{
    sp_print_bind(ctx, this->c2p, 1.0);

    SPGroup::print(ctx);

    sp_print_release(ctx);
}

const char *SPRoot::displayName() const {
    return "SVG";  // Do not translate
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
