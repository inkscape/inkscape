/**
 * \file
 * \brief  Defines the different types of exceptions that 2geom can throw.
 *
 * There are two main exception classes: LogicalError and RangeError.
 * Logical errors are 2geom faults/bugs; RangeErrors are 'user' faults,
 * e.g. invalid arguments to lib2geom methods.
 * This way, the 'user' can distinguish between groups of exceptions
 * ('user' is the coder that uses lib2geom)
 *
 * Several macro's are defined for easily throwing exceptions 
 * (e.g. THROW_CONTINUITYERROR). 
 */
/* Copyright 2007 Johan Engelen <goejendaagh@zonnet.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 */

#ifndef LIB2GEOM_SEEN_EXCEPTION_H
#define LIB2GEOM_SEEN_EXCEPTION_H

#include <exception>
#include <sstream>
#include <string>

namespace Geom {

/**
 * Base exception class, all 2geom exceptions should be derived from this one.
 */
class Exception : public std::exception {
public:
    Exception(const char * message, const char *file, const int line) {
        std::ostringstream os;
        os << "lib2geom exception: " << message << " (" << file << ":" << line << ")";
        msgstr = os.str();
    }

    virtual ~Exception() throw() {} // necessary to destroy the string object!!!

    virtual const char* what() const throw () {
        return msgstr.c_str();
    }
protected:
    std::string msgstr;
};
#define THROW_EXCEPTION(message) throw(Geom::Exception(message, __FILE__, __LINE__))

//-----------------------------------------------------------------------

class LogicalError : public Exception {
public:
    LogicalError(const char * message, const char *file, const int line)
        : Exception(message, file, line) {}
};
#define THROW_LOGICALERROR(message) throw(LogicalError(message, __FILE__, __LINE__))

class RangeError : public Exception {
public:
    RangeError(const char * message, const char *file, const int line)
        : Exception(message, file, line) {}
};
#define THROW_RANGEERROR(message) throw(RangeError(message, __FILE__, __LINE__))

//-----------------------------------------------------------------------
// Special case exceptions. Best used with the defines :)

class NotImplemented : public LogicalError {
public:
    NotImplemented(const char *file, const int line)
        : LogicalError("Method not implemented", file, line) {}
};
#define THROW_NOTIMPLEMENTED(i) throw(NotImplemented(__FILE__, __LINE__))

class InvariantsViolation : public LogicalError {
public:
    InvariantsViolation(const char *file, const int line)
        : LogicalError("Invariants violation", file, line) {}
};
#define THROW_INVARIANTSVIOLATION(i) throw(InvariantsViolation(__FILE__, __LINE__))
#define ASSERT_INVARIANTS(e)       ((e) ? (void)0 : THROW_INVARIANTSVIOLATION())

class NotInvertible : public RangeError {
public:
    NotInvertible(const char *file, const int line)
        : RangeError("Function does not have a unique inverse", file, line) {}
};
#define THROW_NOTINVERTIBLE(i) throw(NotInvertible(__FILE__, __LINE__))

class InfiniteSolutions : public RangeError {
public:
	InfiniteSolutions(const char *file, const int line)
        : RangeError("There are infinite solutions", file, line) {}
};
#define THROW_INFINITESOLUTIONS(i) throw(InfiniteSolutions(__FILE__, __LINE__))

class ContinuityError : public RangeError {
public:
    ContinuityError(const char *file, const int line)
        : RangeError("Non-contiguous path", file, line) {}
};
#define THROW_CONTINUITYERROR(i) throw(ContinuityError(__FILE__, __LINE__))

struct SVGPathParseError : public std::exception {
    char const *what() const throw() { return "parse error"; }
};


} // namespace Geom

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
