#include <2geom/coord.h>
#include "desktop.h"
#include "sp-guide.h"
#include "sp-guide-constraint.h"
#include "sp-namedview.h"
#include "satisfied-guide-cns.h"

void satisfied_guide_cns(SPDesktop const &desktop,
                         std::vector<Inkscape::SnapCandidatePoint> const &snappoints,
                         std::vector<SPGuideConstraint> &cns)
{
    SPNamedView const &nv = *desktop.getNamedView();
    for (GSList const *l = nv.guides; l != NULL; l = l->next) {
        SPGuide &g = *SP_GUIDE(l->data);
        for (unsigned int i = 0; i < snappoints.size(); ++i) {
            if (Geom::are_near(g.getDistanceFrom(snappoints[i].getPoint()), 0, 1e-2)) {
                cns.push_back(SPGuideConstraint(&g, i));
            }
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
