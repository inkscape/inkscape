#ifndef INKSCAPE_MEMEQ_H
#define INKSCAPE_MEMEQ_H

#include <cstring>

/** Convenience/readability wrapper for memcmp(a,b,n)==0. */
inline bool
memeq(void const *a, void const *b, size_t n)
{
    return std::memcmp(a, b, n) == 0;
}


#endif /* !INKSCAPE_MEMEQ_H */

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
