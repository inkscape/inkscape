/**
 *  \file object-snapper.cpp
 *  \brief Snapping things to objects.
 *
 * Authors:
 *   Carl Hetherington <inkscape@carlh.net>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "libnr/n-art-bpath.h"
#include "libnr/nr-rect-ops.h"
#include "document.h"
#include "sp-namedview.h"
#include "sp-path.h"
#include "display/curve.h"
#include "desktop.h"
#include "inkscape.h"
#include "splivarot.h"


Inkscape::ObjectSnapper::ObjectSnapper(SPNamedView const *nv, NR::Coord const d)
    : Snapper(nv, d), _snap_to_nodes(true), _snap_to_paths(true)
{

}


/**
 *  \param p Point we are trying to snap (desktop coordinates)
 */

void Inkscape::ObjectSnapper::_findCandidates(std::list<SPItem*>& c,
                                              SPObject* r,
                                              std::list<SPItem const *> const &it,
                                              NR::Point const &p) const
{
    for (SPObject* o = r->children; o != NULL; o = o->next) {
        if (SP_IS_ITEM(o)) {

            /* See if this item is on the ignore list */
            std::list<SPItem const *>::const_iterator i = it.begin();
            while (i != it.end() && *i != o) {
                i++;
            }

            if (i == it.end()) {
                /* See if the item is within range */
                NR::Rect const b = NR::expand(sp_item_bbox_desktop(SP_ITEM(o)), -getDistance());
                if (b.contains(p)) {
                    c.push_back(SP_ITEM(o));
                }
            }
        }

        _findCandidates(c, o, it, p);
    }
}


void Inkscape::ObjectSnapper::_snapNodes(Inkscape::SnappedPoint &s,
                                         NR::Point const &p,
                                         std::list<SPItem*> const &cand) const
{
    /* FIXME: this seems like a hack.  Perhaps Snappers should be
    ** in SPDesktop rather than SPNamedView?
    */
    SPDesktop const *desktop = SP_ACTIVE_DESKTOP;

    for (std::list<SPItem*>::const_iterator i = cand.begin(); i != cand.end(); i++) {
        if (SP_IS_SHAPE(*i)) {

            SPShape const *sh = SP_SHAPE(*i);
            if (sh->curve) {

                int j = 0;
                NR::Matrix const i2doc = sp_item_i2doc_affine(*i);

                while (sh->curve->bpath[j].code != NR_END) {

                    /* Get this node in desktop coordinates */
                    NArtBpath const &bp = sh->curve->bpath[j];
                    NR::Point const n = desktop->doc2dt(bp.c(3) * i2doc);

                    /* Try to snap to this node of the path */
                    NR::Coord const dist = NR::L2(n - p);
                    if (dist < getDistance() && dist < s.getDistance()) {
                        s = SnappedPoint(n, dist);
                    }

                    j++;
                }
            }
        }
    }
}


void Inkscape::ObjectSnapper::_snapPaths(Inkscape::SnappedPoint &s,
                                         NR::Point const &p,
                                         std::list<SPItem*> const &cand) const
{
    /* FIXME: this seems like a hack.  Perhaps Snappers should be
    ** in SPDesktop rather than SPNamedView?
    */
    SPDesktop const *desktop = SP_ACTIVE_DESKTOP;

    NR::Point const p_doc = desktop->dt2doc(p);

    for (std::list<SPItem*>::const_iterator i = cand.begin(); i != cand.end(); i++) {

        /* Transform the requested snap point to this item's coordinates */
        NR::Matrix const i2doc = sp_item_i2doc_affine(*i);
        NR::Point const p_it = p_doc * i2doc.inverse();

        Path *livarot_path = Path_for_item(*i, true, true);
        if (!livarot_path)
            continue;

        livarot_path->ConvertWithBackData(0.01);

        /* Look for the nearest position on this SPItem to our snap point */
        NR::Maybe<Path::cut_position> const o = get_nearest_position_on_Path(livarot_path, p_it);
        if (o != NR::Nothing() && o.assume().t >= 0 && o.assume().t <= 1) {

            /* Convert the nearest point back to desktop coordinates */
            NR::Point const o_it = get_point_on_Path(livarot_path, o.assume().piece, o.assume().t);
            NR::Point const o_dt = desktop->doc2dt(o_it * i2doc);

            NR::Coord const dist = NR::L2(o_dt - p);
            if (dist < getDistance() && dist < s.getDistance()) {
                s = SnappedPoint(o_dt, dist);
            }
        }

        delete livarot_path;
    }

}


Inkscape::SnappedPoint Inkscape::ObjectSnapper::_doFreeSnap(NR::Point const &p,
                                                            std::list<SPItem const *> const &it) const
{
    /* Get a list of all the SPItems that we will try to snap to */
    std::list<SPItem*> cand;
    _findCandidates(cand, sp_document_root(_named_view->document), it, p);

    SnappedPoint s(p, NR_HUGE);

    if (_snap_to_nodes) {
        _snapNodes(s, p, cand);
    }
    if (_snap_to_paths) {
        _snapPaths(s, p, cand);
    }

    return s;
}



Inkscape::SnappedPoint Inkscape::ObjectSnapper::_doConstrainedSnap(NR::Point const &p,
                                                                   NR::Point const &c,
                                                                   std::list<SPItem const *> const &it) const
{
    /* FIXME: this needs implementing properly; I think we have to do the
    ** intersection of c with the objects.
    */
    return _doFreeSnap(p, it);
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
