#ifndef INKSCAPE_STRINGSTREAM_H
#define INKSCAPE_STRINGSTREAM_H

#include <sstream>
#include <string>

#include <2geom/forward.h>

namespace Inkscape {

typedef std::ios_base &(*std_oct_type)(std::ios_base &);

class SVGOStringStream {
private:
    std::ostringstream ostr;

public:
    SVGOStringStream();

#define INK_SVG_STR_OP(_t) \
    SVGOStringStream &operator<<(_t arg) {  \
        ostr << arg;    \
        return *this;   \
    }

    INK_SVG_STR_OP(char)
    INK_SVG_STR_OP(signed char)
    INK_SVG_STR_OP(unsigned char)
    INK_SVG_STR_OP(short)
    INK_SVG_STR_OP(unsigned short)
    INK_SVG_STR_OP(int)
    INK_SVG_STR_OP(unsigned int)
    INK_SVG_STR_OP(long)
    INK_SVG_STR_OP(unsigned long)
    INK_SVG_STR_OP(char const *)
    INK_SVG_STR_OP(signed char const *)
    INK_SVG_STR_OP(unsigned char const *)
    INK_SVG_STR_OP(std::string const &)
    INK_SVG_STR_OP(std_oct_type)

#undef INK_SVG_STR_OP

    char const *gcharp() const {
        return ostr.str().c_str();
    }

    std::string str() const {
        return ostr.str();
    }
    
    void str (std::string &s) {
        ostr.str(s);
    }

    std::streamsize precision() const {
        return ostr.precision();
    }

    std::streamsize precision(std::streamsize p) {
        return ostr.precision(p);
    }

    std::ios::fmtflags setf(std::ios::fmtflags fmtfl) {
        return ostr.setf(fmtfl);
    }

    std::ios::fmtflags setf(std::ios::fmtflags fmtfl, std::ios::fmtflags mask) {
        return ostr.setf(fmtfl, mask);
    }

    void unsetf(std::ios::fmtflags mask) {
        ostr.unsetf(mask);
    }
};

class SVGIStringStream:public std::istringstream {

public:
    SVGIStringStream();
    SVGIStringStream(const std::string &str);
};

}

Inkscape::SVGOStringStream &operator<<(Inkscape::SVGOStringStream &os, float d);

Inkscape::SVGOStringStream &operator<<(Inkscape::SVGOStringStream &os, double d);

Inkscape::SVGOStringStream &operator<<(Inkscape::SVGOStringStream &os, Geom::Point const & p);

#endif

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
