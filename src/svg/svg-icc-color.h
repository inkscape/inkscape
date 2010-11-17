#ifndef SVG_ICC_COLOR_H_SEEN
#define SVG_ICC_COLOR_H_SEEN

#include <string>
#include <vector>

/**
 * An icc-color specification.  Corresponds to the DOM interface of the same name.
 *
 * Referenced by SPIPaint.
 */
struct SVGICCColor {
    std::string colorProfile;
    std::vector<double> colors;
};


#endif /* !SVG_ICC_COLOR_H_SEEN */

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
