
#include <cstring>
#include <string>
#include <limits>

#include "display/curve.h"
#include "libnr/nr-matrix-fns.h"
#include "xml/repr.h"
#include "sp-conn-end.h"
#include "sp-path.h"
#include "uri.h"
#include "document.h"
#include "sp-item-group.h"
#include "2geom/path.h"
#include "2geom/pathvector.h"
#include "2geom/path-intersection.h"


static void change_endpts(SPCurve *const curve, Geom::Point const h2endPt[2]);

SPConnEnd::SPConnEnd(SPObject *const owner) :
    ref(owner),
    href(NULL),
    _changed_connection(),
    _delete_connection(),
    _transformed_connection()
{
}

static SPObject const *
get_nearest_common_ancestor(SPObject const *const obj, SPItem const *const objs[2]) {
    SPObject const *anc_sofar = obj;
    for (unsigned i = 0; i < 2; ++i) {
        if ( objs[i] != NULL ) {
            anc_sofar = anc_sofar->nearestCommonAncestor(objs[i]);
        }
    }
    return anc_sofar;
}


static bool try_get_intersect_point_with_item_recursive(SPCurve *conn_curve, SPItem& item, 
        const Geom::Matrix& item_transform, const bool at_start, double* intersect_pos, 
        unsigned *intersect_index) {

    double initial_pos = (at_start) ? 0.0 : std::numeric_limits<double>::max();

    // if this is a group...
    if (SP_IS_GROUP(&item)) {
        SPGroup* group = SP_GROUP(&item);
        
        // consider all first-order children
        double child_pos = initial_pos;
        unsigned child_index;
        for (GSList const* i = sp_item_group_item_list(group); i != NULL; i = i->next) {
            SPItem* child_item = SP_ITEM(i->data);
            try_get_intersect_point_with_item_recursive(conn_curve, *child_item, 
                    item_transform * child_item->transform, at_start, &child_pos, &child_index);
            if (fabs(initial_pos - child_pos) > fabs(initial_pos - *intersect_pos)) {
                // It is further away from the initial point than the current intersection
                // point (i.e. the "outermost" intersection), so use this one.
                *intersect_pos = child_pos;
                *intersect_index = child_index;
            }
        }
        return *intersect_pos != initial_pos;
    }

    // if this is a shape...
    if (!SP_IS_SHAPE(&item)) return false;

    // make sure it has an associated curve
    SPCurve* item_curve = sp_shape_get_curve(SP_SHAPE(&item));
    if (!item_curve) return false;

    // apply transformations (up to common ancestor)
    item_curve->transform(item_transform);

    const Geom::PathVector& curve_pv = item_curve->get_pathvector();
    const Geom::PathVector& conn_pv = conn_curve->get_pathvector();
    Geom::CrossingSet cross = crossings(conn_pv, curve_pv);
    // iterate over all Crossings
    for (Geom::CrossingSet::const_iterator i = cross.begin(); i != cross.end(); i++) {
        const Geom::Crossings& cr = *i;

        for (Geom::Crossings::const_iterator i = cr.begin(); i != cr.end(); i++) {
            const Geom::Crossing& cr_pt = *i;
            if (fabs(initial_pos - cr_pt.ta) > fabs(initial_pos - *intersect_pos)) {
                // It is further away from the initial point than the current intersection
                // point (i.e. the "outermost" intersection), so use this one.
                *intersect_pos = cr_pt.ta;
                *intersect_index = cr_pt.a;
            }
        }
    }

    item_curve->unref();

    return *intersect_pos != initial_pos;
}


// This function returns the outermost intersection point between the path (a connector)
// and the item given.  If the item is a group, then the component items are considered.
// The transforms given should be to a common ancestor of both the path and item.
//
static bool try_get_intersect_point_with_item(SPPath& conn, SPItem& item, 
        const Geom::Matrix& item_transform, const Geom::Matrix& conn_transform, 
        const bool at_start, double* intersect_pos, unsigned *intersect_index) {
 
    // We start with the intersection point either at the beginning or end of the 
    // path, depending on whether we are considering the source or target endpoint.
    *intersect_pos = (at_start) ? 0.0 : std::numeric_limits<double>::max();

    // Copy the curve and apply transformations up to common ancestor.
    SPCurve* conn_curve = conn.curve->copy();
    conn_curve->transform(conn_transform);

    // Find the intersection.
    bool result = try_get_intersect_point_with_item_recursive(conn_curve, item, item_transform, 
            at_start, intersect_pos, intersect_index);
    
    // Free the curve copy.
    conn_curve->unref();

    return result;
}


static void
sp_conn_end_move_compensate(Geom::Matrix const */*mp*/, SPItem */*moved_item*/,
                            SPPath *const path,
                            bool const updatePathRepr = true)
{
    // TODO: SPItem::getBounds gives the wrong result for some objects
    //       that have internal representations that are updated later
    //       by the sp_*_update functions, e.g., text.
    sp_document_ensure_up_to_date(path->document);

    // Get the new route around obstacles.
    path->connEndPair.reroutePath();

    SPItem *h2attItem[2];
    path->connEndPair.getAttachedItems(h2attItem);

    SPItem const *const path_item = SP_ITEM(path);
    SPObject const *const ancestor = get_nearest_common_ancestor(path_item, h2attItem);
    Geom::Matrix const path2anc(i2anc_affine(path_item, ancestor));

    Geom::Point endPts[2] = { *(path->curve->first_point()), *(path->curve->last_point()) };
 
    for (unsigned h = 0; h < 2; ++h) {
        if (h2attItem[h]) {
            // For each attached object, change the corresponding point to be
            // at the outermost intersection with the object's path.
            double intersect_pos;
            unsigned intersect_index;
            Geom::Matrix h2i2anc = i2anc_affine(h2attItem[h], ancestor);
            if ( try_get_intersect_point_with_item(*path, *h2attItem[h], h2i2anc, path2anc, 
                        (h == 0), &intersect_pos, &intersect_index) ) {
                const Geom::PathVector& curve = path->curve->get_pathvector();
                endPts[h] = curve[intersect_index].pointAt(intersect_pos);
            }
        }
    }
    change_endpts(path->curve, endPts);
    if (updatePathRepr) {
        path->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        path->updateRepr();
    }
}

// TODO: This triggering of makeInvalidPath could be cleaned up to be
//       another option passed to move_compensate.
static void
sp_conn_end_shape_move_compensate(Geom::Matrix const *mp, SPItem *moved_item,
                            SPPath *const path)
{
    if (path->connEndPair.isAutoRoutingConn()) {
        path->connEndPair.makePathInvalid();
    }
    sp_conn_end_move_compensate(mp, moved_item, path);
}


void
sp_conn_adjust_invalid_path(SPPath *const path)
{
    sp_conn_end_move_compensate(NULL, NULL, path);
}

void
sp_conn_adjust_path(SPPath *const path)
{
    if (path->connEndPair.isAutoRoutingConn()) {
        path->connEndPair.makePathInvalid();
    }
    // Don't update the path repr or else connector dragging is slowed by
    // constant update of values to the xml editor, and each step is also
    // needlessly remembered by undo/redo.
    bool const updatePathRepr = false;
    sp_conn_end_move_compensate(NULL, NULL, path, updatePathRepr);
}


static void
change_endpts(SPCurve *const curve, Geom::Point const h2endPt[2])
{
#if 0
    curve->reset();
    curve->moveto(h2endPt[0]);
    curve->lineto(h2endPt[1]);
#else
    curve->move_endpoints(h2endPt[0], h2endPt[1]);
#endif
}

static void
sp_conn_end_deleted(SPObject *, SPObject *const owner, unsigned const handle_ix)
{
    // todo: The first argument is the deleted object, or just NULL if
    //       called by sp_conn_end_detach.
    g_return_if_fail(handle_ix < 2);
    char const *const attr_str[] = {"inkscape:connection-start",
                                    "inkscape:connection-end"};
    SP_OBJECT_REPR(owner)->setAttribute(attr_str[handle_ix], NULL);
    /* I believe this will trigger sp_conn_end_href_changed. */
}

void
sp_conn_end_detach(SPObject *const owner, unsigned const handle_ix)
{
    sp_conn_end_deleted(NULL, owner, handle_ix);
}

void
SPConnEnd::setAttacherHref(gchar const *value)
{
    if ( value && href && ( strcmp(value, href) == 0 ) ) {
        /* No change, do nothing. */
    } else {
        g_free(href);
        href = NULL;
        if (value) {
            // First, set the href field, because sp_conn_end_href_changed will need it.
            href = g_strdup(value);

            // Now do the attaching, which emits the changed signal.
            try {
                ref.attach(Inkscape::URI(value));
            } catch (Inkscape::BadURIException &e) {
                /* TODO: Proper error handling as per
                 * http://www.w3.org/TR/SVG11/implnote.html#ErrorProcessing.  (Also needed for
                 * sp-use.) */
                g_warning("%s", e.what());
                ref.detach();
            }
        } else {
            ref.detach();
        }
    }
}

void
sp_conn_end_href_changed(SPObject */*old_ref*/, SPObject */*ref*/,
                         SPConnEnd *connEndPtr, SPPath *const path, unsigned const handle_ix)
{
    g_return_if_fail(connEndPtr != NULL);
    SPConnEnd &connEnd = *connEndPtr;
    connEnd._delete_connection.disconnect();
    connEnd._transformed_connection.disconnect();

    if (connEnd.href) {
        SPObject *refobj = connEnd.ref.getObject();
        if (refobj) {
            connEnd._delete_connection
                = SP_OBJECT(refobj)->connectDelete(sigc::bind(sigc::ptr_fun(&sp_conn_end_deleted),
                                                              SP_OBJECT(path), handle_ix));
            connEnd._transformed_connection
                = SP_ITEM(refobj)->connectTransformed(sigc::bind(sigc::ptr_fun(&sp_conn_end_shape_move_compensate),
                                                                 path));
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
