#ifndef INKSCAPE_STREQ_H
#define INKSCAPE_STREQ_H

#include <cstring>

/** Convenience/readability wrapper for strcmp(a,b)==0. */
inline bool
streq(char const *a, char const *b)
{
    return std::strcmp(a, b) == 0;
}

struct streq_rel {
    bool operator()(char const *a, char const *b) const
    {
        return (std::strcmp(a, b) == 0);
    }
};

#endif /* !INKSCAPE_STREQ_H */

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
