/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * Original file from libgimpwidgets: gimpeevl.c
 * Copyright (C) 2008 Fredrik Alstromer <roe@excu.se>
 * Copyright (C) 2008 Martin Nordholts <martinn@svn.gnome.org>
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

#include "config.h"

#include "util/expression-evaluator.h"
#include "util/units.h"

#include <glib/gconvert.h>

#include <math.h>
#include <string.h>

using Inkscape::Util::unit_table;

namespace Inkscape {
namespace Util {

EvaluatorQuantity::EvaluatorQuantity(double value, unsigned int dimension) :
    value(value),
    dimension(dimension)
{
}

EvaluatorToken::EvaluatorToken()
{
    type = 0;
    value.fl = 0;
}

ExpressionEvaluator::ExpressionEvaluator(const char *string, Unit const *unit) :
    string(g_locale_to_utf8(string,-1,0,0,0)),
    unit(unit)
{
    current_token.type  = TOKEN_END;
    
    // Preload symbol
    parseNextToken();
}

/**
 * Evaluates the given arithmetic expression, along with an optional dimension
 * analysis, and basic unit conversions.
 *
 * All units conversions factors are relative to some implicit
 * base-unit. This is also the unit of the returned value.
 *
 * Returns: An EvaluatorQuantity with a value given in the base unit along with
 * the order of the dimension (e.g. if the base unit is inches, a dimension
 * order of two means in^2).
 *
 * @return Result of evaluation.
 * @throws Inkscape::Util::EvaluatorException There was a parse error.
 **/
EvaluatorQuantity ExpressionEvaluator::evaluate()
{
    if (!g_utf8_validate(string, -1, NULL)) {
        throw EvaluatorException("Invalid UTF8 string", NULL);
    }
    
    EvaluatorQuantity result = EvaluatorQuantity();
    EvaluatorQuantity default_unit_factor;
    
    // Empty expression evaluates to 0
    if (acceptToken(TOKEN_END, NULL)) {
        return result;
    }
    
    result = evaluateExpression();
    
    // There should be nothing left to parse by now
    isExpected(TOKEN_END, 0);
    
    resolveUnit(NULL, &default_unit_factor, unit);
    
    // Entire expression is dimensionless, apply default unit if applicable
    if ( result.dimension == 0 && default_unit_factor.dimension != 0 ) {
        result.value     /= default_unit_factor.value;
        result.dimension  = default_unit_factor.dimension;
    }
    return result;
}

EvaluatorQuantity ExpressionEvaluator::evaluateExpression()
{
    bool subtract;
    EvaluatorQuantity evaluated_terms;
    
    evaluated_terms = evaluateTerm();
    
    // Continue evaluating terms, chained with + or -.
    for (subtract = FALSE;
        acceptToken('+', NULL) || (subtract = acceptToken('-', NULL));
        subtract = FALSE)
    {
        EvaluatorQuantity new_term = evaluateTerm();
        
        // If dimensions mismatch, attempt default unit assignent
        if ( new_term.dimension != evaluated_terms.dimension ) {
            EvaluatorQuantity default_unit_factor;
            
            resolveUnit(NULL, &default_unit_factor, unit);
            
            if ( new_term.dimension == 0
                && evaluated_terms.dimension == default_unit_factor.dimension )
            {
                new_term.value     /= default_unit_factor.value;
                new_term.dimension  = default_unit_factor.dimension;
            } else if ( evaluated_terms.dimension == 0
                && new_term.dimension == default_unit_factor.dimension )
            {
                evaluated_terms.value     /= default_unit_factor.value;
                evaluated_terms.dimension  = default_unit_factor.dimension;
            } else {
                throwError("Dimension mismatch during addition");
            }
        }
        
        evaluated_terms.value += (subtract ? -new_term.value : new_term.value);
    }
    
    return evaluated_terms;
}

EvaluatorQuantity ExpressionEvaluator::evaluateTerm()
{
    bool division;
    EvaluatorQuantity evaluated_exp_terms = evaluateExpTerm();
    
    for ( division = false;
        acceptToken('*', NULL) || (division = acceptToken('/', NULL));
        division = false )
    {
        EvaluatorQuantity new_exp_term = evaluateExpTerm();
        
        if (division) {
            evaluated_exp_terms.value     /= new_exp_term.value;
            evaluated_exp_terms.dimension -= new_exp_term.dimension;
        } else {
            evaluated_exp_terms.value     *= new_exp_term.value;
            evaluated_exp_terms.dimension += new_exp_term.dimension;
        }
    }
    
    return evaluated_exp_terms;
}

EvaluatorQuantity ExpressionEvaluator::evaluateExpTerm()
{
    EvaluatorQuantity evaluated_signed_factors = evaluateSignedFactor();
    
    while(acceptToken('^', NULL)) {
        EvaluatorQuantity new_signed_factor = evaluateSignedFactor();
        
        if (new_signed_factor.dimension == 0) {
            evaluated_signed_factors.value = pow(evaluated_signed_factors.value,
                                                 new_signed_factor.value);
            evaluated_signed_factors.dimension *= new_signed_factor.value;
        } else {
            throwError("Unit in exponent");
        }
    }
    
    return evaluated_signed_factors;
}

EvaluatorQuantity ExpressionEvaluator::evaluateSignedFactor()
{
    EvaluatorQuantity result;
    bool negate = FALSE;
    
    if (!acceptToken('+', NULL)) {
        negate = acceptToken ('-', NULL);
    }
    
    result = evaluateFactor();
    
    if (negate) {
        result.value = -result.value;
    }
    
    return result;
}

EvaluatorQuantity ExpressionEvaluator::evaluateFactor()
{
    EvaluatorQuantity evaluated_factor = EvaluatorQuantity();
    EvaluatorToken consumed_token = EvaluatorToken();

    if (acceptToken(TOKEN_END, &consumed_token)) {
        return evaluated_factor;
    }
    else if (acceptToken(TOKEN_NUM, &consumed_token)) {
        evaluated_factor.value = consumed_token.value.fl;
    } else if (acceptToken('(', NULL)) {
        evaluated_factor = evaluateExpression();
        isExpected(')', 0);
    } else {
        throwError("Expected number or '('");
    }
    
    if ( current_token.type == TOKEN_IDENTIFIER ) {
        char *identifier;
        EvaluatorQuantity result;
        
        acceptToken(TOKEN_ANY, &consumed_token);
        
        identifier = g_newa(char, consumed_token.value.size + 1);
        
        strncpy(identifier, consumed_token.value.c, consumed_token.value.size);
        identifier[consumed_token.value.size] = '\0';
        
        if (resolveUnit(identifier, &result, unit)) {
            evaluated_factor.value      /= result.value;
            evaluated_factor.dimension  += result.dimension;
        } else {
            throwError("Unit was not resolved");
        }
    }
    
    return evaluated_factor;
}

bool ExpressionEvaluator::acceptToken(TokenType token_type,
                                      EvaluatorToken *consumed_token)
{
    bool existed = FALSE;
    
    if ( token_type == current_token.type || token_type == TOKEN_ANY ) {
        existed = TRUE;
        
        if (consumed_token) {
            *consumed_token = current_token;
        }
        
        // Parse next token
        parseNextToken();
    }
    
    return existed;
}

void ExpressionEvaluator::parseNextToken()
{
    const char *s;
    
    movePastWhiteSpace();
    s = string;
    start_of_current_token = s;
    
    if ( !s || s[0] == '\0' ) {
        // We're all done
        current_token.type = TOKEN_END;
    } else if ( s[0] == '+' || s[0] == '-' ) {
        // Snatch these before the g_strtod() does, othewise they might
        // be used in a numeric conversion.
        acceptTokenCount(1, s[0]);
    } else {
        // Attempt to parse a numeric value
        char *endptr = NULL;
        gdouble value = g_strtod(s, &endptr);
        
        if ( endptr && endptr != s ) {
            // A numeric could be parsed, use it
            current_token.value.fl = value;
            
            current_token.type = TOKEN_NUM;
            string             = endptr;
        } else if (isUnitIdentifierStart(s[0])) {
            // Unit identifier
            current_token.value.c = s;
            current_token.value.size = getIdentifierSize(s, 0);
            
            acceptTokenCount(current_token.value.size, TOKEN_IDENTIFIER);
        } else {
            // Everything else is a single character token
            acceptTokenCount(1, s[0]);
        }
    }
}

void ExpressionEvaluator::acceptTokenCount (int count, TokenType token_type)
{
    current_token.type  = token_type;
    string             += count;
}

void ExpressionEvaluator::isExpected(TokenType token_type,
                                     EvaluatorToken *value)
{
    if (!acceptToken(token_type, value)) {
        throwError("Unexpected token");
    }
}

void ExpressionEvaluator::movePastWhiteSpace()
{
    if (!string) {
        return;
    }
    
    while (g_ascii_isspace(*string)) {
        string++;
    }
}

bool ExpressionEvaluator::isUnitIdentifierStart(gunichar c)
{
    return (g_unichar_isalpha (c)
        || c == (gunichar) '%'
        || c == (gunichar) '\'');
}

/**
 * getIdentifierSize:
 * @s:
 * @start:
 *
 * Returns: Size of identifier in bytes (not including NULL
 * terminator).
 **/
int ExpressionEvaluator::getIdentifierSize(const char *string, int start_offset)
{
    const char *start  = g_utf8_offset_to_pointer(string, start_offset);
    const char *s      = start;
    gunichar    c      = g_utf8_get_char(s);
    int         length = 0;
    
    if (isUnitIdentifierStart(c)) {
        s = g_utf8_next_char (s);
        c = g_utf8_get_char (s);
        length++;
        
        while ( isUnitIdentifierStart (c) || g_unichar_isdigit (c) ) {
            s = g_utf8_next_char(s);
            c = g_utf8_get_char(s);
            length++;
        }
    }
    
    return g_utf8_offset_to_pointer(start, length) - start;
}

bool ExpressionEvaluator::resolveUnit (const char* identifier,
                                       EvaluatorQuantity *result,
                                       Unit const* unit)
{
    if (!unit) {
        result->value = 1;
        result->dimension = 1;
        return true;
    }else if (!identifier) {
        result->value = 1;
        result->dimension = unit->isAbsolute() ? 1 : 0;
        return true;
    } else if (unit_table.hasUnit(identifier)) {
        Unit const *identifier_unit = unit_table.getUnit(identifier);
        result->value = Quantity::convert(1, unit, identifier_unit);
        result->dimension = identifier_unit->isAbsolute() ? 1 : 0;
        return true;
    } else {
        return false;
    }
}

void ExpressionEvaluator::throwError(const char *msg)
{
    throw EvaluatorException(msg, start_of_current_token);
}

} // namespace Util
} // namespace Inkscape
