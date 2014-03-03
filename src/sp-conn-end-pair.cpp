/*
 * A class for handling connector endpoint movement and libavoid interaction.
 *
 * Authors:
 *   Peter Moulder <pmoulder@mail.csse.monash.edu.au>
 *   Michael Wybrow <mjwybrow@users.sourceforge.net>
 *   Abhishek Sharma
 *
 *    * Copyright (C) 2004-2005 Monash University
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cstring>
#include <string>
#include <iostream>
#include <glibmm/stringutils.h>

#include "attributes.h"
#include "sp-conn-end.h"
#include "uri.h"
#include "display/curve.h"
#include "xml/repr.h"
#include "sp-path.h"
#include "libavoid/vertices.h"
#include "libavoid/router.h"
#include "document.h"
#include "sp-item-group.h"


SPConnEndPair::SPConnEndPair(SPPath *const owner)
    : _path(owner)
    , _connRef(NULL)
    , _connType(SP_CONNECTOR_NOAVOID)
    , _connCurvature(0.0)
    , _transformed_connection()
{
    for (unsigned handle_ix = 0; handle_ix <= 1; ++handle_ix) {
        this->_connEnd[handle_ix] = new SPConnEnd(SP_OBJECT(owner));
        this->_connEnd[handle_ix]->_changed_connection
            = this->_connEnd[handle_ix]->ref.changedSignal()
            .connect(sigc::bind(sigc::ptr_fun(sp_conn_end_href_changed),
                                this->_connEnd[handle_ix], owner, handle_ix));
    }
}

SPConnEndPair::~SPConnEndPair()
{
    for (unsigned handle_ix = 0; handle_ix < 2; ++handle_ix) {
        delete this->_connEnd[handle_ix];
        this->_connEnd[handle_ix] = NULL;
    }
}

void
SPConnEndPair::release()
{
    for (unsigned handle_ix = 0; handle_ix < 2; ++handle_ix) {
        this->_connEnd[handle_ix]->_changed_connection.disconnect();
        this->_connEnd[handle_ix]->_delete_connection.disconnect();
        this->_connEnd[handle_ix]->_transformed_connection.disconnect();
        g_free(this->_connEnd[handle_ix]->href);
        this->_connEnd[handle_ix]->href = NULL;
        this->_connEnd[handle_ix]->ref.detach();
    }

    // If the document is being destroyed then the router instance
    // and the ConnRefs will have been destroyed with it.
    const bool routerInstanceExists = (_path->document->router != NULL);

    if (_connRef && routerInstanceExists) {
        _connRef->removeFromGraph();
        delete _connRef;
    }
    _connRef = NULL;

    _transformed_connection.disconnect();
}

void
sp_conn_end_pair_build(SPObject *object)
{
    object->readAttr( "inkscape:connector-type" );
    object->readAttr( "inkscape:connection-start" );
    object->readAttr( "inkscape:connection-end" );
    object->readAttr( "inkscape:connector-curvature" );
}


static void
avoid_conn_transformed(Geom::Affine const */*mp*/, SPItem *moved_item)
{
    SPPath *path = SP_PATH(moved_item);
    if (path->connEndPair.isAutoRoutingConn()) {
        path->connEndPair.tellLibavoidNewEndpoints();
    }
}


void
SPConnEndPair::setAttr(unsigned const key, gchar const *const value)
{
    switch (key)
    {
        case SP_ATTR_CONNECTOR_TYPE:
            if (value && (strcmp(value, "polyline") == 0 || strcmp(value, "orthogonal") == 0)) {
                int newconnType = strcmp(value, "polyline") ? SP_CONNECTOR_ORTHOGONAL : SP_CONNECTOR_POLYLINE;

                if (!_connRef)
                {
                    _connType = newconnType;
                    Avoid::Router *router = _path->document->router;
                    GQuark itemID = g_quark_from_string(_path->getId());
                    _connRef = new Avoid::ConnRef(router, itemID);
                    switch (newconnType)
                    {
                        case SP_CONNECTOR_POLYLINE:
                            _connRef->setRoutingType(Avoid::ConnType_PolyLine);
                            break;
                        case SP_CONNECTOR_ORTHOGONAL:
                            _connRef->setRoutingType(Avoid::ConnType_Orthogonal);
                    }
                    _transformed_connection = _path->connectTransformed(
                            sigc::ptr_fun(&avoid_conn_transformed));
                }
                else
                    if (newconnType != _connType)
                    {
                        _connType = newconnType;
                        switch (newconnType)
                        {
                            case SP_CONNECTOR_POLYLINE:
                                _connRef->setRoutingType(Avoid::ConnType_PolyLine);
                                break;
                            case SP_CONNECTOR_ORTHOGONAL:
                                _connRef->setRoutingType(Avoid::ConnType_Orthogonal);
                        }
                        sp_conn_reroute_path(_path);
                    }
            }
            else {
                _connType = SP_CONNECTOR_NOAVOID;

                if (_connRef) {
                    _connRef->removeFromGraph();
                    delete _connRef;
                    _connRef = NULL;
                    _transformed_connection.disconnect();
                }
            }
            break;
        case SP_ATTR_CONNECTOR_CURVATURE:
            if (value) {
                _connCurvature = g_strtod(value, NULL);
                if (_connRef && _connRef->isInitialised()) {
                    // Redraw the connector, but only if it has been initialised.
                    sp_conn_reroute_path(_path);
                }
            }
            break;
        case SP_ATTR_CONNECTION_START:
        case SP_ATTR_CONNECTION_END:
            this->_connEnd[(key == SP_ATTR_CONNECTION_START ? 0 : 1)]->setAttacherHref(value, _path);
            break;
    }

}

void
SPConnEndPair::writeRepr(Inkscape::XML::Node *const repr) const
{
    char const * const attr_strs[] = {"inkscape:connection-start", "inkscape:connection-end"};
    for (unsigned handle_ix = 0; handle_ix < 2; ++handle_ix) {
        const Inkscape::URI* U = this->_connEnd[handle_ix]->ref.getURI();
        if (U) {
            gchar *str = U->toString();
            repr->setAttribute(attr_strs[handle_ix], str);
            g_free(str);
        }
    }
    repr->setAttribute("inkscape:connector-curvature", Glib::Ascii::dtostr(_connCurvature).c_str());
    if (_connType == SP_CONNECTOR_POLYLINE || _connType == SP_CONNECTOR_ORTHOGONAL)
        repr->setAttribute("inkscape:connector-type", _connType == SP_CONNECTOR_POLYLINE ? "polyline" : "orthogonal" );
}

void
SPConnEndPair::getAttachedItems(SPItem *h2attItem[2]) const {
    for (unsigned h = 0; h < 2; ++h) {
        h2attItem[h] = this->_connEnd[h]->ref.getObject();

        // Deal with the case of the attached object being an empty group.
        // A group containing no items does not have a valid bbox, so
        // causes problems for the auto-routing code.  Also, since such a
        // group no longer has an onscreen representation and can only be
        // selected through the XML editor, it makes sense just to detach
        // connectors from them.
        if (SP_IS_GROUP(h2attItem[h])) {
            if (SP_GROUP(h2attItem[h])->getItemCount() == 0) {
                // This group is empty, so detach.
                sp_conn_end_detach(_path, h);
                h2attItem[h] = NULL;
            }
        }
    }
}

void SPConnEndPair::getEndpoints(Geom::Point endPts[]) const
{
    SPCurve const *curve = _path->get_curve_reference();
    SPItem *h2attItem[2] = {0};
    getAttachedItems(h2attItem);
    Geom::Affine i2d = _path->i2doc_affine();

    for (unsigned h = 0; h < 2; ++h) {
        if ( h2attItem[h] ) {
            g_assert(h2attItem[h]->avoidRef);
            endPts[h] = h2attItem[h]->avoidRef->getConnectionPointPos();
        }
        else if (!curve->is_empty())
        {
            if (h == 0) {
                endPts[h] = *(curve->first_point())*i2d;
            }
            else {
                endPts[h] = *(curve->last_point())*i2d;
            }
        }
    }
}

gdouble
SPConnEndPair::getCurvature(void) const {
    return _connCurvature;
}

SPConnEnd**
SPConnEndPair::getConnEnds(void)
{
    return _connEnd;
}

bool
SPConnEndPair::isOrthogonal(void) const {
    return _connType == SP_CONNECTOR_ORTHOGONAL;
}


static void redrawConnectorCallback(void *ptr)
{
    SPPath *path = SP_PATH(ptr);
    if (path->document == NULL) {
        // This can happen when the document is being destroyed.
        return;
    }
    sp_conn_redraw_path(path);
}

void
SPConnEndPair::rerouteFromManipulation(void)
{
    sp_conn_reroute_path_immediate(_path);
}


// Called from sp_path_update to initialise the endpoints.
void
SPConnEndPair::update(void)
{
    if (_connType != SP_CONNECTOR_NOAVOID) {
        g_assert(_connRef != NULL);
        if (!(_connRef->isInitialised())) {
            Geom::Point endPt[2];
            getEndpoints(endPt);

            Avoid::Point src(endPt[0][Geom::X], endPt[0][Geom::Y]);
            Avoid::Point dst(endPt[1][Geom::X], endPt[1][Geom::Y]);

            _connRef->setEndpoints(src, dst);
            _connRef->setCallback(&redrawConnectorCallback, _path);
        }
        // Store the ID of the objects attached to the connector.
        storeIds();
    }
}


void SPConnEndPair::storeIds(void)
{
    if (_connEnd[0]->href) {
        // href begins with a '#' which we don't want.
        const char *startId = _connEnd[0]->href + 1;
        GQuark itemId = g_quark_from_string(startId);
        _connRef->setEndPointId(Avoid::VertID::src, itemId);
    }
    else {
        _connRef->setEndPointId(Avoid::VertID::src, 0);
    }
    if (_connEnd[1]->href) {
        // href begins with a '#' which we don't want.
        const char *endId = _connEnd[1]->href + 1;
        GQuark itemId = g_quark_from_string(endId);
        _connRef->setEndPointId(Avoid::VertID::tar, itemId);
    }
    else {
        _connRef->setEndPointId(Avoid::VertID::tar, 0);
    }
}


bool
SPConnEndPair::isAutoRoutingConn(void)
{
    if (_connType != SP_CONNECTOR_NOAVOID) {
        return true;
    }
    return false;
}

void
SPConnEndPair::makePathInvalid(void)
{
    _connRef->makePathInvalid();
}


// Redraws the curve along the recalculated route
// Straight or curved
void recreateCurve(SPCurve *curve, Avoid::ConnRef *connRef, const gdouble curvature)
{
    bool straight = curvature<1e-3;

    Avoid::PolyLine route = connRef->displayRoute();
    if (!straight)
        route = route.curvedPolyline(curvature);
    connRef->calcRouteDist();

    curve->reset();

    curve->moveto( Geom::Point(route.ps[0].x, route.ps[0].y) );
    int pn = route.size();
    for (int i = 1; i < pn; ++i) {
        Geom::Point p(route.ps[i].x, route.ps[i].y);
        if (straight) {
            curve->lineto( p );
        }
        else {
            switch (route.ts[i]) {
                case 'M':
                    curve->moveto( p );
                    break;
                case 'L':
                    curve->lineto( p );
                    break;
                case 'C':
                    g_assert( i+2<pn );
                    curve->curveto( p, Geom::Point(route.ps[i+1].x, route.ps[i+1].y),
                            Geom::Point(route.ps[i+2].x, route.ps[i+2].y) );
                    i+=2;
                    break;
            }
        }
    }
}


void
SPConnEndPair::tellLibavoidNewEndpoints(const bool processTransaction)
{
    if (!isAutoRoutingConn()) {
        // Do nothing
        return;
    }
    makePathInvalid();

    Geom::Point endPt[2];
    getEndpoints(endPt);

    Avoid::Point src(endPt[0][Geom::X], endPt[0][Geom::Y]);
    Avoid::Point dst(endPt[1][Geom::X], endPt[1][Geom::Y]);

    _connRef->setEndpoints(src, dst);
    if (processTransaction)
    {
        _connRef->router()->processTransaction();
    }
    return;
}


bool
SPConnEndPair::reroutePathFromLibavoid(void)
{
    if (!isAutoRoutingConn()) {
        // Do nothing
        return false;
    }

    SPCurve *curve = _path->get_curve();

    recreateCurve( curve, _connRef, _connCurvature );

    Geom::Affine doc2item = _path->i2doc_affine().inverse();
    curve->transform(doc2item);

    return true;
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
