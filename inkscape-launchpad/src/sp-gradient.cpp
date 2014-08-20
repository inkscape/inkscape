/** \file
 * SPGradient, SPStop, SPLinearGradient, SPRadialGradient,
 * SPMeshGradient, SPMeshRow, SPMeshPatch
 */
/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Jasper van de Gronde <th.v.d.gronde@hccnet.nl>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *   Tavmjong Bah <tavmjong@free.fr>
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2004 David Turner
 * Copyright (C) 2009 Jasper van de Gronde
 * Copyright (C) 2011 Tavmjong Bah
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 *
 */

#define noSP_GRADIENT_VERBOSE

#include <cstring>
#include <string>

#include <2geom/transforms.h>

#include <cairo.h>

#include <sigc++/functors/ptr_fun.h>
#include <sigc++/adaptors/bind.h>

#include "display/cairo-utils.h"
#include "svg/svg.h"
#include "svg/svg-color.h"
#include "svg/css-ostringstream.h"
#include "attributes.h"
#include "document-private.h"
#include "sp-gradient.h"
#include "gradient-chemistry.h"
#include "sp-gradient-reference.h"
#include "sp-linear-gradient.h"
#include "sp-radial-gradient.h"
#include "sp-mesh-gradient.h"
#include "sp-mesh-row.h"
#include "sp-mesh-patch.h"
#include "sp-stop.h"
#include "streq.h"
#include "uri.h"
#include "xml/repr.h"
#include "style.h"
#include "display/grayscale.h"

#define SP_MACROS_SILENT
#include "macros.h"

/// Has to be power of 2   Seems to be unused.
//#define NCOLORS NR_GRADIENT_VECTOR_LENGTH

bool SPGradient::hasStops() const
{
    return has_stops;
}

bool SPGradient::hasPatches() const
{
    return has_patches;
}

bool SPGradient::isUnitsSet() const
{
    return units_set;
}

SPGradientUnits SPGradient::getUnits() const
{
    return units;
}

bool SPGradient::isSpreadSet() const
{
    return spread_set;
}

SPGradientSpread SPGradient::getSpread() const
{
    return spread;
}

void SPGradient::setSwatch( bool swatch )
{
    if ( swatch != isSwatch() ) {
        this->swatch = swatch; // to make isSolid() work, this happens first
        gchar const* paintVal = swatch ? (isSolid() ? "solid" : "gradient") : 0;
        setAttribute( "osb:paint", paintVal, 0 );

        requestModified( SP_OBJECT_MODIFIED_FLAG );
    }
}


/**
 * return true if this gradient is "equivalent" to that gradient.
 * Equivalent meaning they have the same stop count, same stop colors and same stop opacity
 * @param that - A gradient to compare this to
 */
gboolean SPGradient::isEquivalent(SPGradient *that)
{
    //TODO Make this work for mesh gradients

    bool status = FALSE;
    
    while(1){ // not really a loop, used to avoid deep nesting or multiple exit points from function
        if (this->getStopCount() != that->getStopCount()) { break; }
        if (this->hasStops() != that->hasStops()) { break; }
        if (!this->getVector() || !that->getVector()) { break; }
        if (this->isSwatch() != that->isSwatch()) {  break; }
        if ( this->isSwatch() ){
           // drop down to check stops.
        }
        else if (
            (SP_IS_LINEARGRADIENT(this) && SP_IS_LINEARGRADIENT(that)) ||
            (SP_IS_RADIALGRADIENT(this) && SP_IS_RADIALGRADIENT(that)) ||
            (SP_IS_MESHGRADIENT(this) && SP_IS_MESHGRADIENT(that))) {
            if(!this->isAligned(that))break;
        }
        else { break; }  // this should never happen, some unhandled type of gradient

        SPStop *as = this->getVector()->getFirstStop();
        SPStop *bs = that->getVector()->getFirstStop();

        bool effective = TRUE;
        while (effective && (as && bs)) {
            if (!as->getEffectiveColor().isClose(bs->getEffectiveColor(), 0.001) ||
                    as->offset != bs->offset) {
                effective = FALSE;
                break;
            } 
            else {
                as = as->getNextStop();
                bs = bs->getNextStop();
            }
        }
        if(!effective)break;

        status = TRUE;
        break;
    }
    return status;
}

/**
 * return true if this gradient is "aligned" to that gradient.
 * Aligned means that they have exactly the same coordinates and transform.
 * @param that - A gradient to compare this to
 */
gboolean SPGradient::isAligned(SPGradient *that)
{
    bool status = FALSE;
    
   /*  Some gradients have coordinates/other values specified, some don't.  
           yes/yes check the coordinates/other values
           no/no   aligned (because both have all default values)
           yes/no  not aligned
           no/yes  not aligned
       It is NOT safe to just compare the computed values because if that field has
       not been set the computed value could be full of garbage.
       
       In theory the yes/no and no/yes cases could be aligned if the specified value
       matches the default value.
    */

    while(1){ // not really a loop, used to avoid deep nesting or multiple exit points from function
        if(this->gradientTransform_set != that->gradientTransform_set) { break; }
        if(this->gradientTransform_set && 
            (this->gradientTransform != that->gradientTransform)) { break; }
        if (SP_IS_LINEARGRADIENT(this) && SP_IS_LINEARGRADIENT(that)) {
            SPLinearGradient *sg=SP_LINEARGRADIENT(this);
            SPLinearGradient *tg=SP_LINEARGRADIENT(that);
 
            if( sg->x1._set != tg->x1._set) { break; }
            if( sg->y1._set != tg->y1._set) { break; }
            if( sg->x2._set != tg->x2._set) { break; }
            if( sg->y2._set != tg->y2._set) { break; }
            if( sg->x1._set && sg->y1._set && sg->x2._set && sg->y2._set) {
                if( (sg->x1.computed != tg->x1.computed) ||
                    (sg->y1.computed != tg->y1.computed) ||
                    (sg->x2.computed != tg->x2.computed) ||
                    (sg->y2.computed != tg->y2.computed) )  { break; }
            } else if( sg->x1._set || sg->y1._set || sg->x2._set || sg->y2._set) { break; } // some mix of set and not set
            // none set? assume aligned and fall through
        } else if (SP_IS_RADIALGRADIENT(this) && SP_IS_LINEARGRADIENT(that)) {
            SPRadialGradient *sg=SP_RADIALGRADIENT(this);
            SPRadialGradient *tg=SP_RADIALGRADIENT(that);

            if( sg->cx._set != tg->cx._set) { break; }
            if( sg->cy._set != tg->cy._set) { break; }
            if( sg->r._set  != tg->r._set)  { break; }
            if( sg->fx._set != tg->fx._set) { break; }
            if( sg->fy._set != tg->fy._set) { break; }
            if( sg->cx._set && sg->cy._set && sg->fx._set && sg->fy._set && sg->r._set) {
                if( (sg->cx.computed != tg->cx.computed) ||
                    (sg->cy.computed != tg->cy.computed) ||
                    (sg->r.computed  != tg->r.computed ) ||
                    (sg->fx.computed != tg->fx.computed) ||
                    (sg->fy.computed != tg->fy.computed)  ) { break; }
            } else if(  sg->cx._set || sg->cy._set || sg->fx._set || sg->fy._set || sg->r._set ) { break; } // some mix of set and not set
            // none set? assume aligned and fall through
        } else if (SP_IS_MESHGRADIENT(this) && SP_IS_MESHGRADIENT(that)) {
            SPMeshGradient *sg=SP_MESHGRADIENT(this);
            SPMeshGradient *tg=SP_MESHGRADIENT(that);
 
            if( sg->x._set  !=  !tg->x._set) { break; }
            if( sg->y._set  !=  !tg->y._set) { break; }
            if( sg->x._set  &&  sg->y._set) { 
                if( (sg->x.computed != tg->x.computed) ||
                    (sg->y.computed != tg->y.computed) ) { break; }
            } else if( sg->x._set || sg->y._set) { break; } // some mix of set and not set
            // none set? assume aligned and fall through
         } else {
            break;
        }
        status = TRUE;
        break;
    }
    return status;
}

/*
 * Gradient
 */
SPGradient::SPGradient() : SPPaintServer(), units(),
        spread(),
        ref(NULL),
        state(2),
        vector() {

	this->has_patches = 0;

    this->ref = new SPGradientReference(this);
    this->ref->changedSignal().connect(sigc::bind(sigc::ptr_fun(SPGradient::gradientRefChanged), this));

    /** \todo
     * Fixme: reprs being rearranged (e.g. via the XML editor)
     * may require us to clear the state.
     */
    this->state = SP_GRADIENT_STATE_UNKNOWN;

    this->units = SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX;
    this->units_set = FALSE;

    this->gradientTransform = Geom::identity();
    this->gradientTransform_set = FALSE;

    this->spread = SP_GRADIENT_SPREAD_PAD;
    this->spread_set = FALSE;

    this->has_stops = FALSE;

    this->vector.built = false;
    this->vector.stops.clear();
}

SPGradient::~SPGradient() {
}

/**
 * Virtual build: set gradient attributes from its associated repr.
 */
void SPGradient::build(SPDocument *document, Inkscape::XML::Node *repr)
{
    // Work-around in case a swatch had been marked for immediate collection:
    if ( repr->attribute("osb:paint") && repr->attribute("inkscape:collect") ) {
        repr->setAttribute("inkscape:collect", 0);
    }

    SPPaintServer::build(document, repr);

    for ( SPObject *ochild = this->firstChild() ; ochild ; ochild = ochild->getNext() ) {
        if (SP_IS_STOP(ochild)) {
            this->has_stops = TRUE;
            break;
        }
    }

    this->readAttr( "gradientUnits" );
    this->readAttr( "gradientTransform" );
    this->readAttr( "spreadMethod" );
    this->readAttr( "xlink:href" );
    this->readAttr( "osb:paint" );

    // Register ourselves
    document->addResource("gradient", this);
}

/**
 * Virtual release of SPGradient members before destruction.
 */
void SPGradient::release()
{

#ifdef SP_GRADIENT_VERBOSE
    g_print("Releasing this %s\n", this->getId());
#endif

    if (this->document) {
        // Unregister ourselves
        this->document->removeResource("gradient", this);
    }

    if (this->ref) {
        this->modified_connection.disconnect();
        this->ref->detach();
        delete this->ref;
        this->ref = NULL;
    }

    //this->modified_connection.~connection();

    SPPaintServer::release();
}

/**
 * Set gradient attribute to value.
 */
void SPGradient::set(unsigned key, gchar const *value)
{
    switch (key) {
        case SP_ATTR_GRADIENTUNITS:
            if (value) {
                if (!strcmp(value, "userSpaceOnUse")) {
                    this->units = SP_GRADIENT_UNITS_USERSPACEONUSE;
                } else {
                    this->units = SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX;
                }

                this->units_set = TRUE;
            } else {
                this->units = SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX;
                this->units_set = FALSE;
            }

            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_GRADIENTTRANSFORM: {
            Geom::Affine t;
            if (value && sp_svg_transform_read(value, &t)) {
                this->gradientTransform = t;
                this->gradientTransform_set = TRUE;
            } else {
                this->gradientTransform = Geom::identity();
                this->gradientTransform_set = FALSE;
            }

            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;
        }
        case SP_ATTR_SPREADMETHOD:
            if (value) {
                if (!strcmp(value, "reflect")) {
                    this->spread = SP_GRADIENT_SPREAD_REFLECT;
                } else if (!strcmp(value, "repeat")) {
                    this->spread = SP_GRADIENT_SPREAD_REPEAT;
                } else {
                    this->spread = SP_GRADIENT_SPREAD_PAD;
                }

                this->spread_set = TRUE;
            } else {
                this->spread_set = FALSE;
            }

            this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            break;

        case SP_ATTR_XLINK_HREF:
            if (value) {
                try {
                    this->ref->attach(Inkscape::URI(value));
                } catch (Inkscape::BadURIException &e) {
                    g_warning("%s", e.what());
                    this->ref->detach();
                }
            } else {
                this->ref->detach();
            }
            break;

        case SP_ATTR_OSB_SWATCH:
        {
            bool newVal = (value != 0);
            bool modified = false;

            if (newVal != this->swatch) {
                this->swatch = newVal;
                modified = true;
            }

            if (newVal) {
                // Might need to flip solid/gradient
                Glib::ustring paintVal = ( this->hasStops() && (this->getStopCount() == 0) ) ? "solid" : "gradient";

                if ( paintVal != value ) {
                    this->setAttribute( "osb:paint", paintVal.c_str(), 0 );
                    modified = true;
                }
            }

            if (modified) {
                this->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
        }
            break;
        default:
            SPPaintServer::set(key, value);
            break;
    }
}

/**
 * Gets called when the gradient is (re)attached to another gradient.
 */
void SPGradient::gradientRefChanged(SPObject *old_ref, SPObject *ref, SPGradient *gr)
{
    if (old_ref) {
        gr->modified_connection.disconnect();
    }
    if ( SP_IS_GRADIENT(ref)
         && ref != gr )
    {
        gr->modified_connection = ref->connectModified(sigc::bind<2>(sigc::ptr_fun(&SPGradient::gradientRefModified), gr));
    }

    // Per SVG, all unset attributes must be inherited from linked gradient.
    // So, as we're now (re)linked, we assign linkee's values to this gradient if they are not yet set -
    // but without setting the _set flags.
    // FIXME: do the same for gradientTransform too
    if (!gr->units_set) {
        gr->units = gr->fetchUnits();
    }
    if (!gr->spread_set) {
        gr->spread = gr->fetchSpread();
    }

    /// \todo Fixme: what should the flags (second) argument be? */
    gradientRefModified(ref, 0, gr);
}

/**
 * Callback for child_added event.
 */
void SPGradient::child_added(Inkscape::XML::Node *child, Inkscape::XML::Node *ref)
{
    this->invalidateVector();

    SPPaintServer::child_added(child, ref);

    SPObject *ochild = this->get_child_by_repr(child);
    if ( ochild && SP_IS_STOP(ochild) ) {
        this->has_stops = TRUE;
        if ( this->getStopCount() > 0 ) {
            gchar const * attr = this->getAttribute("osb:paint");
            if ( attr && strcmp(attr, "gradient") ) {
            	this->setAttribute( "osb:paint", "gradient", 0 );
            }
        }
    }

    /// \todo Fixme: should we schedule "modified" here?
    this->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

/**
 * Callback for remove_child event.
 */
void SPGradient::remove_child(Inkscape::XML::Node *child)
{
    this->invalidateVector();

    SPPaintServer::remove_child(child);

    this->has_stops = FALSE;
    for ( SPObject *ochild = this->firstChild() ; ochild ; ochild = ochild->getNext() ) {
        if (SP_IS_STOP(ochild)) {
            this->has_stops = TRUE;
            break;
        }
    }

    if ( this->getStopCount() == 0 ) {
        gchar const * attr = this->getAttribute("osb:paint");

        if ( attr && strcmp(attr, "solid") ) {
            this->setAttribute( "osb:paint", "solid", 0 );
        }
    }

    /* Fixme: should we schedule "modified" here? */
    this->requestModified(SP_OBJECT_MODIFIED_FLAG);
}

/**
 * Callback for modified event.
 */
void SPGradient::modified(guint flags)
{
    if (flags & SP_OBJECT_CHILD_MODIFIED_FLAG) {
        // CPPIFY
        // This comparison has never worked (i. e. always evaluated to false),
        // the right value would have been SP_TYPE_MESHGRADIENT
        //if( this->get_type() != SP_GRADIENT_TYPE_MESH ) {
//        if (!SP_IS_MESHGRADIENT(this)) {
//            this->invalidateVector();
//        } else {
//            this->invalidateArray();
//        }
        this->invalidateVector();
    }

    if (flags & SP_OBJECT_STYLE_MODIFIED_FLAG) {
        // CPPIFY
        // see above
        //if( this->get_type() != SP_GRADIENT_TYPE_MESH ) {
//        if (!SP_IS_MESHGRADIENT(this)) {
//            this->ensureVector();
//        } else {
//            this->ensureArray();
//        }
        this->ensureVector();
    }

    if (flags & SP_OBJECT_MODIFIED_FLAG) flags |= SP_OBJECT_PARENT_MODIFIED_FLAG;
    flags &= SP_OBJECT_MODIFIED_CASCADE;

    // FIXME: climb up the ladder of hrefs
    GSList *l = NULL;

    for (SPObject *child = this->firstChild() ; child; child = child->getNext() ) {
        sp_object_ref(child);
        l = g_slist_prepend(l, child);
    }

    l = g_slist_reverse(l);

    while (l) {
        SPObject *child = SP_OBJECT(l->data);
        l = g_slist_remove(l, child);

        if (flags || (child->mflags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_CHILD_MODIFIED_FLAG))) {
            child->emitModified(flags);
        }

        sp_object_unref(child);
    }
}

SPStop* SPGradient::getFirstStop()
{
    SPStop* first = 0;
    for (SPObject *ochild = firstChild(); ochild && !first; ochild = ochild->getNext()) {
        if (SP_IS_STOP(ochild)) {
            first = SP_STOP(ochild);
        }
    }
    return first;
}

int SPGradient::getStopCount() const
{
    int count = 0;

    for (SPStop *stop = const_cast<SPGradient*>(this)->getFirstStop(); stop && stop->getNextStop(); stop = stop->getNextStop()) {
        count++;
    }

    return count;
}

/**
 * Write gradient attributes to repr.
 */
Inkscape::XML::Node *SPGradient::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags)
{
    SPPaintServer::write(xml_doc, repr, flags);

    if (flags & SP_OBJECT_WRITE_BUILD) {
        GSList *l = NULL;

        for (SPObject *child = this->firstChild(); child; child = child->getNext()) {
            Inkscape::XML::Node *crepr = child->updateRepr(xml_doc, NULL, flags);

            if (crepr) {
                l = g_slist_prepend(l, crepr);
            }
        }

        while (l) {
            repr->addChild((Inkscape::XML::Node *) l->data, NULL);
            Inkscape::GC::release((Inkscape::XML::Node *) l->data);
            l = g_slist_remove(l, l->data);
        }
    }

    if (this->ref->getURI()) {
        gchar *uri_string = this->ref->getURI()->toString();
        repr->setAttribute("xlink:href", uri_string);
        g_free(uri_string);
    }

    if ((flags & SP_OBJECT_WRITE_ALL) || this->units_set) {
        switch (this->units) {
            case SP_GRADIENT_UNITS_USERSPACEONUSE:
                repr->setAttribute("gradientUnits", "userSpaceOnUse");
                break;
            default:
                repr->setAttribute("gradientUnits", "objectBoundingBox");
                break;
        }
    }

    if ((flags & SP_OBJECT_WRITE_ALL) || this->gradientTransform_set) {
        gchar *c=sp_svg_transform_write(this->gradientTransform);
        repr->setAttribute("gradientTransform", c);
        g_free(c);
    }

    if ((flags & SP_OBJECT_WRITE_ALL) || this->spread_set) {
        /* FIXME: Ensure that this->spread is the inherited value
         * if !this->spread_set.  Not currently happening: see SPGradient::modified.
         */
        switch (this->spread) {
            case SP_GRADIENT_SPREAD_REFLECT:
                repr->setAttribute("spreadMethod", "reflect");
                break;
            case SP_GRADIENT_SPREAD_REPEAT:
                repr->setAttribute("spreadMethod", "repeat");
                break;
            default:
                repr->setAttribute("spreadMethod", "pad");
                break;
        }
    }

    if ( (flags & SP_OBJECT_WRITE_EXT) && this->isSwatch() ) {
        if ( this->isSolid() ) {
            repr->setAttribute( "osb:paint", "solid" );
        } else {
            repr->setAttribute( "osb:paint", "gradient" );
        }
    } else {
        repr->setAttribute( "osb:paint", 0 );
    }

    return repr;
}

/**
 * Forces the vector to be built, if not present (i.e., changed).
 *
 * \pre SP_IS_GRADIENT(gradient).
 */
void SPGradient::ensureVector()
{
    if ( !vector.built ) {
        rebuildVector();
    }
}

/**
 * Forces the array to be built, if not present (i.e., changed).
 *
 * \pre SP_IS_GRADIENT(gradient).
 */
void SPGradient::ensureArray()
{
    //std::cout << "SPGradient::ensureArray()" << std::endl;
    if ( !array.built ) {
        rebuildArray();
    }
}

/**
 * Set units property of gradient and emit modified.
 */
void SPGradient::setUnits(SPGradientUnits units)
{
    if (units != this->units) {
        this->units = units;
        units_set = TRUE;
        requestModified(SP_OBJECT_MODIFIED_FLAG);
    }
}

/**
 * Set spread property of gradient and emit modified.
 */
void SPGradient::setSpread(SPGradientSpread spread)
{
    if (spread != this->spread) {
        this->spread = spread;
        spread_set = TRUE;
        requestModified(SP_OBJECT_MODIFIED_FLAG);
    }
}

/**
 * Returns the first of {src, src-\>ref-\>getObject(),
 * src-\>ref-\>getObject()-\>ref-\>getObject(),...}
 * for which \a match is true, or NULL if none found.
 *
 * The raison d'être of this routine is that it correctly handles cycles in the href chain (e.g., if
 * a gradient gives itself as its href, or if each of two gradients gives the other as its href).
 *
 * \pre SP_IS_GRADIENT(src).
 */
static SPGradient *
chase_hrefs(SPGradient *const src, bool (*match)(SPGradient const *))
{
    g_return_val_if_fail(SP_IS_GRADIENT(src), NULL);

    /* Use a pair of pointers for detecting loops: p1 advances half as fast as p2.  If there is a
       loop, then once p1 has entered the loop, we'll detect it the next time the distance between
       p1 and p2 is a multiple of the loop size. */
    SPGradient *p1 = src, *p2 = src;
    bool do1 = false;
    for (;;) {
        if (match(p2)) {
            return p2;
        }

        p2 = p2->ref->getObject();
        if (!p2) {
            return p2;
        }
        if (do1) {
            p1 = p1->ref->getObject();
        }
        do1 = !do1;

        if ( p2 == p1 ) {
            /* We've been here before, so return NULL to indicate that no matching gradient found
             * in the chain. */
            return NULL;
        }
    }
}

/**
 * True if gradient has stops.
 */
static bool has_stopsFN(SPGradient const *gr)
{
    return gr->hasStops();
}

/**
 * True if gradient has spread set.
 */
static bool has_spread_set(SPGradient const *gr)
{
    return gr->isSpreadSet();
}

/**
 * True if gradient has units set.
 */
static bool
has_units_set(SPGradient const *gr)
{
    return gr->isUnitsSet();
}


SPGradient *SPGradient::getVector(bool force_vector)
{
    SPGradient * src = chase_hrefs(this, has_stopsFN);

    if (force_vector) {
        src = sp_gradient_ensure_vector_normalized(src);
    }
    return src;
}

/**
 * Returns the effective spread of given gradient (climbing up the refs chain if needed).
 *
 * \pre SP_IS_GRADIENT(gradient).
 */
SPGradientSpread SPGradient::fetchSpread()
{
    SPGradient const *src = chase_hrefs(this, has_spread_set);
    return ( src
             ? src->spread
             : SP_GRADIENT_SPREAD_PAD ); // pad is the default
}

/**
 * Returns the effective units of given gradient (climbing up the refs chain if needed).
 *
 * \pre SP_IS_GRADIENT(gradient).
 */
SPGradientUnits SPGradient::fetchUnits()
{
    SPGradient const *src = chase_hrefs(this, has_units_set);
    return ( src
             ? src->units
             : SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX ); // bbox is the default
}


/**
 * Clears the gradient's svg:stop children from its repr.
 */
void
sp_gradient_repr_clear_vector(SPGradient *gr)
{
    Inkscape::XML::Node *repr = gr->getRepr();

    /* Collect stops from original repr */
    GSList *sl = NULL;
    for (Inkscape::XML::Node *child = repr->firstChild() ; child != NULL; child = child->next() ) {
        if (!strcmp(child->name(), "svg:stop")) {
            sl = g_slist_prepend(sl, child);
        }
    }
    /* Remove all stops */
    while (sl) {
        /** \todo
         * fixme: This should work, unless we make gradient
         * into generic group.
         */
        sp_repr_unparent((Inkscape::XML::Node *)sl->data);
        sl = g_slist_remove(sl, sl->data);
    }
}

/**
 * Writes the gradient's internal vector (whether from its own stops, or
 * inherited from refs) into the gradient repr as svg:stop elements.
 */
void
sp_gradient_repr_write_vector(SPGradient *gr)
{
    g_return_if_fail(gr != NULL);
    g_return_if_fail(SP_IS_GRADIENT(gr));

    Inkscape::XML::Document *xml_doc = gr->document->getReprDoc();
    Inkscape::XML::Node *repr = gr->getRepr();

    /* We have to be careful, as vector may be our own, so construct repr list at first */
    GSList *cl = NULL;

    for (guint i = 0; i < gr->vector.stops.size(); i++) {
        Inkscape::CSSOStringStream os;
        Inkscape::XML::Node *child = xml_doc->createElement("svg:stop");
        sp_repr_set_css_double(child, "offset", gr->vector.stops[i].offset);
        /* strictly speaking, offset an SVG <number> rather than a CSS one, but exponents make no
         * sense for offset proportions. */
        os << "stop-color:" << gr->vector.stops[i].color.toString() << ";stop-opacity:" << gr->vector.stops[i].opacity;
        child->setAttribute("style", os.str().c_str());
        /* Order will be reversed here */
        cl = g_slist_prepend(cl, child);
    }

    sp_gradient_repr_clear_vector(gr);

    /* And insert new children from list */
    while (cl) {
        Inkscape::XML::Node *child = static_cast<Inkscape::XML::Node *>(cl->data);
        repr->addChild(child, NULL);
        Inkscape::GC::release(child);
        cl = g_slist_remove(cl, child);
    }
}


void SPGradient::gradientRefModified(SPObject */*href*/, guint /*flags*/, SPGradient *gradient)
{
    if ( gradient->invalidateVector() ) {
        gradient->requestModified(SP_OBJECT_MODIFIED_FLAG);
        // Conditional to avoid causing infinite loop if there's a cycle in the href chain.
    }
}

/** Return true if change made. */
bool SPGradient::invalidateVector()
{
    bool ret = false;

    if (vector.built) {
        vector.built = false;
        vector.stops.clear();
        ret = true;
    }

    return ret;
}

/** Return true if change made. */
bool SPGradient::invalidateArray()
{
    bool ret = false;

    if (array.built) {
        array.built = false;
        array.clear();
        ret = true;
    }

    return ret;
}

/** Creates normalized color vector */
void SPGradient::rebuildVector()
{
    gint len = 0;
    for ( SPObject *child = firstChild() ; child ; child = child->getNext() ) {
        if (SP_IS_STOP(child)) {
            len ++;
        }
    }

    has_stops = (len != 0);

    vector.stops.clear();

    SPGradient *reffed = ref ? ref->getObject() : NULL;
    if ( !hasStops() && reffed ) {
        /* Copy vector from referenced gradient */
        vector.built = true;   // Prevent infinite recursion.
        reffed->ensureVector();
        if (!reffed->vector.stops.empty()) {
            vector.built = reffed->vector.built;
            vector.stops.assign(reffed->vector.stops.begin(), reffed->vector.stops.end());
            return;
        }
    }

    for ( SPObject *child = firstChild(); child; child = child->getNext() ) {
        if (SP_IS_STOP(child)) {
            SPStop *stop = SP_STOP(child);

            SPGradientStop gstop;
            if (!vector.stops.empty()) {
                // "Each gradient offset value is required to be equal to or greater than the
                // previous gradient stop's offset value. If a given gradient stop's offset
                // value is not equal to or greater than all previous offset values, then the
                // offset value is adjusted to be equal to the largest of all previous offset
                // values."
                gstop.offset = MAX(stop->offset, vector.stops.back().offset);
            } else {
                gstop.offset = stop->offset;
            }

            // "Gradient offset values less than 0 (or less than 0%) are rounded up to
            // 0%. Gradient offset values greater than 1 (or greater than 100%) are rounded
            // down to 100%."
            gstop.offset = CLAMP(gstop.offset, 0, 1);

            gstop.color = stop->getEffectiveColor();
            gstop.opacity = stop->opacity;

            vector.stops.push_back(gstop);
        }
    }

    // Normalize per section 13.2.4 of SVG 1.1.
    if (vector.stops.empty()) {
        /* "If no stops are defined, then painting shall occur as if 'none' were specified as the
         * paint style."
         */
        {
            SPGradientStop gstop;
            gstop.offset = 0.0;
            gstop.color.set( 0x00000000 );
            gstop.opacity = 0.0;
            vector.stops.push_back(gstop);
        }
        {
            SPGradientStop gstop;
            gstop.offset = 1.0;
            gstop.color.set( 0x00000000 );
            gstop.opacity = 0.0;
            vector.stops.push_back(gstop);
        }
    } else {
        /* "If one stop is defined, then paint with the solid color fill using the color defined
         * for that gradient stop."
         */
        if (vector.stops.front().offset > 0.0) {
            // If the first one is not at 0, then insert a copy of the first at 0.
            SPGradientStop gstop;
            gstop.offset = 0.0;
            gstop.color = vector.stops.front().color;
            gstop.opacity = vector.stops.front().opacity;
            vector.stops.insert(vector.stops.begin(), gstop);
        }
        if (vector.stops.back().offset < 1.0) {
            // If the last one is not at 1, then insert a copy of the last at 1.
            SPGradientStop gstop;
            gstop.offset = 1.0;
            gstop.color = vector.stops.back().color;
            gstop.opacity = vector.stops.back().opacity;
            vector.stops.push_back(gstop);
        }
    }

    vector.built = true;
}

/** Creates normalized color mesh patch array */
void SPGradient::rebuildArray()
{
    // std::cout << "SPGradient::rebuildArray()" << std::endl;

    if( !SP_IS_MESHGRADIENT(this) ) {
        g_warning( "SPGradient::rebuildArray() called for non-mesh gradient" );
        return;
    }

    array.read( SP_MESHGRADIENT( this ) );

    has_patches = false;
    for ( SPObject *ro = firstChild() ; ro ; ro = ro->getNext() ) {
        if (SP_IS_MESHROW(ro)) {
            has_patches = true;
            // std::cout << "  Has Patches" << std::endl;
            break;
        }
    }

    // MESH_FIXME: TO PROPERLY COPY
    SPGradient *reffed = ref->getObject();
    if ( !hasPatches() && reffed ) {
        std::cout << "SPGradient::rebuildArray(): reffed array    NOT IMPLEMENTED!!!" << std::endl;
        /* Copy array from referenced gradient */
        array.built = true;   // Prevent infinite recursion.
        reffed->ensureArray();
        // if (!reffed->array.nodes.empty()) {
        //     array.built = reffed->array.built;
        //     for( uint i = 0; i < reffed->array.nodes.size(); ++i ) {
        //         array.nodes[i].assign(reffed->array.nodes[i].begin(), reffed->array.nodes[i].end());

        //         // FILL ME
        //     }
        //     return;
        // }
    }
}

Geom::Affine
sp_gradient_get_g2d_matrix(SPGradient const *gr, Geom::Affine const &ctm, Geom::Rect const &bbox)
{
    if (gr->getUnits() == SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX) {
        return ( Geom::Scale(bbox.dimensions())
                 * Geom::Translate(bbox.min())
                 * Geom::Affine(ctm) );
    } else {
        return ctm;
    }
}

Geom::Affine
sp_gradient_get_gs2d_matrix(SPGradient const *gr, Geom::Affine const &ctm, Geom::Rect const &bbox)
{
    if (gr->getUnits() == SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX) {
        return ( gr->gradientTransform
                 * Geom::Scale(bbox.dimensions())
                 * Geom::Translate(bbox.min())
                 * Geom::Affine(ctm) );
    } else {
        return gr->gradientTransform * ctm;
    }
}

void
sp_gradient_set_gs2d_matrix(SPGradient *gr, Geom::Affine const &ctm,
                            Geom::Rect const &bbox, Geom::Affine const &gs2d)
{
    gr->gradientTransform = gs2d * ctm.inverse();
    if (gr->getUnits() == SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX ) {
        gr->gradientTransform = ( gr->gradientTransform
                                  * Geom::Translate(-bbox.min())
                                  * Geom::Scale(bbox.dimensions()).inverse() );
    }
    gr->gradientTransform_set = TRUE;

    gr->requestModified(SP_OBJECT_MODIFIED_FLAG);
}




/* CAIRO RENDERING STUFF */

void
sp_gradient_pattern_common_setup(cairo_pattern_t *cp,
                                 SPGradient *gr,
                                 Geom::OptRect const &bbox,
                                 double opacity)
{
    // set spread type
    switch (gr->getSpread()) {
    case SP_GRADIENT_SPREAD_REFLECT:
        cairo_pattern_set_extend(cp, CAIRO_EXTEND_REFLECT);
        break;
    case SP_GRADIENT_SPREAD_REPEAT:
        cairo_pattern_set_extend(cp, CAIRO_EXTEND_REPEAT);
        break;
    case SP_GRADIENT_SPREAD_PAD:
    default:
        cairo_pattern_set_extend(cp, CAIRO_EXTEND_PAD);
        break;
    }

    // add stops
    for (std::vector<SPGradientStop>::iterator i = gr->vector.stops.begin();
         i != gr->vector.stops.end(); ++i)
    {
        // multiply stop opacity by paint opacity
        cairo_pattern_add_color_stop_rgba(cp, i->offset,
            i->color.v.c[0], i->color.v.c[1], i->color.v.c[2], i->opacity * opacity);
    }

    // set pattern matrix
    Geom::Affine gs2user = gr->gradientTransform;
    if (gr->getUnits() == SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX && bbox) {
        Geom::Affine bbox2user(bbox->width(), 0, 0, bbox->height(), bbox->left(), bbox->top());
        gs2user *= bbox2user;
    }
    ink_cairo_pattern_set_matrix(cp, gs2user.inverse());
}

cairo_pattern_t *
sp_gradient_create_preview_pattern(SPGradient *gr, double width)
{
    cairo_pattern_t *pat = NULL;

    // CPPIFY
    //if( gr->get_type() != SP_GRADIENT_TYPE_MESH ) {
    if (!SP_IS_MESHGRADIENT(gr)) {
        gr->ensureVector();

        pat = cairo_pattern_create_linear(0, 0, width, 0);

        for (std::vector<SPGradientStop>::iterator i = gr->vector.stops.begin();
             i != gr->vector.stops.end(); ++i)
        {
            cairo_pattern_add_color_stop_rgba(pat, i->offset,
              i->color.v.c[0], i->color.v.c[1], i->color.v.c[2], i->opacity);
        }
    }

    return pat;
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
