/** \file
 * Implementation of sp_item_notify_moveto().
 */

#include <sp-item.h>
#include <2geom/transforms.h>
#include <sp-guide.h>
#include <sp-item-rm-unsatisfied-cns.h>
#include <sp-item-notify-moveto.h>
using std::vector;

#define return_if_fail(test) if (!(test)) { printf("WARNING: assertion '%s' failed", #test); return; }

/**
 * Called by sp_guide_moveto to indicate that the guide line corresponding to g has been moved, and
 * that consequently this item should move with it.
 *
 * \pre exist [cn in item.constraints] g eq cn.g.
 */
void sp_item_notify_moveto(SPItem &item, SPGuide const &mv_g, int const snappoint_ix,
                           double const position, bool const commit)
{
    return_if_fail(SP_IS_ITEM(&item));
    return_if_fail( unsigned(snappoint_ix) < 8 );
    Geom::Point const dir( mv_g.getNormal() );
    double const dir_lensq(dot(dir, dir));
    return_if_fail( dir_lensq != 0 );

    std::vector<Inkscape::SnapCandidatePoint> snappoints;
    item.getSnappoints(snappoints, NULL);
    return_if_fail( snappoint_ix < int(snappoints.size()) );

    double const pos0 = dot(dir, snappoints[snappoint_ix].getPoint());
    /// \todo effic: skip if mv_g is already satisfied.

    /* Translate along dir to make dot(dir, snappoints(item)[snappoint_ix]) == position. */

    /* Calculation:
       dot(dir, snappoints[snappoint_ix] + s * dir) = position.
       dot(dir, snappoints[snappoint_ix]) + dot(dir, s * dir) = position.
       pos0 + s * dot(dir, dir) = position.
       s * lensq(dir) = position - pos0.
       s = (position - pos0) / dot(dir, dir). */
    Geom::Translate const tr( ( position - pos0 )
                            * ( dir / dir_lensq ) );
    item.set_i2d_affine(item.i2dt_affine() * tr);
    /// \todo Reget snappoints, check satisfied.

    if (commit) {
        /// \todo Consider maintaining a set of dirty items.

        /* Commit repr. */
        {
            item.doWriteTransform(item.getRepr(), item.transform);
        }

        sp_item_rm_unsatisfied_cns(item);
#if 0 /* nyi */
        move_cn_to_front(mv_g, snappoint_ix, item.constraints);
        /** \note If the guideline is connected to multiple snappoints of
         * this item, then keeping those cns in order requires that the
         * guide send notifications in order of increasing importance.
         */
#endif
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
