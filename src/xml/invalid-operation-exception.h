/*
 * Inkscape::XML::InvalidOperationException - invalid operation for node type
 *
 * Copyright 2004-2005 MenTaLguY <mental@rydia.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 *
 */

#ifndef SEEN_INKSCAPE_XML_INVALID_OPERATION_EXCEPTION_H
#define SEEN_INKSCAPE_XML_INVALID_OPERATION_EXCEPTION_H

#include <exception>
#include <stdexcept>

namespace Inkscape {

namespace XML {

class InvalidOperationException : public std::logic_error {
public:
    InvalidOperationException(std::string const &message) :
        std::logic_error(message)
    { }
};

}

}

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
