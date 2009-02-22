
#include <algorithm>

#include "approx-equal.h"
#include "remove-last.h"
#include "sp-guide.h"
#include "sp-guide-constraint.h"
#include "sp-item.h"

using std::vector;

void sp_item_rm_unsatisfied_cns(SPItem &item)
{
    if (item.constraints.empty()) {
        return;
    }
    std::vector<Geom::Point> snappoints;
    sp_item_snappoints(&item, SnapPointsIter(snappoints), NULL);
    for (unsigned i = item.constraints.size(); i--;) {
        g_assert( i < item.constraints.size() );
        SPGuideConstraint const &cn = item.constraints[i];
        int const snappoint_ix = cn.snappoint_ix;
        g_assert( snappoint_ix < int(snappoints.size()) );
        if (!approx_equal( sp_guide_distance_from_pt(cn.g, snappoints[snappoint_ix]), 0) ) {
            remove_last(cn.g->attached_items, SPGuideAttachment(&item, cn.snappoint_ix));
            g_assert( i < item.constraints.size() );
            vector<SPGuideConstraint>::iterator const ei(&item.constraints[i]);
            item.constraints.erase(ei);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
