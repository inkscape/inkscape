/*
 * A class for handling shape interaction with libavoid.
 *
 * Authors:
 *   Michael Wybrow <mjwybrow@users.sourceforge.net>
 *
 * Copyright (C) 2005 Michael Wybrow
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */



#include "sp-item.h"
#include "conn-avoid-ref.h"
#include "libnr/nr-rect-ops.h"
#include "libavoid/polyutil.h"
#include "libavoid/incremental.h"
#include "xml/simple-node.cpp"
#include "document.h"


static Avoid::Polygn avoid_item_poly(SPItem const *item);
static void avoid_item_move(NR::Matrix const *mp, SPItem *moved_item);


SPAvoidRef::SPAvoidRef(SPItem *spitem)
    : shapeRef(NULL)
    , item(spitem)
    , setting(false)
    , new_setting(false)
    , _transformed_connection()
{
}


SPAvoidRef::~SPAvoidRef()
{
    _transformed_connection.disconnect();
    if (shapeRef) {
        // shapeRef is finalised by delShape,
        // so no memory is lost here.
        Avoid::delShape(shapeRef);
        shapeRef = NULL;
    }
}


void SPAvoidRef::setAvoid(char const *value)
{
    if (SP_OBJECT_IS_CLONED(item)) {
        // Don't keep avoidance information for cloned objects.
        return;
    }
    new_setting = false;
    if (value && (strcmp(value, "true") == 0)) {
        new_setting = true;
    }
}


void SPAvoidRef::handleSettingChange(void)
{
    if (new_setting == setting) {
        // Don't need to make any changes
        return;
    }

    _transformed_connection.disconnect();
    if (new_setting) {
        _transformed_connection = item->connectTransformed(
                sigc::ptr_fun(&avoid_item_move));

        Avoid::Polygn poly = avoid_item_poly(item);
        if (poly.pn > 0) {
            const char *id = SP_OBJECT_REPR(item)->attribute("id");
            g_assert(id != NULL);
            
            // Get a unique ID for the item.
            GQuark itemID = g_quark_from_string(id);

            shapeRef = new Avoid::ShapeRef(itemID, poly);
            Avoid::freePoly(poly);
        
            Avoid::addShape(shapeRef);
        }
    }
    else
    {
        g_assert(shapeRef);
        
        // shapeRef is finalised by delShape,
        // so no memory is lost here.
        Avoid::delShape(shapeRef);
        shapeRef = NULL;
    }
    setting = new_setting;
}


static Avoid::Polygn avoid_item_poly(SPItem const *item)
{
    Avoid::Polygn poly;

    // TODO: The right way to do this is to return the convex hull of
    //       the object, or an approximation in the case of a rounded
    //       object.  Specific SPItems will need to have a new
    //       function that returns points for the convex hull.
    //       For some objects it is enough to feed the snappoints to
    //       some convex hull code, though not NR::ConvexHull as this
    //       only keeps the bounding box of the convex hull currently.

    // TODO: SPItem::invokeBbox gives the wrong result for some objects
    //       that have internal representations that are updated later
    //       by the sp_*_update functions, e.g., text.
    sp_document_ensure_up_to_date(item->document);
    
    NR::Rect rHull = item->invokeBbox(sp_item_i2doc_affine(item));
    
    // Add a little buffer around the edge of each object.
    NR::Rect rExpandedHull = NR::expand(rHull, -10.0); 
    poly = Avoid::newPoly(4);

    for (unsigned n = 0; n < 4; ++n) {
        // TODO: I think the winding order in libavoid or inkscape might
        //       be backwards, probably due to the inverse y co-ordinates
        //       used for the screen.  The '3 - n' reverses the order.
        /* On "correct" winding order: Winding order of NR::Rect::corner is in a positive
         * direction, like libart.  "Positive direction" means the same as in most of Inkscape and
         * SVG: if you visualize y as increasing upwards, as is the convention in mathematics, then
         * positive angle is visualized as anticlockwise, as in mathematics; so if you visualize y
         * as increasing downwards, as is common outside of mathematics, then positive angle
         * direction is visualized as clockwise, as is common outside of mathematics.  This
         * convention makes it easier mix pure mathematics code with graphics code: the important
         * thing when mixing code is that the number values stored in variables (representing y
         * coordinate, angle) match up; variables store numbers, not visualized positions, and the
         * programmer is free to switch between visualizations when thinking about a given piece of
         * code.
         *
         * MathWorld, libart and NR::Rect::corner all seem to take positive winding (i.e. winding
         * that yields +1 winding number inside a simple closed shape) to mean winding in a
         * positive angle.  This, together with the observation that variables store numbers rather
         * than positions, suggests that NR::Rect::corner uses the right direction.
         */
        NR::Point hullPoint = rExpandedHull.corner(3 - n);
        poly.ps[n].x = hullPoint[NR::X];
        poly.ps[n].y = hullPoint[NR::Y];
    }

    return poly;
}


static void avoid_item_move(NR::Matrix const *mp, SPItem *moved_item)
{
    Avoid::ShapeRef *shapeRef = moved_item->avoidRef->shapeRef;
    g_assert(shapeRef);

    Avoid::Polygn poly = avoid_item_poly(moved_item);
    if (poly.pn > 0) {
        // moveShape actually destroys the old shapeRef and returns a new one.
        moved_item->avoidRef->shapeRef = Avoid::moveShape(shapeRef, &poly);
        Avoid::freePoly(poly);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
