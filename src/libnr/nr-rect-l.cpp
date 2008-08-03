#include <libnr/nr-rect-l.h>

NR::Maybe<NR::Rect> NRRectL::upgrade() const {
    if (nr_rect_l_test_empty_ptr(this)) {
        return NR::Nothing();
    } else {
        return NR::Rect(NR::Point(x0, y0), NR::Point(x1, y1));
    }
}

namespace NR {

IRect::IRect(Rect const &r) :
    _min(int(floor(r.min()[X])), int(floor(r.min()[Y]))),
    _max(int(ceil(r.min()[X])), int(ceil(r.min()[Y])))
{
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
