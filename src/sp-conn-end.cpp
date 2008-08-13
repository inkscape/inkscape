
#include <cstring>
#include <string>

#include "display/curve.h"
#include "libnr/nr-matrix-fns.h"
#include "xml/repr.h"
#include "sp-conn-end.h"
#include "sp-path.h"
#include "uri.h"
#include "document.h"


static void change_endpts(SPCurve *const curve, NR::Point const h2endPt[2]);
static NR::Point calc_bbox_conn_pt(NR::Rect const &bbox, NR::Point const &p);
static double signed_one(double const x);

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

static void
sp_conn_end_move_compensate(NR::Matrix const */*mp*/, SPItem */*moved_item*/,
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
    if ( !h2attItem[0] && !h2attItem[1] ) {
        if (updatePathRepr) {
            path->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
            path->updateRepr();
        }
        return;
    }

    SPItem const *const path_item = SP_ITEM(path);
    SPObject const *const ancestor = get_nearest_common_ancestor(path_item, h2attItem);
    NR::Matrix const path2anc(i2anc_affine(path_item, ancestor));

    if (h2attItem[0] != NULL && h2attItem[1] != NULL) {
        /* Initial end-points: centre of attached object. */
        NR::Point h2endPt_icoordsys[2];
        NR::Matrix h2i2anc[2];
        NR::Rect h2bbox_icoordsys[2];
        NR::Point last_seg_endPt[2] = {
            *(path->curve->second_point()),
            *(path->curve->penultimate_point())
        };
        for (unsigned h = 0; h < 2; ++h) {
            boost::optional<NR::Rect> bbox = h2attItem[h]->getBounds(NR::identity());
            if (!bbox) {
                if (updatePathRepr) {
                    path->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
                    path->updateRepr();
                }
                return;
            }
            h2bbox_icoordsys[h] = *bbox;
            h2i2anc[h] = i2anc_affine(h2attItem[h], ancestor);
            h2endPt_icoordsys[h] = h2bbox_icoordsys[h].midpoint();
        }

        // For each attached object, change the corresponding point to be
        // on the edge of the bbox.
        NR::Point h2endPt_pcoordsys[2];
        for (unsigned h = 0; h < 2; ++h) {
            h2endPt_icoordsys[h] = calc_bbox_conn_pt(h2bbox_icoordsys[h],
                                         ( last_seg_endPt[h] * h2i2anc[h].inverse() ));
            h2endPt_pcoordsys[h] = h2endPt_icoordsys[h] * h2i2anc[h] * path2anc.inverse();
        }
        change_endpts(path->curve, h2endPt_pcoordsys);
    } else {
        // We leave the unattached endpoint where it is, and adjust the
        // position of the attached endpoint to be on the edge of the bbox.
        unsigned ind;
        NR::Point other_endpt;
        NR::Point last_seg_pt;
        if (h2attItem[0] != NULL) {
            other_endpt = *(path->curve->last_point());
            last_seg_pt = *(path->curve->second_point());
            ind = 0;
        }
        else {
            other_endpt = *(path->curve->first_point());
            last_seg_pt = *(path->curve->penultimate_point());
            ind = 1;
        }
        NR::Point h2endPt_icoordsys[2];
        NR::Matrix h2i2anc;

        NR::Rect otherpt_rect = NR::Rect(other_endpt, other_endpt);
        NR::Rect h2bbox_icoordsys[2] = { otherpt_rect, otherpt_rect };
        boost::optional<NR::Rect> bbox = h2attItem[ind]->getBounds(NR::identity());
        if (!bbox) {
            if (updatePathRepr) {
                path->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
                path->updateRepr();
            }
            return;
        }

        h2bbox_icoordsys[ind] = *bbox;
        h2i2anc = i2anc_affine(h2attItem[ind], ancestor);
        h2endPt_icoordsys[ind] = h2bbox_icoordsys[ind].midpoint();

        h2endPt_icoordsys[!ind] = other_endpt;

        // For the attached object, change the corresponding point to be
        // on the edge of the bbox.
        NR::Point h2endPt_pcoordsys[2];
        h2endPt_icoordsys[ind] = calc_bbox_conn_pt(h2bbox_icoordsys[ind],
                                                 ( last_seg_pt * h2i2anc.inverse() ));
        h2endPt_pcoordsys[ind] = h2endPt_icoordsys[ind] * h2i2anc * path2anc.inverse();

        // Leave the other where it is.
        h2endPt_pcoordsys[!ind] = other_endpt;

        change_endpts(path->curve, h2endPt_pcoordsys);
    }
    if (updatePathRepr) {
        path->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
        path->updateRepr();
    }
}

// TODO: This triggering of makeInvalidPath could be cleaned up to be
//       another option passed to move_compensate.
static void
sp_conn_end_shape_move_compensate(NR::Matrix const *mp, SPItem *moved_item,
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

static NR::Point
calc_bbox_conn_pt(NR::Rect const &bbox, NR::Point const &p)
{
    using NR::X;
    using NR::Y;
    NR::Point const ctr(bbox.midpoint());
    NR::Point const lengths(bbox.dimensions());
    if ( ctr == p ) {
	/* Arbitrarily choose centre of right edge. */
	return NR::Point(ctr[X] + .5 * lengths[X],
			 ctr[Y]);
    }
    NR::Point const cp( p - ctr );
    NR::Dim2 const edgeDim = ( ( fabs(lengths[Y] * cp[X]) <
	   		         fabs(lengths[X] * cp[Y])  )
			       ? Y
			       : X );
    NR::Dim2 const otherDim = (NR::Dim2) !edgeDim;
    NR::Point offset;
    offset[edgeDim] = (signed_one(cp[edgeDim])
		       * lengths[edgeDim]);
    offset[otherDim] = (lengths[edgeDim]
			* cp[otherDim]
			/ fabs(cp[edgeDim]));
    g_assert((offset[otherDim] >= 0) == (cp[otherDim] >= 0));
#ifndef NDEBUG
    for (unsigned d = 0; d < 2; ++d) {
	g_assert(fabs(offset[d]) <= lengths[d] + .125);
    }
#endif
    return ctr + .5 * offset;
}

static double signed_one(double const x)
{
    return (x < 0
	    ? -1.
	    : 1.);
}

static void
change_endpts(SPCurve *const curve, NR::Point const h2endPt[2])
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
