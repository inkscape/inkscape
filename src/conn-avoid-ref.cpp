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
        for (int ix=0; ix<2; ++ix)
            if (connEnds[ix]->type == ConnPointUserDefined)
                if (updates.find(connEnds[ix]->id) != updates.end())
                    if (path->connEndPair.isAutoRoutingConn())
                        path->connEndPair.tellLibavoidNewEndpoints();
                    else
                    {
                    }
                else
                    if (deletes.find(connEnds[ix]->id) != deletes.end())
                        sp_conn_end_detach(path, ix);
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
        
    this->setConnectionPointsAttrUndoable( ostr.str().c_str(), _("Added a new connection point") );
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
        this->setConnectionPointsAttrUndoable( ostr.str().c_str(), _("Moved a connection point") );
    }
}

void SPAvoidRef::deleteConnectionPoint(ConnectionPoint &cp)
{
    Inkscape::SVGOStringStream ostr;
    IdConnectionPointMap::iterator cp_pos = connection_points.find( cp.id );
    if ( cp_pos != connection_points.end() )
    {
        bool first = true;
        for (IdConnectionPointMap::iterator it = connection_points.begin(); it != connection_points.end(); ++it)
        {
            if ( it != cp_pos )
                if ( first )
                {
                    first = false;
                    ostr<<it->second;
                }
                else
                    ostr<<'|'<<it->second;
        }
        this->setConnectionPointsAttrUndoable( ostr.str().c_str(), _("Removed a connection point") );
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

Geom::Point SPAvoidRef::getConnectionPointPos(const int type, const int id)
{
    g_assert(item);
    Geom::Point pos;
    const Geom::Matrix& transform = sp_item_i2doc_affine(item);
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

static Avoid::Polygon avoid_item_poly(SPItem const *item)
{
    SPDesktop *desktop = inkscape_active_desktop();
    g_assert(desktop != NULL);

    // TODO: The right way to do this is to return the convex hull of
    //       the object, or an approximation in the case of a rounded
    //       object.  Specific SPItems will need to have a new
    //       function that returns points for the convex hull.
    //       For some objects it is enough to feed the snappoints to
    //       some convex hull code, though not NR::ConvexHull as this
    //       only keeps the bounding box of the convex hull currently.

    double spacing = desktop->namedview->connector_spacing;

    // [sommer] If item is a shape, use an approximation of its convex hull
    {
        // MJW: Disable this for the moment.  It still has some issues.
        const bool convex_hull_approximation_enabled = false;

        if ( convex_hull_approximation_enabled && SP_IS_SHAPE (item) ) {
            // The number of points to use for approximation
            const unsigned NUM_POINTS = 64; 

//             printf("[sommer] is a shape\n");
            SPCurve* curve = sp_shape_get_curve (SP_SHAPE (item));
            if (curve) {
//                 printf("[sommer] is a curve\n");

                // apply all transformations
                Geom::Matrix itd_mat = sp_item_i2doc_affine(item);
                curve->transform(itd_mat);

                // iterate over all paths
                const Geom::PathVector& curve_pv = curve->get_pathvector();
                std::vector<Geom::Point> hull_points;
                for (Geom::PathVector::const_iterator i = curve_pv.begin(); i != curve_pv.end(); i++) {
                    const Geom::Path& curve_pv_path = *i; 
//                     printf("[sommer] tracing sub-path\n");

                    // FIXME: enlarge path by "desktop->namedview->connector_spacing" (using sp_selected_path_do_offset)?

                    // use appropriate fraction of points for this path (first one gets any remainder)
                    unsigned num_points = NUM_POINTS / curve_pv.size();
                    if (i == curve_pv.begin()) num_points += NUM_POINTS - (num_points * curve_pv.size());
                    printf("[sommer] using %d points for this path\n", num_points);

                    // sample points along the path for approximation of convex hull
                    for (unsigned n = 0; n < num_points; n++) {
                        double at = curve_pv_path.size() / static_cast<double>(num_points) * n;                        
                        Geom::Point pt = curve_pv_path.pointAt(at);
                        hull_points.push_back(pt);
                    }
                }

                curve->unref();

                // create convex hull from all sampled points
                Geom::ConvexHull hull(hull_points);

                // store expanded convex hull in Avoid::Polygn
                unsigned n = 0;
                Avoid::Polygon poly;
                const Geom::Point& old_pt = *hull.boundary.begin();

                Geom::Line hull_edge(*hull.boundary.begin(), *(hull.boundary.begin()+1));
                Geom::Line parallel_hull_edge;
                parallel_hull_edge.origin(hull_edge.origin()+hull_edge.versor().ccw()*spacing);
                parallel_hull_edge.versor(hull_edge.versor());
                Geom::Line bisector = Geom::make_angle_bisector_line( *(hull.boundary.end()), *hull.boundary.begin(),
                                                                      *(hull.boundary.begin()+1));
                Geom::OptCrossing int_pt = Geom::intersection(parallel_hull_edge, bisector);

                if (int_pt)
                {
                    Avoid::Point avoid_pt((parallel_hull_edge.origin()+parallel_hull_edge.versor()*int_pt->ta)[Geom::X], 
                                            (parallel_hull_edge.origin()+parallel_hull_edge.versor()*int_pt->ta)[Geom::Y]);
//                     printf("[sommer] %f, %f\n", old_pt[Geom::X], old_pt[Geom::Y]);
/*                    printf("[sommer] %f, %f\n", (parallel_hull_edge.origin()+parallel_hull_edge.versor()*int_pt->ta)[Geom::X], 
                                                (parallel_hull_edge.origin()+parallel_hull_edge.versor()*int_pt->ta)[Geom::Y]);*/
                    poly.ps.push_back(avoid_pt);
                }
                for (std::vector<Geom::Point>::const_iterator i = hull.boundary.begin() + 1; i != hull.boundary.end(); i++, n++) {
                        const Geom::Point& old_pt = *i;
                        Geom::Line hull_edge(*i, *(i+1));
                        Geom::Line parallel_hull_edge;
                        parallel_hull_edge.origin(hull_edge.origin()+hull_edge.versor().ccw()*spacing);
                        parallel_hull_edge.versor(hull_edge.versor());
                        Geom::Line bisector = Geom::make_angle_bisector_line( *(i-1), *i, *(i+1));
                        Geom::OptCrossing intersect_pt = Geom::intersection(parallel_hull_edge, bisector);

                        if (int_pt)
                        {
                            Avoid::Point avoid_pt((parallel_hull_edge.origin()+parallel_hull_edge.versor()*int_pt->ta)[Geom::X], 
                                                  (parallel_hull_edge.origin()+parallel_hull_edge.versor()*int_pt->ta)[Geom::Y]);
/*                            printf("[sommer] %f, %f\n", old_pt[Geom::X], old_pt[Geom::Y]);
                            printf("[sommer] %f, %f\n", (parallel_hull_edge.origin()+parallel_hull_edge.versor()*int_pt->ta)[Geom::X], 
                                                        (parallel_hull_edge.origin()+parallel_hull_edge.versor()*int_pt->ta)[Geom::Y]);*/
                            poly.ps.push_back(avoid_pt);
                        }
                }
                

                return poly;
            }// else printf("[sommer] is no curve\n");
        }// else printf("[sommer] is no shape\n");
    }
    
    Geom::OptRect rHull = item->getBounds(sp_item_i2doc_affine(item));
    if (!rHull) {
        return Avoid::Polygon();
    }

    // Add a little buffer around the edge of each object.
    Geom::Rect rExpandedHull = *rHull;
    rExpandedHull.expandBy(spacing); 
    Avoid::Polygon poly(4);

    for (size_t n = 0; n < 4; ++n) {
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
