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

#ifndef SEEN_GIMP_EEVL_H
#define SEEN_GIMP_EEVL_H

#include "util/units.h"

#include <exception>
#include <sstream>
#include <string>

/**
 * @file
 * Introducing eevl eva, the evaluator. A straightforward recursive
 * descent parser, no fuss, no new dependencies. The lexer is hand
 * coded, tedious, not extremely fast but works. It evaluates the
 * expression as it goes along, and does not create a parse tree or
 * anything, and will not optimize anything. It uses doubles for
 * precision, with the given use case, that's enough to combat any
 * rounding errors (as opposed to optimizing the evalutation).
 *
 * It relies on external unit resolving through a callback and does
 * elementary dimensionality constraint check (e.g. "2 mm + 3 px * 4
 * in" is an error, as L + L^2 is a missmatch). It uses g_strtod() for numeric
 * conversions and it's non-destructive in terms of the paramters, and
 * it's reentrant.
 *
 * EBNF:
 *
 *   expression    ::= term { ('+' | '-') term }*  |
 *                     <empty string> ;
 *
 *   term          ::= signed factor { ( '*' | '/' ) signed factor }* ;
 *
 *   signed factor ::= ( '+' | '-' )? factor ;
 *
 *   unit factor   ::= factor unit? ;
 *
 *   factor        ::= number | '(' expression ')' ;
 *
 *   number        ::= ? what g_strtod() consumes ? ;
 *
 *   unit          ::= ? what not g_strtod() consumes and not whitespace ? ;
 *
 * The code should match the EBNF rather closely (except for the
 * non-terminal unit factor, which is inlined into factor) for
 * maintainability reasons.
 *
 * It will allow 1++1 and 1+-1 (resulting in 2 and 0, respectively),
 * but I figured one might want that, and I don't think it's going to
 * throw anyone off.
 */

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

#endif // SEEN_GIMP_EEVL_H
