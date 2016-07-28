
#include "satisfied-guide-cns.h"
#include "sp-guide-constraint.h"
#include "sp-item-update-cns.h"
#include "sp-guide.h"
#include "sp-item.h"
#include <algorithm>
using std::find;
using std::vector;

void sp_item_update_cns(SPItem &item, SPDesktop const &desktop)
{
    std::vector<Inkscape::SnapCandidatePoint> snappoints;
    item.getSnappoints(snappoints, NULL);
    /* TODO: Implement the ordering. */
    vector<SPGuideConstraint> found_cns;
    satisfied_guide_cns(desktop, snappoints, found_cns);
    /* effic: It might be nice to avoid an n^2 algorithm, but in practice n will be
       small enough that it's still usually more efficient. */

    for (vector<SPGuideConstraint>::const_iterator fi(found_cns.begin()),
             fiEnd(found_cns.end());
         fi != fiEnd; ++fi)
    {
        SPGuideConstraint const &cn = *fi;
        if ( find(item.constraints.begin(),
                  item.constraints.end(),
                  cn)
             == item.constraints.end() )
        {
            item.constraints.push_back(cn);
            cn.g->attached_items.push_back(SPGuideAttachment(&item, cn.snappoint_ix));
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
