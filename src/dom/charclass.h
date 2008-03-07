#ifndef __CHARCLASS_H__
#define __CHARCLASS_H__
/**
 *
 * Phoebe DOM Implementation.
 *
 * This is a C++ approximation of the W3C DOM model, which follows
 * fairly closely the specifications in the various .idl files, copies of
 * which are provided for reference.  Most important is this one:
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2008 Bob Jamison
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


/**
 * Enumerated Unicode general category types
 */
typedef enum UniCharType
{
    UNI_UNASSIGNED                =  0,  /* Cn */
    UNI_UPPERCASE_LETTER          =  1,  /* Lu */
    UNI_LOWERCASE_LETTER          =  2,  /* Ll */
    UNI_TITLECASE_LETTER          =  3,  /* Lt */
    UNI_MODIFIER_LETTER           =  4,  /* Lm */
    UNI_OTHER_LETTER              =  5,  /* Lo */
    UNI_NON_SPACING_MARK          =  6,  /* Mn */
    UNI_ENCLOSING_MARK            =  7,  /* Me */
    UNI_COMBINING_SPACING_MARK    =  8,  /* Mc */
    UNI_DECIMAL_DIGIT_NUMBER      =  9,  /* Nd */
    UNI_LETTER_NUMBER             = 10,  /* Nl */
    UNI_OTHER_NUMBER              = 11,  /* No */
    UNI_SPACE_SEPARATOR           = 12,  /* Zs */
    UNI_LINE_SEPARATOR            = 13,  /* Zl */
    UNI_PARAGRAPH_SEPARATOR       = 14,  /* Zp */
    UNI_CONTROL                   = 15,  /* Cc */
    UNI_FORMAT                    = 16,  /* Cf */
    UNI_UNUSED_RESERVE            = 17,  /* xx */
    UNI_PRIVATE_USE               = 18,  /* Co */
    UNI_SURROGATE                 = 19,  /* Cs */
    UNI_DASH_PUNCTUATION          = 20,  /* Pd */
    UNI_START_PUNCTUATION         = 21,  /* Ps */
    UNI_END_PUNCTUATION           = 22,  /* Pe */
    UNI_CONNECTOR_PUNCTUATION     = 23,  /* Pc */
    UNI_OTHER_PUNCTUATION         = 24,  /* Po */
    UNI_MATH_SYMBOL               = 25,  /* Sm */
    UNI_CURRENCY_SYMBOL           = 26,  /* Sc */
    UNI_MODIFIER_SYMBOL           = 27,  /* Sk */
    UNI_OTHER_SYMBOL              = 28,  /* So */
    UNI_INITIAL_QUOTE_PUNCTUATION = 29,  /* Pi */
    UNI_FINAL_QUOTE_PUNCTUATION   = 30   /* Pf */
} UniCharType;


/**
 * Get the raw table entry for this Unicode codepoint
 * @param ch the Unicode codepoint to test
 * @return the raw UCD property table entry 
 */
unsigned int uni_code(int ch);


/**
 * Get the Unicode General Category of ths character
 * @param ch the Unicode codepoint to test
 * @return the 'UniCharType' General Category enumeration (above)
 */
unsigned int uni_type(int ch);


/**
 * Test if this Unicode code point is lower case
 * @param ch the Unicode codepoint to test
 * @return 1 if successful, else 0
 */
int uni_is_lower(int ch);


/**
 * Test if this Unicode code point is upper case
 * @param ch the Unicode codepoint to test
 * @return 1 if successful, else 0
 */
int uni_is_upper(int ch);


/**
 * Test if this Unicode code point is title case
 * @param ch the Unicode codepoint to test
 * @return 1 if successful, else 0
 */
int uni_is_title(int ch);


/**
 * Test if this Unicode code point is a numeric digit
 * @param ch the Unicode codepoint to test
 * @return 1 if successful, else 0
 */
int uni_is_digit(int ch);


/**
 * Test if this Unicode code point is defined in the database
 * @param ch the Unicode codepoint to test
 * @return 1 if successful, else 0
 */
int uni_is_defined(int ch);

/**
 * Test if this Unicode code point is a letter
 * @param ch the Unicode codepoint to test
 * @return 1 if successful, else 0
 */
int uni_is_letter(int ch);


/**
 * Test if this Unicode code point is a letter or a digit
 * @param ch the Unicode codepoint to test
 * @return 1 if successful, else 0
 */
int uni_is_letter_or_digit(int ch);

/**
 * Test if this Unicode code point is considered to be a space
 * @param ch the Unicode codepoint to test
 * @return 1 if successful, else 0
 */
int uni_is_space(int ch);



#endif /* __CHARCLASS_H__ */

           
