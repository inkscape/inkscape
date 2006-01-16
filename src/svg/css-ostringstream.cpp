#include "svg/css-ostringstream.h"
#include "svg/strip-trailing-zeros.h"
#include <glib/gmessages.h>
#include <glib/gstrfuncs.h>

Inkscape::CSSOStringStream::CSSOStringStream()
{
    /* These two are probably unnecessary now that we provide our own operator<< for float and
     * double. */
    ostr.imbue(std::locale::classic());
    ostr.setf(std::ios::showpoint);

    /* This one is (currently) needed though, as we currently use ostr.precision as a sort of
       variable for storing the desired precision: see our two precision methods and our operator<<
       methods for float and double. */
    ostr.precision(8);
}

static void
write_num(Inkscape::CSSOStringStream &os, unsigned const prec, double const d)
{
    char buf[32];  // haven't thought about how much is really required.
    if (prec != 8) {
        static bool warned;
        if (!warned) {
            g_warning("Using precision of 8 instead of the requested %u.  Won't re-warn.", prec);
            warned = true;
        }
    }
    g_ascii_formatd(buf, sizeof(buf), "%.8f", d);
    os << strip_trailing_zeros(buf);
}

Inkscape::CSSOStringStream &
operator<<(Inkscape::CSSOStringStream &os, float const d)
{
    /* Try as integer first. */
    {
        long const n = long(d);
        if (d == n) {
            os << n;
            return os;
        }
    }

    write_num(os, os.precision(), d);
    return os;
}

Inkscape::CSSOStringStream &
operator<<(Inkscape::CSSOStringStream &os, double const d)
{
    /* Try as integer first. */
    {
        long const n = long(d);
        if (d == n) {
            os << n;
            return os;
        }
    }

    write_num(os, os.precision(), d);
    return os;
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
