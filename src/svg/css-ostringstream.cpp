#include "svg/css-ostringstream.h"
#include "svg/strip-trailing-zeros.h"
#include "preferences.h"
#include <glib.h>

Inkscape::CSSOStringStream::CSSOStringStream()
{
    /* These two are probably unnecessary now that we provide our own operator<< for float and
     * double. */
    ostr.imbue(std::locale::classic());
    ostr.setf(std::ios::showpoint);

    /* This one is (currently) needed though, as we currently use ostr.precision as a sort of
       variable for storing the desired precision: see our two precision methods and our operator<<
       methods for float and double. */
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    ostr.precision(prefs->getInt("/options/svgoutput/numericprecision", 8));
}

static void
write_num(Inkscape::CSSOStringStream &os, unsigned const prec, double const d)
{
    char buf[32];  // haven't thought about how much is really required.
    switch (prec) {
        case 9: g_ascii_formatd(buf, sizeof(buf), "%.9f", d); break;
        case 8: g_ascii_formatd(buf, sizeof(buf), "%.8f", d); break;
        case 7: g_ascii_formatd(buf, sizeof(buf), "%.7f", d); break;
        case 6: g_ascii_formatd(buf, sizeof(buf), "%.6f", d); break;
        case 5: g_ascii_formatd(buf, sizeof(buf), "%.5f", d); break;
        case 4: g_ascii_formatd(buf, sizeof(buf), "%.4f", d); break;
        case 3: g_ascii_formatd(buf, sizeof(buf), "%.3f", d); break;
        case 2: g_ascii_formatd(buf, sizeof(buf), "%.2f", d); break;
        case 1: g_ascii_formatd(buf, sizeof(buf), "%.1f", d); break;
        case 0: g_ascii_formatd(buf, sizeof(buf), "%.0f", d); break;
        case 10: default: g_ascii_formatd(buf, sizeof(buf), "%.10f", d); break;
    }
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
