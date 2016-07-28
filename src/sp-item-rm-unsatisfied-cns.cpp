
#include <algorithm>
#include <2geom/coord.h>

#include "remove-last.h"
#include "sp-guide.h"
#include "sp-guide-constraint.h"
#include "sp-item.h"
#include "sp-item-rm-unsatisfied-cns.h"

using std::vector;

void sp_item_rm_unsatisfied_cns(SPItem &item)
{
    if (item.constraints.empty()) {
        return;
    }
    std::vector<Inkscape::SnapCandidatePoint> snappoints;
    item.getSnappoints(snappoints, NULL);
    for (unsigned i = item.constraints.size(); i--;) {
        g_assert( i < item.constraints.size() );
        SPGuideConstraint const &cn = item.constraints[i];
        int const snappoint_ix = cn.snappoint_ix;
        g_assert( snappoint_ix < int(snappoints.size()) );

        if (!Geom::are_near(cn.g->getDistanceFrom(snappoints[snappoint_ix].getPoint()), 0, 1e-2)) {

            remove_last(cn.g->attached_items, SPGuideAttachment(&item, cn.snappoint_ix));

            g_assert( i < item.constraints.size() );

            item.constraints.erase(item.constraints.begin() + i);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
