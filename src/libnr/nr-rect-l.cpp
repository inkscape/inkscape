#include <libnr/nr-rect-l.h>

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
