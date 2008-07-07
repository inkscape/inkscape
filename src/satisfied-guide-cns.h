#ifndef __SATISFIED_GUIDE_CNS_H__
#define __SATISFIED_GUIDE_CNS_H__

#include <forward.h>
#include <libnr/nr-forward.h>
#include <vector>
class SPGuideConstraint;

void satisfied_guide_cns(SPDesktop const &desktop,
                         std::vector<NR::Point> const &snappoints,
                         std::vector<SPGuideConstraint> &cns);


#endif /* !__SATISFIED_GUIDE_CNS_H__ */

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
