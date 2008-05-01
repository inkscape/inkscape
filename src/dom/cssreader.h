#ifndef __CSSREADER_H__
#define __CSSREADER_H__
/**
 * Phoebe DOM Implementation.
 *
 * This is a C++ approximation of the W3C DOM model, which follows
 * fairly closely the specifications in the various .idl files, copies of
 * which are provided for reference.  Most important is this one:
 *
 * http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/idl-definitions.html
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2005-2008 Bob Jamison
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include "dom.h"

#include "css.h"

namespace org
{
namespace w3c
{
namespace dom
{
namespace css
{

class CssParser
{

public:

    /**
     *
     */
    CssParser()
        {}

    /**
     *
     */
    virtual ~CssParser()
        {}

    /**
     *
     */
    virtual bool parse(const DOMString &str);

    /**
     *
     */
    virtual bool parseFile(const DOMString &str);


private:

    DOMString parsebuf;
    long parselen;
    CSSStyleSheet stylesheet;


    /**
     *
     */
    void error(char const *fmt, ...)
    #ifdef G_GNUC_PRINTF
    G_GNUC_PRINTF(2, 3)
    #endif
    ;

    /**
     * Get the character at the given location in the buffer.
     *  Return 0 if out-of-bounds
     */
    XMLCh get(int index);


    /**
     *  Test if the given substring exists at the given position
     *  in parsebuf.  Use get() in case of out-of-bounds
     */
    bool match(int pos, const char *str);

    /**
     * Skip over whitespace
     * Return new position
     */
    int skipwhite(int index);

    /**
     * Get the word at the current position.  Return the new
     * position if successful, the current position if no word,
     * -1 if error.
     */
    int getWord(int index, DOMString &str);


    /**
     * Get a number at the current position
     * Return the new position if a proper number, else the original pos
     */
    int getNumber(int index, double &result);

    /**
     * Assume that we are starting on a quote.  Ends on the char
     * after the final '"'
     */
    int getQuoted(int p0, DOMString &result);

/**
 * Not in api.  replaces URI return by lexer
 */
int getUri(int p0, DOMString &str);


/**
 * Skip to the next rule
 */
int skipBlock(int p0);

//#########################################################################
//# P R O D U C T I O N S
//#########################################################################

/**
 * stylesheet
 *   : [ CHARSET_SYM S* STRING S* ';' ]?
 *     [S|CDO|CDC]* [ import [S|CDO|CDC]* ]*
 *     [ [ ruleset | media | page ] [S|CDO|CDC]* ]*
 *   ;
 */
int getStyleSheet(int p0);

/**
 * import
 *   : IMPORT_SYM S*
 *     [STRING|URI] S* [ medium [ COMMA S* medium]* ]? ';' S*
 *   ;
 */
int getImport(int p0);

/**
 * media
 *   : MEDIA_SYM S* medium [ COMMA S* medium ]* LBRACE S* ruleset* '}' S*
 *   ;
 */
int getMedia(int p0);

/**
 * medium
 *   : IDENT S*
 *   ;
 */
int getMedium(int p0);

/**
 * page
 *   : PAGE_SYM S* pseudo_page? S*
 *     LBRACE S* declaration [ ';' S* declaration ]* '}' S*
 *   ;
 */
int getPage(int p0);

/**
 * pseudo_page
 *   : ':' IDENT
 *   ;
 */
int getPseudoPage(int p0);

/**
 * ruleset
 *   : selector [ COMMA S* selector ]*
 *     LBRACE S* declaration [ ';' S* declaration ]* '}' S*
 *   ;
 */
int getRuleSet(int p0);

/**
 * selector
 *   : simple_selector [ combinator simple_selector ]*
 *   ;
 */
int getSelector(int p0);

/**
 * simple_selector
 *   : element_name [ HASH | class | attrib | pseudo ]*
 *   | [ HASH | class | attrib | pseudo ]+
 *   ;
 */
int getSimpleSelector(int p0);

/**
 * declaration
 *   : property ':' S* expr prio?
 *   | {empty}
 *   ;
 */
int getDeclaration(int p0, CSSStyleDeclaration &declarationList);

/**
 * prio
 *   : IMPORTANT_SYM S*
 *   ;
 */
int getPrio(int p0, DOMString &val);

/**
 * expr
 *   : term [ operator term ]*
 *   ;
 */
int getExpr(int p0);

/**
 * term
 *   : unary_operator?
 *     [ NUMBER S* | PERCENTAGE S* | LENGTH S* | EMS S* | EXS S* | ANGLE S* |
 *       TIME S* | FREQ S* | function ]
 *   | STRING S* | IDENT S* | URI S* | hexcolor
 *   ;
 */
int getTerm(int p0);

/**
 * function
 *   : FUNCTION S* expr ')' S*
 *   ;
 */
int getFunction(int p0);

/**
 * There is a constraint on the color that it must
 * have either 3 or 6 hex-digits (i.e., [0-9a-fA-F])
 * after the "#"; e.g., "#000" is OK, but "#abcd" is not.
 *
 * hexcolor
 *   : HASH S*
 *   ;
 */
int getHexColor(int p0);


int lastPosition;

/**
 * Get the column and row number of the given character position.
 * Also gets the last-occuring newline before the position
 */
void getColumnAndRow(int p, int &col, int &row, int &lastNL);

};


} // namespace css
} // namespace dom
} // namespace w3c
} // namespace org







#endif /* __CSSREADER_H__ */
//#########################################################################
//# E N D    O F    F I L E
//#########################################################################

