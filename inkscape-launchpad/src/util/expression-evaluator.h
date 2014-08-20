/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * Original file from libgimpwidgets: gimpeevl.h
 * Copyright (C) 2008-2009 Fredrik Alstromer <roe@excu.se>
 * Copyright (C) 2008-2009 Martin Nordholts <martinn@svn.gnome.org>
 * Modified for Inkscape by Johan Engelen
 * Copyright (C) 2011 Johan Engelen
 * Copyright (C) 2013 Matthew Petroff
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

#ifndef INKSCAPE_UTIL_EXPRESSION_EVALUATOR_H
#define INKSCAPE_UTIL_EXPRESSION_EVALUATOR_H

#include "util/units.h"

#include <exception>
#include <sstream>
#include <string>

/**
 * @file
 * Expression evaluator: A straightforward recursive
 * descent parser, no fuss, no new dependencies. The lexer is hand
 * coded, tedious, not extremely fast but works. It evaluates the
 * expression as it goes along, and does not create a parse tree or
 * anything, and will not optimize anything. It uses doubles for
 * precision, with the given use case, that's enough to combat any
 * rounding errors (as opposed to optimizing the evalutation).
 *
 * It relies on external unit resolving through a callback and does
 * elementary dimensionality constraint check (e.g. "2 mm + 3 px * 4
 * in" is an error, as L + L^2 is a mismatch). It uses g_strtod() for numeric
 * conversions and it's non-destructive in terms of the parameters, and
 * it's reentrant.
 *
 * EBNF:
 *
 *   expression    ::= term { ('+' | '-') term }*  |
 *                     <empty string> ;
 *
 *   term          ::= exponent { ( '*' | '/' ) exponent }* ;
 *
 *   exponent      ::= signed factor { '^' signed factor }* ;
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
 * EvaluatorQuantity:
 * @param value         In reference units.
 * @param dimension     mm has a dimension of 1, mm^2 has a dimension of 2, etc.
 */
class EvaluatorQuantity
{
public:
    EvaluatorQuantity(double value = 0, unsigned int dimension = 0);
    
    double value;
    unsigned int dimension;
};

/**
 * TokenType
 */
enum {
  TOKEN_NUM        = 30000,
  TOKEN_IDENTIFIER = 30001,
  TOKEN_ANY        = 40000,
  TOKEN_END        = 50000
};
typedef int TokenType;

/**
 * EvaluatorToken
 */
class EvaluatorToken
{
public:
    EvaluatorToken();
    
    TokenType type;
    
    union {
        double fl;
        struct {
            const char *c;
            int size;
        };
    } value;
};

/**
 * ExpressionEvaluator
 * @param string    NULL terminated input string to evaluate
 * @param unit      Unit output should be in
 */
class ExpressionEvaluator
{
public:
    ExpressionEvaluator(const char *string, Unit const *unit = NULL);
    
    EvaluatorQuantity evaluate();

private:
    const char *string;
    Unit const *unit;
    
    EvaluatorToken current_token;
    const char *start_of_current_token;
    
    EvaluatorQuantity evaluateExpression();
    EvaluatorQuantity evaluateTerm();
    EvaluatorQuantity evaluateExpTerm();
    EvaluatorQuantity evaluateSignedFactor();
    EvaluatorQuantity evaluateFactor();
    
    bool acceptToken(TokenType token_type, EvaluatorToken *consumed_token);
    void parseNextToken();
    void acceptTokenCount(int count, TokenType token_type);
    void isExpected(TokenType token_type, EvaluatorToken *value);
    
    void movePastWhiteSpace();
    
    static bool isUnitIdentifierStart(gunichar c);
    static int getIdentifierSize(const char *s, int start);
    
    static bool resolveUnit(const char *identifier, EvaluatorQuantity *result, Unit const *unit);
    
    void throwError(const char *msg);
};

/**
 * Special exception class for the expression evaluator.
 */
class EvaluatorException : public std::exception {
public:
    EvaluatorException(const char *message, const char *at_position) {
        std::ostringstream os;
        const char *token = at_position ? at_position : "<End of input>";
        os << "Expression evaluator error: " << message << " at '" << token << "'";
        msgstr = os.str();
    }

    virtual ~EvaluatorException() throw() {} // necessary to destroy the string object!!!

    virtual const char *what() const throw () {
        return msgstr.c_str();
    }
protected:
    std::string msgstr;
};

}
}

#endif // INKSCAPE_UTIL_EXPRESSION_EVALUATOR_H
