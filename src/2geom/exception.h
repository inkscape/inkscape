#ifndef LIB2GEOM_EXCEPTION_HEADER
#define LIB2GEOM_EXCEPTION_HEADER

/** Defines the different types of exceptions that 2geom can throw.
 *
 * Copyright 2007 Johan Engelen <goejendaagh@zonnet.nl>
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

#include <exception>
#include <sstream>
#include <string>

namespace Geom {

// Base exception class, all 2geom exceptions should be derrived from this one.
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
#define throwException(message) throw(Geom::Exception(message, __FILE__, __LINE__))

//-----------------------------------------------------------------------
// Two main exception classes: LogicalError and RangeError.
// Logical errors are 2geom faults/bugs, RangeErrors are 'user' faults.
// This way, the 'user' can distinguish between groups of exceptions
// ('user' is the coder that uses lib2geom)
class LogicalError : public Exception {
public:
    LogicalError(const char * message, const char *file, const int line)
        : Exception(message, file, line) {}
};
#define throwLogicalError(message) throw(LogicalError(message, __FILE__, __LINE__))

class RangeError : public Exception {
public:
    RangeError(const char * message, const char *file, const int line)
        : Exception(message, file, line) {}
};
#define throwRangeError(message) throw(RangeError(message, __FILE__, __LINE__))

//-----------------------------------------------------------------------
// Special case exceptions. Best used with the defines :)

class NotImplemented : public LogicalError {
public:
    NotImplemented(const char *file, const int line)
        : LogicalError("Method not implemented", file, line) {}
};
#define throwNotImplemented(i) throw(NotImplemented(__FILE__, __LINE__))

class InvariantsViolation : public LogicalError {
public:
    InvariantsViolation(const char *file, const int line)
        : LogicalError("Invariants violation", file, line) {}
};
#define throwInvariantsViolation(i) throw(InvariantsViolation(__FILE__, __LINE__))
#define assert_invariants(e)       ((e) ? (void)0 : throwInvariantsViolation())

class NotInvertible : public RangeError {
public:
    NotInvertible(const char *file, const int line)
        : RangeError("Function does not have a unique inverse", file, line) {}
};
#define throwNotInvertible(i) throw(NotInvertible(__FILE__, __LINE__))

class ContinuityError : public RangeError {
public:
    ContinuityError(const char *file, const int line)
        : RangeError("Non-contiguous path", file, line) {}
};
#define throwContinuityError(i) throw(ContinuityError(__FILE__, __LINE__))

struct SVGPathParseError : public std::exception {
    char const *what() const throw() { return "parse error"; }
};


} // namespace Geom

#endif
