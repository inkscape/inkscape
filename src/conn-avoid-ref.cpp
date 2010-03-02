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
#include <iostream>

#include "sp-item.h"
#include "display/curve.h"
#include "2geom/line.h"
#include "2geom/crossing.h"
#include "2geom/convex-cover.h"
#include "helper/geom-curves.h"
#include "svg/stringstream.h"
#include "conn-avoid-ref.h"
#include "connection-points.h"
#include "sp-conn-end.h"
#include "sp-path.h"
#include "libavoid/router.h"
#include "libavoid/connector.h"
#include "libavoid/geomtypes.h"
#include "xml/node.h"
#include "document.h"
#include "desktop.h"
#include "desktop-handles.h"
#include "sp-namedview.h"
#include "sp-item-group.h"
#include "inkscape.h"
#include <glibmm/i18n.h>



using Avoid::Router;

static Avoid::Polygon avoid_item_poly(SPItem const *item);


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

    // If the document is being destroyed then the router instance
    // and the ShapeRefs will have been destroyed with it.
    const bool routerInstanceExists = (item->document->router != NULL);

    if (shapeRef && routerInstanceExists) {
        Router *router = shapeRef->router();
        router->removeShape(shapeRef);
        delete shapeRef;
    }
    shapeRef = NULL;
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

void print_connection_points(std::map<int, ConnectionPoint>& cp)
{
    std::map<int, ConnectionPoint>::iterator i;
    for (i=cp.begin(); i!=cp.end(); ++i)
    {
        const ConnectionPoint& p = i->second;
        std::cout<<p.id<<" "<<p.type<<" "<<p.pos[Geom::X]<<" "<<p.pos[Geom::Y]<<std::endl;
    }
}

void SPAvoidRef::setConnectionPoints(gchar const *value)
{
    std::set<int> updates;
    std::set<int> deletes;
    std::set<int> seen;

    if (value)
    {
        /* Rebuild the connection points list.
           Update the connectors for which
           the endpoint has changed.
        */

        gchar ** strarray = g_strsplit(value, "|", 0);
        gchar ** iter = strarray;

        while (*iter != NULL) {
            ConnectionPoint cp;
            Inkscape::SVGIStringStream is(*iter);
            is>>cp;
            cp.type = ConnPointUserDefined;

            /* Mark this connection point as seen, so we can delete
               the other ones.
            */
            seen.insert(cp.id);
            if ( connection_points.find(cp.id) != connection_points.end() )
            {
                /* An already existing connection point.
                   Check to see if changed, and, if it is
                   the case, trigger connector update for
                   the connector attached to this connection
                   point. This is done by adding the
                   connection point to a list of connection
                   points to be updated.
                */
                if ( connection_points[cp.id] != cp )
                    // The connection point got updated.
                    // Put it in the update list.
                    updates.insert(cp.id);
            }
            connection_points[cp.id] = cp;
            ++iter;
        }
        /* Delete the connection points that didn't appear
           in the new connection point list.
        */
        std::map<int, ConnectionPoint>::iterator it;

        for (it=connection_points.begin(); it!=connection_points.end(); ++it)
            if ( seen.find(it->first) == seen.end())
                deletes.insert(it->first);
        g_strfreev(strarray);
    }
    else
    {
        /* Delete all the user-defined connection points
           Actually we do this by adding them to the list
           of connection points to be deleted.
        */
        std::map<int, ConnectionPoint>::iterator it;

        for (it=connection_points.begin(); it!=connection_points.end(); ++it)
            deletes.insert(it->first);
    }
    /* Act upon updates and deletes.
    */
    if (deletes.empty() && updates.empty())
        // Nothing to do, just return.
        return;
    // Get a list of attached connectors.
    GSList* conns = getAttachedConnectors(Avoid::runningToAndFrom);
    for (GSList *i = conns; i != NULL; i = i->next)
    {
        SPPath* path = SP_PATH(i->data);
        SPConnEnd** connEnds = path->connEndPair.getConnEnds();
        for (int ix=0; ix<2; ++ix) {
            if (connEnds[ix]->type == ConnPointUserDefined) {
                if (updates.find(connEnds[ix]->id) != updates.end()) {
                    if (path->connEndPair.isAutoRoutingConn()) {
                        path->connEndPair.tellLibavoidNewEndpoints();
                    } else {
                    }
                }
                else if (deletes.find(connEnds[ix]->id) != deletes.end()) {
                    sp_conn_end_detach(path, ix);
                }
            }
        }
    }
    g_slist_free(conns);
    // Remove all deleted connection points
    if (deletes.size())
        for (std::set<int>::iterator it = deletes.begin(); it != deletes.end(); ++it)
            connection_points.erase(*it);
}

void SPAvoidRef::setConnectionPointsAttrUndoable(const gchar* value, const gchar* action)
{
    SPDocument* doc = SP_OBJECT_DOCUMENT(item);

    sp_object_setAttribute( SP_OBJECT(item), "inkscape:connection-points", value, 0 );
    item->updateRepr();
    sp_document_ensure_up_to_date(doc);
    sp_document_done(doc, SP_VERB_CONTEXT_CONNECTOR, action);
}

void SPAvoidRef::addConnectionPoint(ConnectionPoint &cp)
{
    Inkscape::SVGOStringStream ostr;
    bool first = true;
    int newId = 1;
    if ( connection_points.size() )
    {
        for (IdConnectionPointMap::iterator it = connection_points.begin(); ; )
        {
            if ( first )
            {
                first = false;
                ostr<<it->second;
            }
            else
                ostr<<'|'<<it->second;
            IdConnectionPointMap::iterator prev_it = it;
            ++it;
            if ( it == connection_points.end() || prev_it->first + 1 != it->first )
            {
                newId = prev_it->first + 1;
                break;
            }
        }
    }
    cp.id = newId;
    if ( first )
    {
        first = false;
        ostr<<cp;
    }
    else
        ostr<<'|'<<cp;

    this->setConnectionPointsAttrUndoable( ostr.str().c_str(), _("Add a new connection point") );
}

void SPAvoidRef::updateConnectionPoint(ConnectionPoint &cp)
{
    Inkscape::SVGOStringStream ostr;
    IdConnectionPointMap::iterator cp_pos = connection_points.find( cp.id );
    if ( cp_pos != connection_points.end() )
    {
        bool first = true;
        for (IdConnectionPointMap::iterator it = connection_points.begin(); it != connection_points.end(); ++it)
        {
            ConnectionPoint* to_write;
            if ( it != cp_pos )
                to_write = &it->second;
            else
                to_write = &cp;
            if ( first )
            {
                first = false;
                ostr<<*to_write;
            }
            else
                ostr<<'|'<<*to_write;
        }
        this->setConnectionPointsAttrUndoable( ostr.str().c_str(), _("Move a connection point") );
    }
}

void SPAvoidRef::deleteConnectionPoint(ConnectionPoint &cp)
{
    Inkscape::SVGOStringStream ostr;
    IdConnectionPointMap::iterator cp_pos = connection_points.find( cp.id );
    if ( cp_pos != connection_points.end() ) {
        bool first = true;
        for (IdConnectionPointMap::iterator it = connection_points.begin(); it != connection_points.end(); ++it) {
            if ( it != cp_pos ) {
                if ( first ) {
                    first = false;
                    ostr<<it->second;
                } else {
                    ostr<<'|'<<it->second;
                }
            }
        }
        this->setConnectionPointsAttrUndoable( ostr.str().c_str(), _("Remove a connection point") );
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
        Avoid::Polygon poly = avoid_item_poly(item);
        if (poly.size() > 0) {
            _transformed_connection = item->connectTransformed(
                    sigc::ptr_fun(&avoid_item_move));

            const char *id = SP_OBJECT_REPR(item)->attribute("id");
            g_assert(id != NULL);

            // Get a unique ID for the item.
            GQuark itemID = g_quark_from_string(id);

            shapeRef = new Avoid::ShapeRef(router, poly, itemID);

            router->addShape(shapeRef);
        }
    }
    else
    {
        g_assert(shapeRef);

        router->removeShape(shapeRef);
        delete shapeRef;
        shapeRef = NULL;
    }
}


GSList *SPAvoidRef::getAttachedShapes(const unsigned int type)
{
    GSList *list = NULL;

    Avoid::IntList shapes;
    GQuark shapeId = g_quark_from_string(item->getId());
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
    GQuark shapeId = g_quark_from_string(item->getId());
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

Geom::Point SPAvoidRef::getConnectionPointPos(const int type, const int id)
{
    g_assert(item);
    Geom::Point pos;
    const Geom::Matrix& transform = sp_item_i2doc_affine(item);
    // TODO investigate why this was asking for the active desktop:
    SPDesktop *desktop = inkscape_active_desktop();

    if ( type == ConnPointDefault )
    {
        // For now, just default to the centre of the item
        Geom::OptRect bbox = item->getBounds(sp_item_i2doc_affine(item));
        pos = (bbox) ? bbox->midpoint() : Geom::Point(0, 0);
    }
    else
    {
        // Get coordinates from the list of connection points
        // that are attached to the item
        pos = connection_points[id].pos * transform;
    }

    return pos;
}

bool SPAvoidRef::isValidConnPointId( const int type, const int id )
{
    if ( type < 0 || type > 1 )
        return false;
    else
    {
        if ( type == ConnPointDefault )
            if ( id < 0 || id > 8 )
                return false;
            else
            {
            }
        else
            return connection_points.find( id ) != connection_points.end();
    }

    return true;
}

static std::vector<Geom::Point> approxCurveWithPoints(SPCurve *curve)
{
    // The number of segments to use for not straight curves approximation
    const unsigned NUM_SEGS = 4;
    
    const Geom::PathVector& curve_pv = curve->get_pathvector();
   
    // The structure to hold the output
    std::vector<Geom::Point> poly_points;

    // Iterate over all curves, adding the endpoints for linear curves and
    // sampling the other curves
    double seg_size = 1.0 / NUM_SEGS;
    double at;
    at = 0;
    Geom::PathVector::const_iterator pit = curve_pv.begin();
    while (pit != curve_pv.end())
    {
        Geom::Path::const_iterator cit = pit->begin();
        while (cit != pit->end())
            if (dynamic_cast<Geom::CubicBezier const*>(&*cit))
            {
                at += seg_size;
                if (at <= 1.0 )
                    poly_points.push_back(cit->pointAt(at));
                else
                {
                    at = 0.0;
                    ++cit;
                }
            }
            else
            {
                poly_points.push_back(cit->finalPoint());
                ++cit;
            }
        ++pit;
    }
    return poly_points;
}

static std::vector<Geom::Point> approxItemWithPoints(SPItem const *item, const Geom::Matrix& item_transform)
{
    // The structure to hold the output
    std::vector<Geom::Point> poly_points;

    if (SP_IS_GROUP(item))
    {
        SPGroup* group = SP_GROUP(item);
        // consider all first-order children
        for (GSList const* i = sp_item_group_item_list(group); i != NULL; i = i->next) {
            SPItem* child_item = SP_ITEM(i->data);
            std::vector<Geom::Point> child_points = approxItemWithPoints(child_item, item_transform * child_item->transform);
            poly_points.insert(poly_points.end(), child_points.begin(), child_points.end());
        }
    }
    else if (SP_IS_SHAPE(item))
    {
        SPCurve* item_curve = sp_shape_get_curve(SP_SHAPE(item));
        // make sure it has an associated curve
        if (item_curve)
        {
            // apply transformations (up to common ancestor)
            item_curve->transform(item_transform);
            std::vector<Geom::Point> curve_points = approxCurveWithPoints(item_curve);
            poly_points.insert(poly_points.end(), curve_points.begin(), curve_points.end());
            item_curve->unref();
        }
    }

    return poly_points;
}
static Avoid::Polygon avoid_item_poly(SPItem const *item)
{
    SPDesktop *desktop = inkscape_active_desktop();
    g_assert(desktop != NULL);
    double spacing = desktop->namedview->connector_spacing;

    Geom::Matrix itd_mat = sp_item_i2doc_affine(item);
    std::vector<Geom::Point> hull_points;
    hull_points = approxItemWithPoints(item, itd_mat);

    // create convex hull from all sampled points
    Geom::ConvexHull hull(hull_points);

    // enlarge path by "desktop->namedview->connector_spacing"
    // store expanded convex hull in Avoid::Polygn
    Avoid::Polygon poly;

    Geom::Line hull_edge(hull[-1], hull[0]);
    Geom::Line prev_parallel_hull_edge;
    prev_parallel_hull_edge.origin(hull_edge.origin()+hull_edge.versor().ccw()*spacing);
    prev_parallel_hull_edge.versor(hull_edge.versor());
    int hull_size = hull.boundary.size();
    for (int i = 0; i <= hull_size; ++i)
    {
        hull_edge.setBy2Points(hull[i], hull[i+1]);
        Geom::Line parallel_hull_edge;
        parallel_hull_edge.origin(hull_edge.origin()+hull_edge.versor().ccw()*spacing);
        parallel_hull_edge.versor(hull_edge.versor());
        
        // determine the intersection point
        
        Geom::OptCrossing int_pt = Geom::intersection(parallel_hull_edge, prev_parallel_hull_edge);
        if (int_pt)
        {
            Avoid::Point avoid_pt((parallel_hull_edge.origin()+parallel_hull_edge.versor()*int_pt->ta)[Geom::X],
                                    (parallel_hull_edge.origin()+parallel_hull_edge.versor()*int_pt->ta)[Geom::Y]);
            poly.ps.push_back(avoid_pt);
        }
        else
        {
            // something went wrong...
            std::cout<<"conn-avoid-ref.cpp: avoid_item_poly: Geom:intersection failed."<<std::endl;
        }
        prev_parallel_hull_edge = parallel_hull_edge;
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
    Avoid::Polygon poly = avoid_item_poly(moved_item);
    if (!poly.empty()) {
        router->moveShape(shapeRef, poly);
    }
}


void init_avoided_shape_geometry(SPDesktop *desktop)
{
    // Don't count this as changes to the document,
    // it is basically just late initialisation.
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
