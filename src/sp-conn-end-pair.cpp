/*
 * A class for handling connector endpoint movement and libavoid interaction.
 *
 * Authors:
 *   Peter Moulder <pmoulder@mail.csse.monash.edu.au>
 *   Michael Wybrow <mjwybrow@users.sourceforge.net>
 *
 *    * Copyright (C) 2004-2005 Monash University
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cstring>
#include <string>

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
    : _invalid_path_connection()
    , _path(owner)
    , _connRef(NULL)
    , _connType(SP_CONNECTOR_NOAVOID)
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
    if (_connRef) {
        _connRef->removeFromGraph();
        delete _connRef;
        _connRef = NULL;
    }

    _invalid_path_connection.disconnect();
    _transformed_connection.disconnect();
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
}

void
sp_conn_end_pair_build(SPObject *object)
{
    sp_object_read_attr(object, "inkscape:connector-type");
    sp_object_read_attr(object, "inkscape:connection-start");
    sp_object_read_attr(object, "inkscape:connection-end");
}


static void
avoid_conn_move(Geom::Matrix const */*mp*/, SPItem *moved_item)
{
    // Reroute connector
    SPPath *path = SP_PATH(moved_item);
    path->connEndPair.makePathInvalid();
    sp_conn_adjust_invalid_path(path);
}


void
SPConnEndPair::setAttr(unsigned const key, gchar const *const value)
{
    if (key == SP_ATTR_CONNECTOR_TYPE) {
        if (value && (strcmp(value, "polyline") == 0)) {
            _connType = SP_CONNECTOR_POLYLINE;

            Avoid::Router *router = _path->document->router;
            GQuark itemID = g_quark_from_string(SP_OBJECT(_path)->id);
            _connRef = new Avoid::ConnRef(router, itemID);
            _invalid_path_connection = connectInvalidPath(
                    sigc::ptr_fun(&sp_conn_adjust_invalid_path));
            _transformed_connection = _path->connectTransformed(
                    sigc::ptr_fun(&avoid_conn_move));
        }
        else {
            _connType = SP_CONNECTOR_NOAVOID;

            if (_connRef) {
                _connRef->removeFromGraph();
                delete _connRef;
                _connRef = NULL;
                _invalid_path_connection.disconnect();
                _transformed_connection.disconnect();
            }
        }
        return;

    }

    unsigned const handle_ix = key - SP_ATTR_CONNECTION_START;
    g_assert( handle_ix <= 1 );
    this->_connEnd[handle_ix]->setAttacherHref(value);
}

void
SPConnEndPair::writeRepr(Inkscape::XML::Node *const repr) const
{
    for (unsigned handle_ix = 0; handle_ix < 2; ++handle_ix) {
        if (this->_connEnd[handle_ix]->ref.getURI()) {
            char const * const attr_strs[] = {"inkscape:connection-start",
                                              "inkscape:connection-end"};
            gchar *uri_string = this->_connEnd[handle_ix]->ref.getURI()->toString();
            repr->setAttribute(attr_strs[handle_ix], uri_string);
            g_free(uri_string);
        }
    }
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
            if (SP_GROUP(h2attItem[h])->group->getItemCount() == 0) {
                // This group is empty, so detach.
                sp_conn_end_detach(_path, h);
                h2attItem[h] = NULL;
            }
        }
    }
}

void
SPConnEndPair::getEndpoints(Geom::Point endPts[]) const {
    SPCurve *curve = _path->curve;
    SPItem *h2attItem[2];
    getAttachedItems(h2attItem);

    for (unsigned h = 0; h < 2; ++h) {
        if ( h2attItem[h] ) {
            Geom::OptRect bbox = h2attItem[h]->getBounds(sp_item_i2doc_affine(h2attItem[h]));
            if (bbox) {
                endPts[h] = bbox->midpoint();
            } else {
                // FIXME
                endPts[h] = Geom::Point(0, 0);
            }
        }
        else
        {
            if (h == 0) {
                endPts[h] = *(curve->first_point());
            }
            else {
                endPts[h] = *(curve->last_point());
            }
        }
    }
}

sigc::connection
SPConnEndPair::connectInvalidPath(sigc::slot<void, SPPath *> slot)
{
    return _invalid_path_signal.connect(slot);
}

static void emitPathInvalidationNotification(void *ptr)
{
    // We emit a signal here rather than just calling the reroute function
    // since this allows all the movement action computation to happen,
    // then all connectors (that require it) will be rerouted.  Otherwise,
    // one connector could get rerouted several times as a result of
    // dragging a couple of shapes.

    SPPath *path = SP_PATH(ptr);
    path->connEndPair._invalid_path_signal.emit(path);
}

void
SPConnEndPair::rerouteFromManipulation(void)
{
    _connRef->makePathInvalid();
    sp_conn_adjust_path(_path);
}

void
SPConnEndPair::reroute(void)
{
    sp_conn_adjust_path(_path);
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

            _connRef->lateSetup(src, dst);
            _connRef->setCallback(&emitPathInvalidationNotification, _path);
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

void
SPConnEndPair::reroutePath(void)
{
    if (!isAutoRoutingConn()) {
        // Do nothing
        return;
    }

    SPCurve *curve = _path->curve;

    Geom::Point endPt[2];
    getEndpoints(endPt);

    Avoid::Point src(endPt[0][Geom::X], endPt[0][Geom::Y]);
    Avoid::Point dst(endPt[1][Geom::X], endPt[1][Geom::Y]);

    _connRef->updateEndPoint(Avoid::VertID::src, src);
    _connRef->updateEndPoint(Avoid::VertID::tar, dst);

    _connRef->generatePath(src, dst);

    Avoid::PolyLine route = _connRef->route();
    _connRef->calcRouteDist();

    curve->reset();
    curve->moveto(endPt[0]);

    for (int i = 1; i < route.pn; ++i) {
        Geom::Point p(route.ps[i].x, route.ps[i].y);
        curve->lineto(p);
    }

    Geom::Matrix doc2item = sp_item_i2doc_affine(SP_ITEM(_path)).inverse();
    curve->transform(doc2item); 
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
