#ifndef SEEN_NR_TRANSLATE_H
#define SEEN_NR_TRANSLATE_H

#include <libnr/nr-point.h>

namespace NR {

class translate {
public:
    Point offset;
private:
    translate();
public:
    explicit translate(Point const &p) : offset(p) {}
    explicit translate(Coord const x, Coord const y) : offset(x, y) {}
    Coord operator[](Dim2 const dim) const { return offset[dim]; }
    Coord operator[](unsigned const dim) const { return offset[dim]; }
};

} /* namespace NR */


#endif /* !SEEN_NR_TRANSLATE_H */

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
