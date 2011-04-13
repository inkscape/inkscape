/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * Original file from libgimpwidgets: gimpeevl.h
 * Copyright (C) 2008-2009 Fredrik Alstromer <roe@excu.se>
 * Copyright (C) 2008-2009 Martin Nordholts <martinn@svn.gnome.org>
 * Modified for Inkscape by Johan Engelen
 * Copyright (C) 2011 Johan Engelen
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef __GIMP_EEVL_H__
#define __GIMP_EEVL_H__

#include "util/units.h"

#include <exception>
#include <sstream>
#include <string>

namespace Inkscape {
namespace Util {

class Unit;

/**
* GimpEevlQuantity:
* @value: In reference units.
* @dimension: in has a dimension of 1, in^2 has a dimension of 2 etc
*/
typedef struct
{
    double value;
    gint dimension;
} GimpEevlQuantity;

typedef bool (* GimpEevlUnitResolverProc) (const gchar      *identifier,
                                               GimpEevlQuantity *result,
                                               Unit* unit);

GimpEevlQuantity gimp_eevl_evaluate (const gchar* string, Unit* unit = NULL);

/**
 * Special exception class for the expression evaluator.
 */
class EvaluatorException : public std::exception {
public:
    EvaluatorException(const char * message, const char *at_position) {
        std::ostringstream os;
        const char* token = at_position ? at_position : "<End of input>";
        os << "Expression evaluator error: " << message << " at '" << token << "'";
        msgstr = os.str();
    }

    virtual ~EvaluatorException() throw() {} // necessary to destroy the string object!!!

    virtual const char* what() const throw () {
        return msgstr.c_str();
    }
protected:
    std::string msgstr;
};

}
}

#endif /* __GIMP_EEVL_H__ */
