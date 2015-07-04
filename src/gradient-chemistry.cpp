/*
 * Various utility methods for gradients
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak
 *   Johan Engelen <j.b.c.engelen@ewi.utwente.nl>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *   Tavmjong Bah <tavmjong@free.fr>
 *
 * Copyright (C) 2012 Tavmjong Bah
 * Copyright (C) 2010 Authors
 * Copyright (C) 2007 Johan Engelen
 * Copyright (C) 2001-2005 authors
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "widgets/gradient-vector.h"

#include <2geom/transforms.h>
#include <2geom/bezier-curve.h>
#include <2geom/crossing.h>
#include <2geom/line.h>
#include <2geom/angle.h>

#include "style.h"
#include "document-private.h"
#include "document-undo.h"
#include "desktop.h"
#include "desktop-style.h"

#include "ui/tools/tool-base.h"
#include "selection.h"
#include "verbs.h"
#include <glibmm/i18n.h>

#include "sp-gradient-reference.h"
#include "sp-gradient-vector.h"
#include "sp-linear-gradient.h"
#include "sp-radial-gradient.h"
#include "sp-mesh.h"
#include "sp-stop.h"
#include "gradient-drag.h"
#include "gradient-chemistry.h"

#include "sp-text.h"
#include "sp-tspan.h"
#include "xml/repr.h"
#include "svg/svg.h"
#include "svg/svg-color.h"
#include "svg/css-ostringstream.h"
#include "preferences.h"

#define noSP_GR_VERBOSE

using Inkscape::DocumentUndo;

namespace {

Inkscape::PaintTarget paintTargetItems[] = {Inkscape::FOR_FILL, Inkscape::FOR_STROKE};

std::vector<Inkscape::PaintTarget> vectorOfPaintTargets(paintTargetItems, paintTargetItems + (sizeof(paintTargetItems) / sizeof(paintTargetItems[0])));

} // namespace

namespace Inkscape {

std::vector<PaintTarget> const &allPaintTargets()
{
    return vectorOfPaintTargets;
}

} // namespace Inkscape

// Terminology:
//
// "vector" is a gradient that has stops but not position coords. It can be referenced by one or
// more privates. Objects should not refer to it directly. It has no radial/linear distinction.
//
// "private" is a gradient that has no stops but has position coords (e.g. center, radius etc for a
// radial). It references a vector for the actual colors. Each private is only used by one
// object. It is either linear or radial.

static void sp_gradient_repr_set_link(Inkscape::XML::Node *repr, SPGradient *gr);

SPGradient *sp_gradient_ensure_vector_normalized(SPGradient *gr)
{
#ifdef SP_GR_VERBOSE
    g_message("sp_gradient_ensure_vector_normalized(%p)", gr);
#endif
    g_return_val_if_fail(gr != NULL, NULL);
    g_return_val_if_fail(SP_IS_GRADIENT(gr), NULL);

    /* If we are already normalized vector, just return */
    if (gr->state == SP_GRADIENT_STATE_VECTOR) return gr;
    /* Fail, if we have wrong state set */
    if (gr->state != SP_GRADIENT_STATE_UNKNOWN) {
        g_warning("file %s: line %d: Cannot normalize private gradient to vector (%s)", __FILE__, __LINE__, gr->getId());
        return NULL;
    }

    /* First make sure we have vector directly defined (i.e. gr has its own stops) */
    if ( !gr->hasStops() ) {
        /* We do not have stops ourselves, so flatten stops as well */
        gr->ensureVector();
        g_assert(gr->vector.built);
        // this adds stops from gr->vector as children to gr
        sp_gradient_repr_write_vector (gr);
    }

    /* If gr hrefs some other gradient, remove the href */
    if (gr->ref){
        if (gr->ref->getObject()) {
            // We are hrefing someone, so require flattening
            gr->updateRepr(SP_OBJECT_WRITE_EXT | SP_OBJECT_WRITE_ALL);
            sp_gradient_repr_set_link(gr->getRepr(), NULL);
        }
    }

    /* Everything is OK, set state flag */
    gr->state = SP_GRADIENT_STATE_VECTOR;
    return gr;
}

/**
 * Creates new private gradient for the given vector
 */

static SPGradient *sp_gradient_get_private_normalized(SPDocument *document, SPGradient *vector, SPGradientType type)
{
#ifdef SP_GR_VERBOSE
    g_message("sp_gradient_get_private_normalized(%p, %p, %d)", document, vector, type);
#endif

    g_return_val_if_fail(document != NULL, NULL);
    g_return_val_if_fail(vector != NULL, NULL);
    g_return_val_if_fail(SP_IS_GRADIENT(vector), NULL);
    g_return_val_if_fail(vector->hasStops(), NULL);

    SPDefs *defs = document->getDefs();

    Inkscape::XML::Document *xml_doc = document->getReprDoc();
    // create a new private gradient of the requested type
    Inkscape::XML::Node *repr;
    if (type == SP_GRADIENT_TYPE_LINEAR) {
        repr = xml_doc->createElement("svg:linearGradient");
    } else if(type == SP_GRADIENT_TYPE_RADIAL) {
        repr = xml_doc->createElement("svg:radialGradient");
    } else {
        // Rows/patches added in sp_gradient_reset_to_userspace for new meshes.
        repr = xml_doc->createElement("svg:mesh");
    }

    // privates are garbage-collectable
    repr->setAttribute("inkscape:collect", "always");

    // link to vector
    // MESH FIXME: Meshes don't used vector... but meshes simulating gradient across/along path might.
    sp_gradient_repr_set_link(repr, vector);

    /* Append the new private gradient to defs */
    defs->getRepr()->appendChild(repr);
    Inkscape::GC::release(repr);

    // get corresponding object
    SPGradient *gr = static_cast<SPGradient *>(document->getObjectByRepr(repr));
    g_assert(gr != NULL);
    g_assert(SP_IS_GRADIENT(gr));

    return gr;
}

/**
Count how many times gr is used by the styles of o and its descendants
*/
static guint count_gradient_hrefs(SPObject *o, SPGradient *gr)
{
    if (!o)
        return 1;

    guint i = 0;

    SPStyle *style = o->style;
    if (style
        && style->fill.isPaintserver()
        && SP_IS_GRADIENT(SP_STYLE_FILL_SERVER(style))
        && SP_GRADIENT(SP_STYLE_FILL_SERVER(style)) == gr)
    {
        i ++;
    }
    if (style
        && style->stroke.isPaintserver()
        && SP_IS_GRADIENT(SP_STYLE_STROKE_SERVER(style))
        && SP_GRADIENT(SP_STYLE_STROKE_SERVER(style)) == gr)
    {
        i ++;
    }

    for ( SPObject *child = o->firstChild(); child; child = child->getNext() ) {
        i += count_gradient_hrefs(child, gr);
    }

    return i;
}


/**
 * If gr has other users, create a new private; also check if gr links to vector, relink if not
 */
static SPGradient *sp_gradient_fork_private_if_necessary(SPGradient *gr, SPGradient *vector,
                                                         SPGradientType type, SPObject *o)
{
#ifdef SP_GR_VERBOSE
    g_message("sp_gradient_fork_private_if_necessary(%p, %p, %d, %p)", gr, vector, type, o);
#endif
    g_return_val_if_fail(gr != NULL, NULL);
    g_return_val_if_fail(SP_IS_GRADIENT(gr), NULL);

    // Orphaned gradient, no vector with stops at the end of the line; this used to be an assert
    // but i think we should not abort on this - maybe just write a validity warning into some sort
    // of log
    if ( !vector || !vector->hasStops() ) {
        return (gr);
    }

    // user is the object that uses this gradient; normally it's item but for tspans, we
    // check its ancestor text so that tspans don't get different gradients from their
    // texts.
    SPObject *user = o;
    while (SP_IS_TSPAN(user)) {
        user = user->parent;
    }

    // Check the number of uses of the gradient within this object;
    // if we are private and there are no other users,
    if (!vector->isSwatch() && (gr->hrefcount <= count_gradient_hrefs(user, gr))) {
        // check vector
        if ( gr != vector && gr->ref->getObject() != vector ) {
            /* our href is not the vector, and vector is different from gr; relink */
            sp_gradient_repr_set_link(gr->getRepr(), vector);
        }
        return gr;
    }

    SPDocument *doc = gr->document;
    SPObject *defs = doc->getDefs();

    if ((gr->hasStops()) ||
        (gr->state != SP_GRADIENT_STATE_UNKNOWN) ||
        (gr->parent != SP_OBJECT(defs)) ||
        (gr->hrefcount > 1)) {
        // we have to clone a fresh new private gradient for the given vector

        // create an empty one
        SPGradient *gr_new = sp_gradient_get_private_normalized(doc, vector, type);

        // copy all the attributes to it
        Inkscape::XML::Node *repr_new = gr_new->getRepr();
        Inkscape::XML::Node *repr = gr->getRepr();
        repr_new->setAttribute("gradientUnits", repr->attribute("gradientUnits"));
        repr_new->setAttribute("gradientTransform", repr->attribute("gradientTransform"));
        repr_new->setAttribute("spreadMethod", repr->attribute("spreadMethod"));
        if (SP_IS_RADIALGRADIENT(gr)) {
            repr_new->setAttribute("cx", repr->attribute("cx"));
            repr_new->setAttribute("cy", repr->attribute("cy"));
            repr_new->setAttribute("fx", repr->attribute("fx"));
            repr_new->setAttribute("fy", repr->attribute("fy"));
            repr_new->setAttribute("r", repr->attribute("r"));
        } else if (SP_IS_LINEARGRADIENT(gr)) {
            repr_new->setAttribute("x1", repr->attribute("x1"));
            repr_new->setAttribute("y1", repr->attribute("y1"));
            repr_new->setAttribute("x2", repr->attribute("x2"));
            repr_new->setAttribute("y2", repr->attribute("y2"));
        } else {
            std::cout << "sp_gradient_fork_private_if_necessary: mesh not implemented" << std::endl;
        }

        return gr_new;
    } else {
        return gr;
    }
}

SPGradient *sp_gradient_fork_vector_if_necessary(SPGradient *gr)
{
#ifdef SP_GR_VERBOSE
    g_message("sp_gradient_fork_vector_if_necessary(%p)", gr);
#endif
    // Some people actually prefer their gradient vectors to be shared...
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (!prefs->getBool("/options/forkgradientvectors/value", true))
        return gr;

    if (gr->hrefcount > 1) {
        SPDocument *doc = gr->document;
        Inkscape::XML::Document *xml_doc = doc->getReprDoc();

        Inkscape::XML::Node *repr = gr->getRepr()->duplicate(xml_doc);
        doc->getDefs()->getRepr()->addChild(repr, NULL);
        SPGradient *gr_new = static_cast<SPGradient *>(doc->getObjectByRepr(repr));
        gr_new = sp_gradient_ensure_vector_normalized (gr_new);
        Inkscape::GC::release(repr);
        return gr_new;
    }
    return gr;
}

/**
 *  Obtain the vector from the gradient. A forked vector will be created and linked to this gradient if another gradient uses it.
 */
SPGradient *sp_gradient_get_forked_vector_if_necessary(SPGradient *gradient, bool force_vector)
{
#ifdef SP_GR_VERBOSE
    g_message("sp_gradient_get_forked_vector_if_necessary(%p, %d)", gradient, force_vector);
#endif
    SPGradient *vector = gradient->getVector(force_vector);
    vector = sp_gradient_fork_vector_if_necessary (vector);
    if ( gradient != vector && gradient->ref->getObject() != vector ) {
        sp_gradient_repr_set_link(gradient->getRepr(), vector);
    }
    return vector;
}


/**
 * Convert an item's gradient to userspace _without_ preserving coords, setting them to defaults
 * instead. No forking or reapplying is done because this is only called for newly created privates.
 * @return The new gradient.
 */
SPGradient *sp_gradient_reset_to_userspace(SPGradient *gr, SPItem *item)
{
#ifdef SP_GR_VERBOSE
    g_message("sp_gradient_reset_to_userspace(%p, %p)", gr, item);
#endif
    Inkscape::XML::Node *repr = gr->getRepr();

    // calculate the bbox of the item
    item->document->ensureUpToDate();
    Geom::OptRect bbox = item->visualBounds(); // we need "true" bbox without item_i2d_affine

    if (!bbox)
        return gr;

    Geom::Coord const width = bbox->dimensions()[Geom::X];
    Geom::Coord const height = bbox->dimensions()[Geom::Y];

    Geom::Point const center = bbox->midpoint();

    if (SP_IS_RADIALGRADIENT(gr)) {
        sp_repr_set_svg_double(repr, "cx", center[Geom::X]);
        sp_repr_set_svg_double(repr, "cy", center[Geom::Y]);
        sp_repr_set_svg_double(repr, "fx", center[Geom::X]);
        sp_repr_set_svg_double(repr, "fy", center[Geom::Y]);
        sp_repr_set_svg_double(repr, "r", width/2);

        // we want it to be elliptic, not circular
        Geom::Affine squeeze = Geom::Translate (-center) *
            Geom::Scale(1, height/width) *
            Geom::Translate (center);

        gr->gradientTransform = squeeze;
        {
            gchar *c=sp_svg_transform_write(gr->gradientTransform);
            gr->getRepr()->setAttribute("gradientTransform", c);
            g_free(c);
        }
    } else if (SP_IS_LINEARGRADIENT(gr)) {

        // Assume horizontal gradient by default (as per SVG 1.1)
        Geom::Point pStart = center - Geom::Point(width/2, 0);
        Geom::Point pEnd = center + Geom::Point(width/2, 0);

        // Get the preferred gradient angle from prefs
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        double angle = prefs->getDouble("/dialogs/gradienteditor/angle", 0.0);

        if (angle != 0.0) {

            Geom::Line grl(center, Geom::deg_to_rad(angle));
            Geom::LineSegment bbl1(bbox->corner(0), bbox->corner(1));
            Geom::LineSegment bbl2(bbox->corner(1), bbox->corner(2));
            Geom::LineSegment bbl3(bbox->corner(2), bbox->corner(3));
            Geom::LineSegment bbl4(bbox->corner(3), bbox->corner(0));

            // Find where our gradient line intersects the bounding box.
            if (intersection(bbl1, grl)) {
                pStart = bbl1.pointAt((*intersection(bbl1, grl)).ta);
                pEnd = bbl3.pointAt((*intersection(bbl3, grl)).ta);
                if (intersection(bbl1, grl.ray(grl.angle()))) {
                    std::swap(pStart, pEnd);
                }
            } else if (intersection(bbl2, grl)) {
                pStart = bbl2.pointAt((*intersection(bbl2, grl)).ta);
                pEnd = bbl4.pointAt((*intersection(bbl4, grl)).ta);
                if (intersection(bbl2, grl.ray(grl.angle()))) {
                    std::swap(pStart, pEnd);
                }
            }

        }

        sp_repr_set_svg_double(repr, "x1", pStart[Geom::X]);
        sp_repr_set_svg_double(repr, "y1", pStart[Geom::Y]);
        sp_repr_set_svg_double(repr, "x2", pEnd[Geom::X]);
        sp_repr_set_svg_double(repr, "y2", pEnd[Geom::Y]);

    } else {
        // Mesh
        // THIS IS BEING CALLED TWICE WHENEVER A NEW GRADIENT IS CREATED, WRITING HERE CAUSES PROBLEMS
        // IN SPMeshNodeArray::create()
        //sp_repr_set_svg_double(repr, "x", bbox->min()[Geom::X]);
        //sp_repr_set_svg_double(repr, "y", bbox->min()[Geom::Y]);
        SPMesh* mg = SP_MESH( gr );
        mg->array.create( mg, item, bbox );
    }

    // set the gradientUnits
    repr->setAttribute("gradientUnits", "userSpaceOnUse");

    return gr;
}

/**
 * Convert an item's gradient to userspace if necessary, also fork it if necessary.
 * @return The new gradient.
 */
SPGradient *sp_gradient_convert_to_userspace(SPGradient *gr, SPItem *item, gchar const *property)
{
#ifdef SP_GR_VERBOSE
    g_message("sp_gradient_convert_to_userspace(%p, %p, \"%s\")", gr, item, property);
#endif
    g_return_val_if_fail(SP_IS_GRADIENT(gr), NULL);

    if ( gr && gr->isSolid() ) {
        return gr;
    }

    // First, fork it if it is shared
    gr = sp_gradient_fork_private_if_necessary(gr, gr->getVector(),
                                               SP_IS_RADIALGRADIENT(gr) ? SP_GRADIENT_TYPE_RADIAL : SP_GRADIENT_TYPE_LINEAR, item);

    if (gr->getUnits() == SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX) {

        Inkscape::XML::Node *repr = gr->getRepr();

        // calculate the bbox of the item
        item->document->ensureUpToDate();
        Geom::Affine bbox2user;
        Geom::OptRect bbox = item->visualBounds(); // we need "true" bbox without item_i2d_affine
        if ( bbox ) {
            bbox2user = Geom::Affine(bbox->dimensions()[Geom::X], 0,
                                   0, bbox->dimensions()[Geom::Y],
                                   bbox->min()[Geom::X], bbox->min()[Geom::Y]);
        } else {
            // would be degenerate otherwise
            bbox2user = Geom::identity();
        }

        /* skew is the additional transform, defined by the proportions of the item, that we need
         * to apply to the gradient in order to work around this weird bit from SVG 1.1
         * (http://www.w3.org/TR/SVG11/pservers.html#LinearGradients):
         *
         *   When gradientUnits="objectBoundingBox" and gradientTransform is the identity
         *   matrix, the stripes of the linear gradient are perpendicular to the gradient
         *   vector in object bounding box space (i.e., the abstract coordinate system where
         *   (0,0) is at the top/left of the object bounding box and (1,1) is at the
         *   bottom/right of the object bounding box). When the object's bounding box is not
         *   square, the stripes that are conceptually perpendicular to the gradient vector
         *   within object bounding box space will render non-perpendicular relative to the
         *   gradient vector in user space due to application of the non-uniform scaling
         *   transformation from bounding box space to user space.
         */
        Geom::Affine skew = bbox2user;
        double exp = skew.descrim();
        skew[0] /= exp;
        skew[1] /= exp;
        skew[2] /= exp;
        skew[3] /= exp;
        skew[4] = 0;
        skew[5] = 0;

        // apply skew to the gradient
        gr->gradientTransform = skew;
        {
            gchar *c=sp_svg_transform_write(gr->gradientTransform);
            gr->getRepr()->setAttribute("gradientTransform", c);
            g_free(c);
        }

        // Matrix to convert points to userspace coords; postmultiply by inverse of skew so
        // as to cancel it out when it's applied to the gradient during rendering
        Geom::Affine point_convert = bbox2user * skew.inverse();

        if (SP_IS_RADIALGRADIENT(gr)) {
            SPRadialGradient *rg = SP_RADIALGRADIENT(gr);

            // original points in the bbox coords
            Geom::Point c_b = Geom::Point(rg->cx.computed, rg->cy.computed);
            Geom::Point f_b = Geom::Point(rg->fx.computed, rg->fy.computed);
            double r_b = rg->r.computed;

            // converted points in userspace coords
            Geom::Point c_u = c_b * point_convert;
            Geom::Point f_u = f_b * point_convert;
            double r_u = r_b * point_convert.descrim();

            sp_repr_set_svg_double(repr, "cx", c_u[Geom::X]);
            sp_repr_set_svg_double(repr, "cy", c_u[Geom::Y]);
            sp_repr_set_svg_double(repr, "fx", f_u[Geom::X]);
            sp_repr_set_svg_double(repr, "fy", f_u[Geom::Y]);
            sp_repr_set_svg_double(repr, "r", r_u);

        } else {
            SPLinearGradient *lg = SP_LINEARGRADIENT(gr);

            Geom::Point p1_b = Geom::Point(lg->x1.computed, lg->y1.computed);
            Geom::Point p2_b = Geom::Point(lg->x2.computed, lg->y2.computed);

            Geom::Point p1_u = p1_b * point_convert;
            Geom::Point p2_u = p2_b * point_convert;

            sp_repr_set_svg_double(repr, "x1", p1_u[Geom::X]);
            sp_repr_set_svg_double(repr, "y1", p1_u[Geom::Y]);
            sp_repr_set_svg_double(repr, "x2", p2_u[Geom::X]);
            sp_repr_set_svg_double(repr, "y2", p2_u[Geom::Y]);
        }

        // set the gradientUnits
        repr->setAttribute("gradientUnits", "userSpaceOnUse");
    }

    // apply the gradient to the item (may be necessary if we forked it); not recursive
    // generally because grouped items will be taken care of later (we're being called
    // from sp_item_adjust_paint_recursive); however text and all its children should all
    // refer to one gradient, hence the recursive call for text (because we can't/don't
    // want to access tspans and set gradients on them separately)
    if (SP_IS_TEXT(item)) {
        sp_style_set_property_url(item, property, gr, true);
    } else {
        sp_style_set_property_url(item, property, gr, false);
    }

    return gr;
}

void sp_gradient_transform_multiply(SPGradient *gradient, Geom::Affine postmul, bool set)
{
#ifdef SP_GR_VERBOSE
    g_message("sp_gradient_transform_multiply(%p, , %d)", gradient, set);
#endif
    if (set) {
        gradient->gradientTransform = postmul;
    } else {
        gradient->gradientTransform *= postmul; // fixme: get gradient transform by climbing to hrefs?
    }
    gradient->gradientTransform_set = TRUE;

    gchar *c=sp_svg_transform_write(gradient->gradientTransform);
    gradient->getRepr()->setAttribute("gradientTransform", c);
    g_free(c);
}

SPGradient *getGradient(SPItem *item, Inkscape::PaintTarget fill_or_stroke)
{
    SPStyle *style = item->style;
    SPGradient *gradient = 0;

    switch (fill_or_stroke)
    {
        case Inkscape::FOR_FILL:
            if (style && (style->fill.isPaintserver())) {
                SPPaintServer *server = item->style->getFillPaintServer();
                if ( SP_IS_GRADIENT(server) ) {
                    gradient = SP_GRADIENT(server);
                }
            }
            break;
        case Inkscape::FOR_STROKE:
            if (style && (style->stroke.isPaintserver())) {
                SPPaintServer *server = item->style->getStrokePaintServer();
                if ( SP_IS_GRADIENT(server) ) {
                    gradient = SP_GRADIENT(server);
                }
            }
            break;
    }

   return gradient;
}

SPStop *sp_last_stop(SPGradient *gradient)
{
    for (SPStop *stop = gradient->getFirstStop(); stop != NULL; stop = stop->getNextStop()) {
        if (stop->getNextStop() == NULL)
            return stop;
    }
    return NULL;
}

SPStop *sp_get_stop_i(SPGradient *gradient, guint stop_i)
{
    SPStop *stop = gradient->getFirstStop();
    if (!stop) {
        return NULL;
    }

    // if this is valid but weird gradient without an offset-zero stop element,
    // inkscape has created a handle for the start of gradient anyway,
    // so when it asks for stop N that corresponds to stop element N-1
    if (stop->offset != 0)
    {
        stop_i--;
    }
    
    for (guint i = 0; i < stop_i; i++) {
        if (!stop) {
            return NULL;
        }
        stop = stop->getNextStop();
    }

    return stop;
}

guint32 average_color(guint32 c1, guint32 c2, gdouble p)
{
    guint32 r = static_cast<guint32>(SP_RGBA32_R_U (c1) * (1 - p) + SP_RGBA32_R_U (c2) * p);
    guint32 g = static_cast<guint32>(SP_RGBA32_G_U (c1) * (1 - p) + SP_RGBA32_G_U (c2) * p);
    guint32 b = static_cast<guint32>(SP_RGBA32_B_U (c1) * (1 - p) + SP_RGBA32_B_U (c2) * p);
    guint32 a = static_cast<guint32>(SP_RGBA32_A_U (c1) * (1 - p) + SP_RGBA32_A_U (c2) * p);

    return SP_RGBA32_U_COMPOSE(r, g, b, a);
}

SPStop *sp_vector_add_stop(SPGradient *vector, SPStop* prev_stop, SPStop* next_stop, gfloat offset)
{
#ifdef SP_GR_VERBOSE
    g_message("sp_vector_add_stop(%p, %p, %p, %f)", vector, prev_stop, next_stop, offset);
#endif

    Inkscape::XML::Node *new_stop_repr = NULL;
    new_stop_repr = prev_stop->getRepr()->duplicate(vector->getRepr()->document());
    vector->getRepr()->addChild(new_stop_repr, prev_stop->getRepr());

    SPStop *newstop = reinterpret_cast<SPStop *>(vector->document->getObjectByRepr(new_stop_repr));
    newstop->offset = offset;
    sp_repr_set_css_double( newstop->getRepr(), "offset", (double)offset);
    guint32 const c1 = prev_stop->get_rgba32();
    guint32 const c2 = next_stop->get_rgba32();
    guint32 cnew = average_color (c1, c2, (offset - prev_stop->offset) / (next_stop->offset - prev_stop->offset));
    Inkscape::CSSOStringStream os;
    gchar c[64];
    sp_svg_write_color (c, sizeof(c), cnew);
    gdouble opacity = (gdouble) SP_RGBA32_A_F (cnew);
    os << "stop-color:" << c << ";stop-opacity:" << opacity <<";";
    newstop->getRepr()->setAttribute("style", os.str().c_str());
    Inkscape::GC::release(new_stop_repr);

    return newstop;
}

void sp_item_gradient_edit_stop(SPItem *item, GrPointType point_type, guint point_i, Inkscape::PaintTarget fill_or_stroke)
{
    SPGradient *gradient = getGradient(item, fill_or_stroke);

    if (!gradient || !SP_IS_GRADIENT(gradient)) {
        return;
    }

    SPGradient *vector = gradient->getVector();
    switch (point_type) {
        case POINT_LG_BEGIN:
        case POINT_RG_CENTER:
        case POINT_RG_FOCUS:
        {
            GtkWidget *dialog = sp_gradient_vector_editor_new (vector, vector->getFirstStop());
            gtk_widget_show (dialog);
        }
        break;

        case POINT_LG_END:
        case POINT_RG_R1:
        case POINT_RG_R2:
        {
            GtkWidget *dialog = sp_gradient_vector_editor_new (vector, sp_last_stop (vector));
            gtk_widget_show (dialog);
        }
        break;

        case POINT_LG_MID:
        case POINT_RG_MID1:
        case POINT_RG_MID2:
        {
            GtkWidget *dialog = sp_gradient_vector_editor_new (vector, sp_get_stop_i (vector, point_i));
            gtk_widget_show (dialog);
        }
        break;
        default:
            g_warning( "Unhandled gradient handle" );
            break;
    }
}

guint32 sp_item_gradient_stop_query_style(SPItem *item, GrPointType point_type, guint point_i, Inkscape::PaintTarget fill_or_stroke)
{
    SPGradient *gradient = getGradient(item, fill_or_stroke);

    if (!gradient || !SP_IS_GRADIENT(gradient)) {
        return 0;
    }

    if (SP_IS_LINEARGRADIENT(gradient) || SP_IS_RADIALGRADIENT(gradient) ) {

        SPGradient *vector = gradient->getVector();

        if (!vector) // orphan!
            return 0; // what else to do?

        switch (point_type) {
            case POINT_LG_BEGIN:
            case POINT_RG_CENTER:
            case POINT_RG_FOCUS:
            {
                SPStop *first = vector->getFirstStop();
                if (first) {
                    return first->get_rgba32();
                }
            }
            break;

            case POINT_LG_END:
            case POINT_RG_R1:
            case POINT_RG_R2:
            {
                SPStop *last = sp_last_stop (vector);
                if (last) {
                    return last->get_rgba32();
                }
            }
            break;

            case POINT_LG_MID:
            case POINT_RG_MID1:
            case POINT_RG_MID2:
            {
                SPStop *stopi = sp_get_stop_i (vector, point_i);
                if (stopi) {
                    return stopi->get_rgba32();
                }
            }
            break;

            default:
                g_warning( "Bad linear/radial gradient handle type" );
                break;
        }
        return 0;
    } else if (SP_IS_MESH(gradient)) {

        // Mesh gradient
        SPMesh *mg = SP_MESH(gradient);

        switch (point_type) {
            case POINT_MG_CORNER: {
                if (point_i >= mg->array.corners.size()) {
                    return 0;
                }
                SPMeshNode const* cornerpoint = mg->array.corners[ point_i ];

                if (cornerpoint) {
                    SPColor color  = cornerpoint->color;
                    double opacity = cornerpoint->opacity;
                    return  color.toRGBA32( opacity );
                } else {
                    return 0;
                }
                break;
            }

            case POINT_MG_HANDLE:
            case POINT_MG_TENSOR:
            {
                // Do nothing. Handles and tensors don't have color
                break;
            }

            default:
                g_warning( "Bad mesh handle type" );
        }
        return 0;
    }

    return 0;
}

void sp_item_gradient_stop_set_style(SPItem *item, GrPointType point_type, guint point_i, Inkscape::PaintTarget fill_or_stroke, SPCSSAttr *stop)
{
#ifdef SP_GR_VERBOSE
    g_message("sp_item_gradient_stop_set_style(%p, %d, %d, %d, %p)", item, point_type, point_i, fill_or_stroke, stop);
#endif
    SPGradient *gradient = getGradient(item, fill_or_stroke);

    if (!gradient || !SP_IS_GRADIENT(gradient))
        return;

    if (SP_IS_LINEARGRADIENT(gradient) || SP_IS_RADIALGRADIENT(gradient) ) {

        SPGradient *vector = gradient->getVector();

        if (!vector) // orphan!
            return;

        vector = sp_gradient_fork_vector_if_necessary (vector);
        if ( gradient != vector && gradient->ref->getObject() != vector ) {
            sp_gradient_repr_set_link(gradient->getRepr(), vector);
        }

        switch (point_type) {
            case POINT_LG_BEGIN:
            case POINT_RG_CENTER:
            case POINT_RG_FOCUS:
            {
                SPStop *first = vector->getFirstStop();
                if (first) {
                    sp_repr_css_change(first->getRepr(), stop, "style");
                }
            }
            break;

            case POINT_LG_END:
            case POINT_RG_R1:
            case POINT_RG_R2:
            {
                SPStop *last = sp_last_stop (vector);
                if (last) {
                    sp_repr_css_change(last->getRepr(), stop, "style");
                }
            }
            break;

            case POINT_LG_MID:
            case POINT_RG_MID1:
            case POINT_RG_MID2:
            {
                SPStop *stopi = sp_get_stop_i (vector, point_i);
                if (stopi) {
                    sp_repr_css_change(stopi->getRepr(), stop, "style");
                }
            }
            break;

            default:
                g_warning( "Bad linear/radial gradient handle type" );
                break;
        }
    } else {

        // Mesh gradient
        SPMesh *mg = SP_MESH(gradient);

        bool changed = false;
        switch (point_type) {
            case POINT_MG_CORNER: {

                gchar const* color_str = sp_repr_css_property( stop, "stop-color", NULL );
                if( color_str ) {
                    SPColor color( 0 );
                    SPIPaint paint;
                    paint.read( color_str );
                    if( paint.isColor() ) {
                        color = paint.value.color;
                    }
                    mg->array.corners[ point_i ]->color = color;
                    changed = true;
                }
                gchar const* opacity_str = sp_repr_css_property( stop, "stop-opacity", NULL );
                if( opacity_str ) {
                    std::stringstream os( opacity_str );
                    double opacity = 1.0;
                    os >> opacity;
                    mg->array.corners[ point_i ]->opacity = opacity;
                    changed = true;
                }
                if( changed ) {
                    gradient->requestModified(SP_OBJECT_MODIFIED_FLAG);
                    mg->array.write( mg );
                }
                break;
            }

            case POINT_MG_HANDLE:
            case POINT_MG_TENSOR:
            {
                // Do nothing. Handles and tensors don't have colors.
                break;
            }

            default:
                g_warning( "Bad mesh handle type" );
        }
    }
}

void sp_item_gradient_reverse_vector(SPItem *item, Inkscape::PaintTarget fill_or_stroke)
{
#ifdef SP_GR_VERBOSE
    g_message("sp_item_gradient_reverse_vector(%p, %d)", item, fill_or_stroke);
#endif
    SPGradient *gradient = getGradient(item, fill_or_stroke);
    if (!gradient || !SP_IS_GRADIENT(gradient))
        return;

    SPGradient *vector = gradient->getVector();
    if (!vector) // orphan!
        return;

    vector = sp_gradient_fork_vector_if_necessary (vector);
    if ( gradient != vector && gradient->ref->getObject() != vector ) {
        sp_gradient_repr_set_link(gradient->getRepr(), vector);
    }

    GSList *child_reprs = NULL;
    GSList *child_objects = NULL;
    std::vector<double> offsets;
    double offset;
    for ( SPObject *child = vector->firstChild(); child; child = child->getNext()) {
        child_reprs = g_slist_prepend (child_reprs, child->getRepr());
        child_objects = g_slist_prepend (child_objects, child);
        offset=0;
        sp_repr_get_double(child->getRepr(), "offset", &offset);
        offsets.push_back(offset);
    }

    GSList *child_copies = NULL;
    for (GSList *i = child_reprs; i != NULL; i = i->next) {
        Inkscape::XML::Node *repr = (Inkscape::XML::Node *) i->data;
        Inkscape::XML::Document *xml_doc = vector->getRepr()->document();
        child_copies = g_slist_append (child_copies, repr->duplicate(xml_doc));
    }


    for (GSList *i = child_objects; i != NULL; i = i->next) {
        SPObject *child = SP_OBJECT (i->data);
        child->deleteObject();
    }

    std::vector<double>::iterator iter = offsets.end() - 1;
    for (GSList *i = child_copies; i != NULL; i = i->next) {
        Inkscape::XML::Node *copy = (Inkscape::XML::Node *) i->data;
        vector->appendChildRepr(copy);
        sp_repr_set_svg_double (copy, "offset", 1 - *iter);
        --iter;
        Inkscape::GC::release(copy);
    }

    g_slist_free (child_reprs);
    g_slist_free (child_copies);
    g_slist_free (child_objects);
}

void sp_item_gradient_invert_vector_color(SPItem *item, Inkscape::PaintTarget fill_or_stroke)
{
#ifdef SP_GR_VERBOSE
    g_message("sp_item_gradient_invert_vector_color(%p, %d)", item, fill_or_stroke);
#endif
    SPGradient *gradient = getGradient(item, fill_or_stroke);
    if (!gradient || !SP_IS_GRADIENT(gradient))
        return;

    SPGradient *vector = gradient->getVector();
    if (!vector) // orphan!
        return;

    vector = sp_gradient_fork_vector_if_necessary (vector);
    if ( gradient != vector && gradient->ref->getObject() != vector ) {
        sp_gradient_repr_set_link(gradient->getRepr(), vector);
    }

    for ( SPObject *child = vector->firstChild(); child; child = child->getNext()) {
        if (SP_IS_STOP(child)) {
            guint32 color =  SP_STOP(child)->get_rgba32();
            //g_message("Stop color %d", color);
            gchar c[64];
            sp_svg_write_color (c, sizeof(c),
                SP_RGBA32_U_COMPOSE(
                        (255 - SP_RGBA32_R_U(color)),
                        (255 - SP_RGBA32_G_U(color)),
                        (255 - SP_RGBA32_B_U(color)),
                        SP_RGBA32_A_U(color)
                )
            );
            SPCSSAttr *css = sp_repr_css_attr_new ();
            sp_repr_css_set_property (css, "stop-color", c);
            sp_repr_css_change(child->getRepr(), css, "style");
            sp_repr_css_attr_unref (css);
        }
    }
}

/**
Set the position of point point_type of the gradient applied to item (either fill_or_stroke) to
p_w (in desktop coordinates). Write_repr if you want the change to become permanent.
*/
void sp_item_gradient_set_coords(SPItem *item, GrPointType point_type, guint point_i, Geom::Point p_w, Inkscape::PaintTarget fill_or_stroke, bool write_repr, bool scale)
{
#ifdef SP_GR_VERBOSE
    g_message("sp_item_gradient_set_coords(%p, %d, %d, (%f, %f), ...)", item, point_type, point_i, p_w[Geom::X], p_w[Geom::Y] );
#endif
    SPGradient *gradient = getGradient(item, fill_or_stroke);

    if (!gradient || !SP_IS_GRADIENT(gradient))
        return;

    // Needed only if units are set to SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX
    gradient = sp_gradient_convert_to_userspace(gradient, item, (fill_or_stroke == Inkscape::FOR_FILL) ? "fill" : "stroke");

    Geom::Affine i2d (item->i2dt_affine ());
    Geom::Point p = p_w * i2d.inverse();
    p *= (gradient->gradientTransform).inverse();
    // now p is in gradient's original coordinates

    Inkscape::XML::Node *repr = gradient->getRepr();

    if (SP_IS_LINEARGRADIENT(gradient)) {
        SPLinearGradient *lg = SP_LINEARGRADIENT(gradient);
        switch (point_type) {
            case POINT_LG_BEGIN:
                if (scale) {
                    lg->x2.computed += (lg->x1.computed - p[Geom::X]);
                    lg->y2.computed += (lg->y1.computed - p[Geom::Y]);
                }
                lg->x1.computed = p[Geom::X];
                lg->y1.computed = p[Geom::Y];
                if (write_repr) {
                    if (scale) {
                        sp_repr_set_svg_double(repr, "x2", lg->x2.computed);
                        sp_repr_set_svg_double(repr, "y2", lg->y2.computed);
                    }
                    sp_repr_set_svg_double(repr, "x1", lg->x1.computed);
                    sp_repr_set_svg_double(repr, "y1", lg->y1.computed);
                } else {
                    gradient->requestModified(SP_OBJECT_MODIFIED_FLAG);
                }
                break;
            case POINT_LG_END:
                if (scale) {
                    lg->x1.computed += (lg->x2.computed - p[Geom::X]);
                    lg->y1.computed += (lg->y2.computed - p[Geom::Y]);
                }
                lg->x2.computed = p[Geom::X];
                lg->y2.computed = p[Geom::Y];
                if (write_repr) {
                    if (scale) {
                        sp_repr_set_svg_double(repr, "x1", lg->x1.computed);
                        sp_repr_set_svg_double(repr, "y1", lg->y1.computed);
                    }
                    sp_repr_set_svg_double(repr, "x2", lg->x2.computed);
                    sp_repr_set_svg_double(repr, "y2", lg->y2.computed);
                } else {
                    gradient->requestModified(SP_OBJECT_MODIFIED_FLAG);
                }
                break;
            case POINT_LG_MID:
            {
                // using X-coordinates only to determine the offset, assuming p has been snapped to the vector from begin to end.
                Geom::Point begin(lg->x1.computed, lg->y1.computed);
                Geom::Point end(lg->x2.computed, lg->y2.computed);
                double offset = Geom::LineSegment(begin, end).nearestTime(p);
                SPGradient *vector = sp_gradient_get_forked_vector_if_necessary (lg, false);
                lg->ensureVector();
                lg->vector.stops.at(point_i).offset = offset;
                SPStop* stopi = sp_get_stop_i(vector, point_i);
                stopi->offset = offset;
                if (write_repr) {
                    sp_repr_set_css_double(stopi->getRepr(), "offset", stopi->offset);
                } else {
                    stopi->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
                }
            }
            break;
            default:
                g_warning( "Bad linear gradient handle type" );
                break;
        }
    } else if (SP_IS_RADIALGRADIENT(gradient)) {
        SPRadialGradient *rg = SP_RADIALGRADIENT(gradient);
        Geom::Point c (rg->cx.computed, rg->cy.computed);
        Geom::Point c_w = c * gradient->gradientTransform * i2d; // now in desktop coords
        if ((point_type == POINT_RG_R1 || point_type == POINT_RG_R2) && Geom::L2 (p_w - c_w) < 1e-3) {
            // prevent setting a radius too close to the center
            return;
        }
        Geom::Affine new_transform;
        bool transform_set = false;

        switch (point_type) {
            case POINT_RG_CENTER:
                rg->fx.computed = p[Geom::X] + (rg->fx.computed - rg->cx.computed);
                rg->fy.computed = p[Geom::Y] + (rg->fy.computed - rg->cy.computed);
                rg->cx.computed = p[Geom::X];
                rg->cy.computed = p[Geom::Y];
                if (write_repr) {
                    sp_repr_set_svg_double(repr, "fx", rg->fx.computed);
                    sp_repr_set_svg_double(repr, "fy", rg->fy.computed);
                    sp_repr_set_svg_double(repr, "cx", rg->cx.computed);
                    sp_repr_set_svg_double(repr, "cy", rg->cy.computed);
                } else {
                    gradient->requestModified(SP_OBJECT_MODIFIED_FLAG);
                }
                break;
            case POINT_RG_FOCUS:
                rg->fx.computed = p[Geom::X];
                rg->fy.computed = p[Geom::Y];
                if (write_repr) {
                    sp_repr_set_svg_double(repr, "fx", rg->fx.computed);
                    sp_repr_set_svg_double(repr, "fy", rg->fy.computed);
                } else {
                    gradient->requestModified(SP_OBJECT_MODIFIED_FLAG);
                }
                break;
            case POINT_RG_R1:
            {
                Geom::Point r1_w = (c + Geom::Point(rg->r.computed, 0)) * gradient->gradientTransform * i2d;
                double r1_angle = Geom::atan2(r1_w - c_w);
                double move_angle = Geom::atan2(p_w - c_w) - r1_angle;
                double move_stretch = Geom::L2(p_w - c_w) / Geom::L2(r1_w - c_w);

                Geom::Affine move = Geom::Affine (Geom::Translate (-c_w)) *
                    Geom::Affine (Geom::Rotate(-r1_angle)) *
                    Geom::Affine (Geom::Scale(move_stretch, scale? move_stretch : 1)) *
                    Geom::Affine (Geom::Rotate(r1_angle)) *
                    Geom::Affine (Geom::Rotate(move_angle)) *
                    Geom::Affine (Geom::Translate (c_w));

                new_transform = gradient->gradientTransform * i2d * move * i2d.inverse();
                transform_set = true;

                break;
            }
            case POINT_RG_R2:
            {
                Geom::Point r2_w = (c + Geom::Point(0, -rg->r.computed)) * gradient->gradientTransform * i2d;
                double r2_angle = Geom::atan2(r2_w - c_w);
                double move_angle = Geom::atan2(p_w - c_w) - r2_angle;
                double move_stretch = Geom::L2(p_w - c_w) / Geom::L2(r2_w - c_w);

                Geom::Affine move = Geom::Affine (Geom::Translate (-c_w)) *
                    Geom::Affine (Geom::Rotate(-r2_angle)) *
                    Geom::Affine (Geom::Scale(move_stretch, scale? move_stretch : 1)) *
                    Geom::Affine (Geom::Rotate(r2_angle)) *
                    Geom::Affine (Geom::Rotate(move_angle)) *
                    Geom::Affine (Geom::Translate (c_w));

                new_transform = gradient->gradientTransform * i2d * move * i2d.inverse();
                transform_set = true;

                break;
            }
            case POINT_RG_MID1:
            {
                Geom::Point start = Geom::Point (rg->cx.computed, rg->cy.computed);
                Geom::Point end   = Geom::Point (rg->cx.computed + rg->r.computed, rg->cy.computed);
                double offset = Geom::LineSegment(start, end).nearestTime(p);
                SPGradient *vector = sp_gradient_get_forked_vector_if_necessary (rg, false);
                rg->ensureVector();
                rg->vector.stops.at(point_i).offset = offset;
                SPStop* stopi = sp_get_stop_i(vector, point_i);
                stopi->offset = offset;
                if (write_repr) {
                    sp_repr_set_css_double(stopi->getRepr(), "offset", stopi->offset);
                } else {
                    stopi->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
                }
                break;
            }
            case POINT_RG_MID2:
            {
                Geom::Point start = Geom::Point (rg->cx.computed, rg->cy.computed);
                Geom::Point end   = Geom::Point (rg->cx.computed, rg->cy.computed - rg->r.computed);
                double offset = Geom::LineSegment(start, end).nearestTime(p);
                SPGradient *vector = sp_gradient_get_forked_vector_if_necessary(rg, false);
                rg->ensureVector();
                rg->vector.stops.at(point_i).offset = offset;
                SPStop* stopi = sp_get_stop_i(vector, point_i);
                stopi->offset = offset;
                if (write_repr) {
                    sp_repr_set_css_double(stopi->getRepr(), "offset", stopi->offset);
                } else {
                    stopi->requestModified(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
                }
                break;
            }
            default:
                g_warning( "Bad radial gradient handle type" );
                break;
        }

        if (transform_set) {
            gradient->gradientTransform = new_transform;
            gradient->gradientTransform_set = TRUE;
            if (write_repr) {
                gchar *s=sp_svg_transform_write(gradient->gradientTransform);
                gradient->getRepr()->setAttribute("gradientTransform", s);
                g_free(s);
            } else {
                gradient->requestModified(SP_OBJECT_MODIFIED_FLAG);
            }
        }
    } else if (SP_IS_MESH(gradient)) {
        SPMesh *mg = SP_MESH(gradient);
        //Geom::Affine new_transform;
        //bool transform_set = false;

        switch (point_type) {
            case POINT_MG_CORNER:
            {
                mg->array.corners[ point_i ]->p = p;
                // Handles are moved in gradient-drag.cpp
                gradient->requestModified(SP_OBJECT_MODIFIED_FLAG);
                break;
            }

            case POINT_MG_HANDLE: {
                mg->array.handles[ point_i ]->p = p;
                gradient->requestModified(SP_OBJECT_MODIFIED_FLAG);
                break;
            }

            case POINT_MG_TENSOR: {
                mg->array.tensors[ point_i ]->p = p;
                gradient->requestModified(SP_OBJECT_MODIFIED_FLAG);
                break;
            }

            default:
                g_warning( "Bad mesh handle type" );
        }
        if( write_repr ) {
            //std::cout << "Write mesh repr" << std::endl;
            sp_mesh_repr_write( mg );
        }
    }

}

SPGradient *sp_item_gradient_get_vector(SPItem *item, Inkscape::PaintTarget fill_or_stroke)
{
    SPGradient *gradient = getGradient(item, fill_or_stroke);

    if (gradient) {
        return gradient->getVector();
    }
    return NULL;
}

SPGradientSpread sp_item_gradient_get_spread(SPItem *item, Inkscape::PaintTarget fill_or_stroke)
{
    SPGradientSpread spread = SP_GRADIENT_SPREAD_PAD;
    SPGradient *gradient = getGradient(item, fill_or_stroke);

    if (gradient) {
        spread = gradient->fetchSpread();
    }
    return spread;
}


/**
Returns the position of point point_type of the gradient applied to item (either fill_or_stroke),
in desktop coordinates.
*/
Geom::Point getGradientCoords(SPItem *item, GrPointType point_type, guint point_i, Inkscape::PaintTarget fill_or_stroke)
{
#ifdef SP_GR_VERBOSE
    g_message("getGradientCoords(%p, %d, %d, %d)", item, point_type, point_i, fill_or_stroke);
#endif
    SPGradient *gradient = getGradient(item, fill_or_stroke);

    Geom::Point p (0, 0);

    if (!gradient)
        return p;

    if (SP_IS_LINEARGRADIENT(gradient)) {
        SPLinearGradient *lg = SP_LINEARGRADIENT(gradient);
        switch (point_type) {
            case POINT_LG_BEGIN:
                p = Geom::Point (lg->x1.computed, lg->y1.computed);
                break;
            case POINT_LG_END:
                p = Geom::Point (lg->x2.computed, lg->y2.computed);
                break;
            case POINT_LG_MID:
                {
                    if (lg->vector.stops.size() < point_i) {
                        g_message("POINT_LG_MID bug trigger, see LP bug #453067");
                        break;
                    }
                    gdouble offset = lg->vector.stops.at(point_i).offset;
                    p = (1-offset) * Geom::Point(lg->x1.computed, lg->y1.computed) + offset * Geom::Point(lg->x2.computed, lg->y2.computed);
                }
                break;
            default:
                g_warning( "Bad linear gradient handle type" );
                break;
        }
    } else     if (SP_IS_RADIALGRADIENT(gradient)) {
        SPRadialGradient *rg = SP_RADIALGRADIENT(gradient);
        switch (point_type) {
            case POINT_RG_CENTER:
                p = Geom::Point (rg->cx.computed, rg->cy.computed);
                break;
            case POINT_RG_FOCUS:
                p = Geom::Point (rg->fx.computed, rg->fy.computed);
                break;
            case POINT_RG_R1:
                p = Geom::Point (rg->cx.computed + rg->r.computed, rg->cy.computed);
                break;
            case POINT_RG_R2:
                p = Geom::Point (rg->cx.computed, rg->cy.computed - rg->r.computed);
                break;
            case POINT_RG_MID1:
                {
                    if (rg->vector.stops.size() < point_i) {
                        g_message("POINT_RG_MID1 bug trigger, see LP bug #453067");
                        break;
                    }
                    gdouble offset = rg->vector.stops.at(point_i).offset;
                    p = (1-offset) * Geom::Point (rg->cx.computed, rg->cy.computed) + offset * Geom::Point(rg->cx.computed + rg->r.computed, rg->cy.computed);
                }
                break;
            case POINT_RG_MID2:
                {
                    if (rg->vector.stops.size() < point_i) {
                        g_message("POINT_RG_MID2 bug trigger, see LP bug #453067");
                        break;
                    }
                    gdouble offset = rg->vector.stops.at(point_i).offset;
                    p = (1-offset) * Geom::Point (rg->cx.computed, rg->cy.computed) + offset * Geom::Point(rg->cx.computed, rg->cy.computed - rg->r.computed);
                }
                break;
            default:
                g_warning( "Bad radial gradient handle type" );
                break;
        }
    } else     if (SP_IS_MESH(gradient)) {
        SPMesh *mg = SP_MESH(gradient);
        switch (point_type) {

            case POINT_MG_CORNER:
                p = mg->array.corners[ point_i ]->p;
                break;

            case POINT_MG_HANDLE: {
                p = mg->array.handles[ point_i ]->p;
                break;
            }

            case POINT_MG_TENSOR: {
                p = mg->array.tensors[ point_i ]->p;
                break;
            }

            default:
                g_warning( "Bad mesh handle type" );
        }
    }


    if (SP_GRADIENT(gradient)->getUnits() == SP_GRADIENT_UNITS_OBJECTBOUNDINGBOX) {
        item->document->ensureUpToDate();
        Geom::OptRect bbox = item->visualBounds(); // we need "true" bbox without item_i2d_affine
        if (bbox) {
            p *= Geom::Affine(bbox->dimensions()[Geom::X], 0,
                            0, bbox->dimensions()[Geom::Y],
                            bbox->min()[Geom::X], bbox->min()[Geom::Y]);
        }
    }
    p *= Geom::Affine(gradient->gradientTransform) * (Geom::Affine)item->i2dt_affine();
    return p;
}

/**
 * Sets item fill or stroke to the gradient of the specified type with given vector, creating
 * new private gradient, if needed.
 * gr has to be a normalized vector.
 */

SPGradient *sp_item_set_gradient(SPItem *item, SPGradient *gr, SPGradientType type, Inkscape::PaintTarget fill_or_stroke)
{
#ifdef SP_GR_VERBOSE
    g_message("sp_item_set_gradient(%p, %p, %d, %d)", item, gr, type, fill_or_stroke);
#endif
    g_return_val_if_fail(item != NULL, NULL);
    g_return_val_if_fail(SP_IS_ITEM(item), NULL);
    g_return_val_if_fail(gr != NULL, NULL);
    g_return_val_if_fail(SP_IS_GRADIENT(gr), NULL);
    g_return_val_if_fail(gr->state == SP_GRADIENT_STATE_VECTOR, NULL);

    SPStyle *style = item->style;
    g_assert(style != NULL);

    SPPaintServer *ps = NULL;
    if ((fill_or_stroke == Inkscape::FOR_FILL) ? style->fill.isPaintserver() : style->stroke.isPaintserver()) {
        ps = (fill_or_stroke == Inkscape::FOR_FILL) ? SP_STYLE_FILL_SERVER(style) : SP_STYLE_STROKE_SERVER(style);
    }

    if (ps
        && ( (type == SP_GRADIENT_TYPE_LINEAR && SP_IS_LINEARGRADIENT(ps)) ||
             (type == SP_GRADIENT_TYPE_RADIAL && SP_IS_RADIALGRADIENT(ps))   ) )
    {

        /* Current fill style is the gradient of the required type */
        SPGradient *current = SP_GRADIENT(ps);

        //g_message("hrefcount %d   count %d\n", current->hrefcount, count_gradient_hrefs(item, current));

        if (!current->isSwatch()
            && (current->hrefcount == 1 ||
            current->hrefcount == count_gradient_hrefs(item, current))) {

            // current is private and it's either used once, or all its uses are by children of item;
            // so just change its href to vector

            if ( current != gr && current->getVector() != gr ) {
                // href is not the vector
                sp_gradient_repr_set_link(current->getRepr(), gr);
            }
            item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
            return current;

        } else {

            // the gradient is not private, or it is shared with someone else;
            // normalize it (this includes creating new private if necessary)
            SPGradient *normalized = sp_gradient_fork_private_if_necessary(current, gr, type, item);

            g_return_val_if_fail(normalized != NULL, NULL);

            if (normalized != current) {

                /* We have to change object style here; recursive because this is used from
                 * fill&stroke and must work for groups etc. */
                sp_style_set_property_url(item, (fill_or_stroke == Inkscape::FOR_FILL) ? "fill" : "stroke", normalized, true);
            }
            item->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG);
            return normalized;
        }

    } else {
        /* Current fill style is not a gradient or wrong type, so construct everything */
        /* This is where mesh gradients are constructed. */
        g_assert(SP_IS_GRADIENT(gr)); // TEMP
        SPGradient *constructed = sp_gradient_get_private_normalized(item->document, gr, type);
        constructed = sp_gradient_reset_to_userspace(constructed, item);
        sp_style_set_property_url(item, ( (fill_or_stroke == Inkscape::FOR_FILL) ? "fill" : "stroke" ), constructed, true);
        item->requestDisplayUpdate(( SP_OBJECT_MODIFIED_FLAG |
                                     SP_OBJECT_STYLE_MODIFIED_FLAG ));
        return constructed;
    }
}

static void sp_gradient_repr_set_link(Inkscape::XML::Node *repr, SPGradient *link)
{
#ifdef SP_GR_VERBOSE
    g_message("sp_gradient_repr_set_link(%p, %p)", repr, link);
#endif
    g_return_if_fail(repr != NULL);
    if (link) {
        g_return_if_fail(SP_IS_GRADIENT(link));
    }

    if (link) {
        Glib::ustring ref("#");
        ref += link->getId();
        repr->setAttribute("xlink:href", ref.c_str());
    } else {
        repr->setAttribute("xlink:href", 0);
    }
}


static void addStop( Inkscape::XML::Node *parent, Glib::ustring const &color, gint opacity, gchar const *offset )
{
#ifdef SP_GR_VERBOSE
    g_message("addStop(%p, %s, %d, %s)", parent, color.c_str(), opacity, offset);
#endif
    Inkscape::XML::Node *stop = parent->document()->createElement("svg:stop");
    {
        gchar *tmp = g_strdup_printf( "stop-color:%s;stop-opacity:%d;", color.c_str(), opacity );
        stop->setAttribute( "style", tmp );
        g_free(tmp);
    }

    stop->setAttribute( "offset", offset );

    parent->appendChild(stop);
    Inkscape::GC::release(stop);
}

/*
 * Get default normalized gradient vector of document, create if there is none
 */
SPGradient *sp_document_default_gradient_vector( SPDocument *document, SPColor const &color, bool singleStop )
{
    SPDefs *defs = document->getDefs();
    Inkscape::XML::Document *xml_doc = document->rdoc;

    Inkscape::XML::Node *repr = xml_doc->createElement("svg:linearGradient");

    if ( !singleStop ) {
        repr->setAttribute("inkscape:collect", "always");
        // set here, but removed when it's edited in the gradient editor
        // to further reduce clutter, we could
        // (1) here, search gradients by color and return what is found without duplication
        // (2) in fill & stroke, show only one copy of each gradient in list
    }

    Glib::ustring colorStr = color.toString();
    addStop( repr, colorStr, 1, "0" );
    if ( !singleStop ) {
        addStop( repr, colorStr, 0, "1" );
    }

    defs->getRepr()->addChild(repr, NULL);
    Inkscape::GC::release(repr);

    /* fixme: This does not look like nice */
    SPGradient *gr = static_cast<SPGradient *>(document->getObjectByRepr(repr));
    g_assert(gr != NULL);
    g_assert(SP_IS_GRADIENT(gr));
    /* fixme: Maybe add extra sanity check here */
    gr->state = SP_GRADIENT_STATE_VECTOR;

    return gr;
}

SPGradient *sp_gradient_vector_for_object( SPDocument *const doc, SPDesktop *const desktop,
                                           SPObject *const o, Inkscape::PaintTarget const fill_or_stroke, bool singleStop )
{
    SPColor color;
    if ( (o == NULL) || (o->style == NULL) ) {
        color = sp_desktop_get_color(desktop, (fill_or_stroke == Inkscape::FOR_FILL));
    } else {
        // take the color of the object
        SPStyle const &style = *(o->style);
        SPIPaint const &paint = ( (fill_or_stroke == Inkscape::FOR_FILL) ? style.fill : style.stroke );
        if (paint.isPaintserver()) {
            SPObject *server = (fill_or_stroke == Inkscape::FOR_FILL) ? o->style->getFillPaintServer() : o->style->getStrokePaintServer();
            if ( SP_IS_GRADIENT(server) ) {
                return SP_GRADIENT(server)->getVector(true);
            } else {
                color = sp_desktop_get_color(desktop, (fill_or_stroke == Inkscape::FOR_FILL));
            }
        } else if (paint.isColor()) {
            color = paint.value.color;
        } else {
            // if o doesn't use flat color, then take current color of the desktop.
            color = sp_desktop_get_color(desktop, (fill_or_stroke == Inkscape::FOR_FILL));
        }
    }

    return sp_document_default_gradient_vector( doc, color, singleStop );
}

void sp_gradient_invert_selected_gradients(SPDesktop *desktop, Inkscape::PaintTarget fill_or_stroke)
{
    Inkscape::Selection *selection = desktop->getSelection();

    const std::vector<SPItem*> list=selection->itemList();
    for (std::vector<SPItem*>::const_iterator i = list.begin(); i != list.end(); i++) {
        sp_item_gradient_invert_vector_color(*i, fill_or_stroke);
    }

    // we did an undoable action
    DocumentUndo::done(desktop->getDocument(), SP_VERB_CONTEXT_GRADIENT,
                       _("Invert gradient colors"));
}

void sp_gradient_reverse_selected_gradients(SPDesktop *desktop)
{
    Inkscape::Selection *selection = desktop->getSelection();
    Inkscape::UI::Tools::ToolBase *ev = desktop->getEventContext();

    if (!ev) {
        return;
    }

    GrDrag *drag = ev->get_drag();

    // First try selected dragger
    if (drag && drag->selected) {
        drag->selected_reverse_vector();
    } else { // If no drag or no dragger selected, act on selection (both fill and stroke gradients)
        const std::vector<SPItem*> list=selection->itemList();
        for (std::vector<SPItem*>::const_iterator i = list.begin(); i != list.end(); i++) {
            sp_item_gradient_reverse_vector(*i, Inkscape::FOR_FILL);
            sp_item_gradient_reverse_vector(*i, Inkscape::FOR_STROKE);
        }
    }

    // we did an undoable action
    DocumentUndo::done(desktop->getDocument(), SP_VERB_CONTEXT_GRADIENT,
                       _("Reverse gradient"));
}

void sp_gradient_unset_swatch(SPDesktop *desktop, std::string id)
{
    SPDocument *doc = desktop ? desktop->doc() : 0;

    if (doc) {
        const GSList *gradients = doc->getResourceList("gradient");
        for (const GSList *item = gradients; item; item = item->next) {
            SPGradient* grad = SP_GRADIENT(item->data);
            if ( id == grad->getId() ) {
                grad->setSwatch(false);
                DocumentUndo::done(doc, SP_VERB_CONTEXT_GRADIENT,
                                   _("Delete swatch"));
                break;
            }
        }
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
