#ifndef SEEN_BAD_URI_EXCEPTION_H
#define SEEN_BAD_URI_EXCEPTION_H

#include <exception>

namespace Inkscape {

class BadURIException : public std::exception {};

class UnsupportedURIException : public BadURIException {
public:
    char const *what() const throw() { return "Unsupported URI"; }
};

class MalformedURIException : public BadURIException {
public:
    char const *what() const throw() { return "Malformed URI"; }
};

}


#endif /* !SEEN_BAD_URI_EXCEPTION_H */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
