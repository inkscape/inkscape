#ifndef SVG_CSS_OSTRINGSTREAM_H_INKSCAPE
#define SVG_CSS_OSTRINGSTREAM_H_INKSCAPE

#include <sstream>

namespace Inkscape {

typedef std::ios_base &(*std_oct_type)(std::ios_base &);

/**
 * A thin wrapper around std::ostringstream, but writing floating point numbers in the format
 * required by CSS: `.' as decimal separator, no `e' notation, no nan or inf.
 */
class CSSOStringStream {
private:
    std::ostringstream ostr;

public:
    CSSOStringStream();

#define INK_CSS_STR_OP(_t) \
    CSSOStringStream &operator<<(_t arg) {  \
        ostr << arg;    \
        return *this;   \
    }

    INK_CSS_STR_OP(char)
    INK_CSS_STR_OP(signed char)
    INK_CSS_STR_OP(unsigned char)
    INK_CSS_STR_OP(short)
    INK_CSS_STR_OP(unsigned short)
    INK_CSS_STR_OP(int)
    INK_CSS_STR_OP(unsigned int)
    INK_CSS_STR_OP(long)
    INK_CSS_STR_OP(unsigned long)
    INK_CSS_STR_OP(char const *)
    INK_CSS_STR_OP(signed char const *)
    INK_CSS_STR_OP(unsigned char const *)
    INK_CSS_STR_OP(std::string const &)
    INK_CSS_STR_OP(std_oct_type)

#undef INK_CSS_STR_OP

    char const *gcharp() const {
        return ostr.str().c_str();
    }

    std::string str() const {
        return ostr.str();
    }

    std::streamsize precision() const {
        return ostr.precision();
    }

    std::streamsize precision(std::streamsize p) {
        return ostr.precision(p);
    }
};

}

Inkscape::CSSOStringStream &operator<<(Inkscape::CSSOStringStream &os, float d);

Inkscape::CSSOStringStream &operator<<(Inkscape::CSSOStringStream &os, double d);


#endif /* !SVG_CSS_OSTRINGSTREAM_H_INKSCAPE */

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
