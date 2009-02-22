#include <desktop-handles.h>
#include <libnr/nr-point-fns.h>
#include <sp-guide.h>
#include <sp-guide-constraint.h>
#include <sp-namedview.h>
#include <approx-equal.h>

void satisfied_guide_cns(SPDesktop const &desktop,
                         std::vector<Geom::Point> const &snappoints,
                         std::vector<SPGuideConstraint> &cns)
{
    SPNamedView const &nv = *sp_desktop_namedview(&desktop);
    for (GSList const *l = nv.guides; l != NULL; l = l->next) {
        SPGuide &g = *SP_GUIDE(l->data);
        for (unsigned int i = 0; i < snappoints.size(); ++i) {
            if (approx_equal( sp_guide_distance_from_pt(&g, snappoints[i]), 0) ) {
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
