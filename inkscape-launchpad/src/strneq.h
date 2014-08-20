#ifndef INKSCAPE_STRNEQ_H
#define INKSCAPE_STRNEQ_H

#include <cstring>

/** Convenience/readability wrapper for strncmp(a,b,n)==0. */
inline bool
strneq(char const *a, char const *b, size_t n)
{
    return std::strncmp(a, b, n) == 0;
}


#endif /* !INKSCAPE_STRNEQ_H */

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
