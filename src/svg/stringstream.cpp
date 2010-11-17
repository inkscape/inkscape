#include "svg/stringstream.h"
#include "svg/strip-trailing-zeros.h"
#include "preferences.h"
#include <2geom/point.h>

Inkscape::SVGOStringStream::SVGOStringStream()
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

Inkscape::SVGOStringStream &
operator<<(Inkscape::SVGOStringStream &os, float d)
{
    /* Try as integer first. */
    {
        int const n = int(d);
        if (d == n) {
            os << n;
            return os;
        }
    }

    std::ostringstream s;
    s.imbue(std::locale::classic());
    s.flags(os.setf(std::ios::showpoint));
    s.precision(os.precision());
    s << d;
    os << strip_trailing_zeros(s.str());
    return os;
}

Inkscape::SVGOStringStream &
operator<<(Inkscape::SVGOStringStream &os, double d)
{
    /* Try as integer first. */
    {
        int const n = int(d);
        if (d == n) {
            os << n;
            return os;
        }
    }

    std::ostringstream s;
    s.imbue(std::locale::classic());
    s.flags(os.setf(std::ios::showpoint));
    s.precision(os.precision());
    s << d;
    os << strip_trailing_zeros(s.str());
    return os;
}

Inkscape::SVGOStringStream &
operator<<(Inkscape::SVGOStringStream &os, Geom::Point const & p)
{
    os << p[0] << ',' << p[1];
    return os;
}

Inkscape::SVGIStringStream::SVGIStringStream():std::istringstream()
{
    this->imbue(std::locale::classic());
    this->setf(std::ios::showpoint);

    /* This one is (currently) needed though, as we currently use ostr.precision as a sort of
       variable for storing the desired precision: see our two precision methods and our operator<<
       methods for float and double. */
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    this->precision(prefs->getInt("/options/svgoutput/numericprecision", 8));
}

Inkscape::SVGIStringStream::SVGIStringStream(const std::string& str):std::istringstream(str)
{
    this->imbue(std::locale::classic());
    this->setf(std::ios::showpoint);

    /* This one is (currently) needed though, as we currently use ostr.precision as a sort of
       variable for storing the desired precision: see our two precision methods and our operator<<
       methods for float and double. */
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    this->precision(prefs->getInt("/options/svgoutput/numericprecision", 8));
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
