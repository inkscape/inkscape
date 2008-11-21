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


#include <cstring>
#include <string>

#include "sp-item.h"
#include "conn-avoid-ref.h"
#include "libavoid/polyutil.h"
#include "libavoid/router.h"
#include "libavoid/connector.h"
#include "xml/node.h"
#include "document.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "sp-namedview.h"
#include "inkscape.h"


using Avoid::Router;

static Avoid::Polygn avoid_item_poly(SPItem const *item);


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
        Router *router = shapeRef->router();
        // shapeRef is finalised by delShape,
        // so no memory is lost here.
        router->delShape(shapeRef);
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
    SPDesktop *desktop = inkscape_active_desktop();
    if (desktop == NULL) {
        return;
    }
    if (sp_desktop_document(desktop) != item->document) {
        // We don't want to go any further if the active desktop's document
        // isn't the same as the document that this item is part of.  This
        // case can happen if a new document is loaded from the file chooser
        // or via the recent file menu.  In this case, we can end up here
        // as a rersult of a sp_document_ensure_up_to_date performed on a
        // document not yet attached to the active desktop.
        return;
    }

    if (new_setting == setting) {
        // Don't need to make any changes
        return;
    }
    setting = new_setting;

    Router *router = item->document->router;
    
    _transformed_connection.disconnect();
    if (new_setting) {
        Avoid::Polygn poly = avoid_item_poly(item);
        if (poly.pn > 0) {
            _transformed_connection = item->connectTransformed(
                    sigc::ptr_fun(&avoid_item_move));

            const char *id = SP_OBJECT_REPR(item)->attribute("id");
            g_assert(id != NULL);
            
            // Get a unique ID for the item.
            GQuark itemID = g_quark_from_string(id);

            shapeRef = new Avoid::ShapeRef(router, itemID, poly);
            Avoid::freePoly(poly);
        
            router->addShape(shapeRef);
        }
    }
    else
    {
        g_assert(shapeRef);
        
        // shapeRef is finalised by delShape,
        // so no memory is lost here.
        router->delShape(shapeRef);
        shapeRef = NULL;
    }
}


GSList *SPAvoidRef::getAttachedShapes(const unsigned int type)
{
    GSList *list = NULL;

    Avoid::IntList shapes;
    GQuark shapeId = g_quark_from_string(item->id);
    item->document->router->attachedShapes(shapes, shapeId, type);
    
    Avoid::IntList::iterator finish = shapes.end();
    for (Avoid::IntList::iterator i = shapes.begin(); i != finish; ++i) {
        const gchar *connId = g_quark_to_string(*i);
        SPObject *obj = item->document->getObjectById(connId);
        if (obj == NULL) {
            g_warning("getAttachedShapes: Object with id=\"%s\" is not "
                    "found. Skipping.", connId);
            continue;
        }
        SPItem *shapeItem = SP_ITEM(obj);
        list = g_slist_prepend(list, shapeItem);
    }
    return list;
}


GSList *SPAvoidRef::getAttachedConnectors(const unsigned int type)
{
    GSList *list = NULL;

    Avoid::IntList conns;
    GQuark shapeId = g_quark_from_string(item->id);
    item->document->router->attachedConns(conns, shapeId, type);
    
    Avoid::IntList::iterator finish = conns.end();
    for (Avoid::IntList::iterator i = conns.begin(); i != finish; ++i) {
        const gchar *connId = g_quark_to_string(*i);
        SPObject *obj = item->document->getObjectById(connId);
        if (obj == NULL) {
            g_warning("getAttachedConnectors: Object with id=\"%s\" is not "
                    "found. Skipping.", connId);
            continue;
        }
        SPItem *connItem = SP_ITEM(obj);
        list = g_slist_prepend(list, connItem);
    }
    return list;
}


static Avoid::Polygn avoid_item_poly(SPItem const *item)
{
    SPDesktop *desktop = inkscape_active_desktop();
    g_assert(desktop != NULL);

    Avoid::Polygn poly;

    // TODO: The right way to do this is to return the convex hull of
    //       the object, or an approximation in the case of a rounded
    //       object.  Specific SPItems will need to have a new
    //       function that returns points for the convex hull.
    //       For some objects it is enough to feed the snappoints to
    //       some convex hull code, though not NR::ConvexHull as this
    //       only keeps the bounding box of the convex hull currently.

    // TODO: SPItem::getBounds gives the wrong result for some objects
    //       that have internal representations that are updated later
    //       by the sp_*_update functions, e.g., text.
    sp_document_ensure_up_to_date(item->document);
    
    Geom::OptRect rHull = item->getBounds(sp_item_i2doc_affine(item));
    if (!rHull) {
        return Avoid::newPoly(0);
    }

    double spacing = desktop->namedview->connector_spacing;

    // Add a little buffer around the edge of each object.
    Geom::Rect rExpandedHull = *rHull;
    rExpandedHull.expandBy(-spacing); 
    poly = Avoid::newPoly(4);

    for (unsigned n = 0; n < 4; ++n) {
        Geom::Point hullPoint = rExpandedHull.corner(n);
        poly.ps[n].x = hullPoint[Geom::X];
        poly.ps[n].y = hullPoint[Geom::Y];
    }

    return poly;
}


GSList *get_avoided_items(GSList *list, SPObject *from, SPDesktop *desktop, 
        bool initialised)
{
    for (SPObject *child = sp_object_first_child(SP_OBJECT(from)) ;
            child != NULL; child = SP_OBJECT_NEXT(child) ) {
        if (SP_IS_ITEM(child) &&
            !desktop->isLayer(SP_ITEM(child)) &&
            !SP_ITEM(child)->isLocked() && 
            !desktop->itemIsHidden(SP_ITEM(child)) &&
            (!initialised || SP_ITEM(child)->avoidRef->shapeRef)
            )
        {
            list = g_slist_prepend (list, SP_ITEM(child));
        }

        if (SP_IS_ITEM(child) && desktop->isLayer(SP_ITEM(child))) {
            list = get_avoided_items(list, child, desktop, initialised);
        }
    }

    return list;
}


void avoid_item_move(Geom::Matrix const */*mp*/, SPItem *moved_item)
{
    Avoid::ShapeRef *shapeRef = moved_item->avoidRef->shapeRef;
    g_assert(shapeRef);

    Router *router = moved_item->document->router;
    Avoid::Polygn poly = avoid_item_poly(moved_item);
    if (poly.pn > 0) {
        router->moveShape(shapeRef, &poly);
        Avoid::freePoly(poly);
    }
}


void init_avoided_shape_geometry(SPDesktop *desktop)
{
    // Don't count this as changes to the document,
    // it is basically just llate initialisation.
    SPDocument *document = sp_desktop_document(desktop);
    bool saved = sp_document_get_undo_sensitive(document);
    sp_document_set_undo_sensitive(document, false);
    
    bool initialised = false;
    GSList *items = get_avoided_items(NULL, desktop->currentRoot(), desktop,
            initialised);

    for ( GSList const *iter = items ; iter != NULL ; iter = iter->next ) {
        SPItem *item = reinterpret_cast<SPItem *>(iter->data);
        item->avoidRef->handleSettingChange();
    }

    if (items) {
        g_slist_free(items);
    }
    sp_document_set_undo_sensitive(document, saved);
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
