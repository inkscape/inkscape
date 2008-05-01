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

#include "cssreader.h"
#include "ucd.h"

#include <stdio.h>
#include <stdarg.h>

namespace org
{
namespace w3c
{
namespace dom
{
namespace css
{

//#########################################################################
//# M E S S A G E S
//#########################################################################

/**
 * Get the column and row number of the given character position
 */
void CssParser::getColumnAndRow(int p, int &colResult, int &rowResult, int &lastNL)
{
    int col    = 1;
    int row    = 1;
    int lastnl = 0;

    for (int i=0 ; i<p ; i++)
        {
        XMLCh ch = parsebuf[i];
        if (ch == '\n')
            {
            lastnl = i;
            row++;
            col=0;
            }
        else
            col++;
        }

    colResult  = col;
    rowResult  = row;
    lastNL     = lastnl;
}

/**
 *
 */
void CssParser::error(char const *fmt, ...)
{
    int lineNr;
    int colNr;
    int lastNL;
    getColumnAndRow(lastPosition, colNr, lineNr, lastNL);

    va_list args;
    fprintf(stderr, "CssParser:error at %d, line %d, column %d:",
                        lastPosition, lineNr, colNr);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args) ;
    fprintf(stderr, "\n");

    /*
    int lineLen = lastPosition - lastNL;
    printf("lineLen:%d lastNL:%d\n", lineLen, lastNL);
    for (int i=lastNL+1 ; i<lastPosition ; i++)
        fprintf(stderr, "%c", parsebuf[i]);
    fprintf(stderr, "\n");
    for (int i=0 ; i<lineLen-1 ; i++)
        fprintf(stderr, " ");
    fprintf(stderr, "^\n");
    */
    for (int i=0 ; i<lastPosition ; i++)
        fprintf(stderr, "%c", parsebuf[i]);
    fprintf(stderr, "\n");
}



//#########################################################################
//# P A R S I N G
//#########################################################################

/**
 *  Get the character at the position and record the fact
 */
XMLCh CssParser::get(int p)
{
    if (p >= parselen)
        return 0;
    XMLCh ch = parsebuf[p];
    //printf("%c", ch);
    lastPosition = p;
    return ch;
}



/**
 *  Test if the given substring exists at the given position
 *  in parsebuf.  Use get() in case of out-of-bounds
 */
bool CssParser::match(int pos, const char *str)
{
    while (*str)
       {
       if (get(pos++) != (XMLCh) *str++)
           return false;
       }
   return true;
}

/**
 *
 */
int CssParser::skipwhite(int p)
{
  while (p < parselen)
    {
    //# XML COMMENT
    if (match(p, "<!--"))
        {
        p+=4;
        bool done=false;
        while (p<parselen)
            {
            if (match(p, "-->"))
                {
                p+=3;
                done=true;
                break;
                }
            p++;
            }
        lastPosition = p;
        if (!done)
            {
            error("unterminated <!-- .. --> comment");
            return -1;
            }
        }
    //# C comment
    else if (match(p, "/*"))
        {
        p+=2;
        bool done=false;
        while (p<parselen)
            {
            if (match(p, "*/"))
                {
                p+=2;
                done=true;
                break;
                }
            p++;
            }
        lastPosition = p;
        if (!done)
            {
            error("unterminated /* .. */ comment");
            return -1;
            }
        }
    else if (!uni_is_space(get(p)))
        break;
    else
        p++;
    }
  lastPosition = p;
  return p;
}

/**
 * get a word from the buffer
 */
int CssParser::getWord(int p, DOMString &result)
{
    XMLCh ch = get(p);
    if (!uni_is_letter(ch))
        return p;
    DOMString str;
    str.push_back(ch);
    p++;

    while (p < parselen)
        {
        ch = get(p);
        if (uni_is_letter_or_digit(ch) || ch=='-' || ch=='_')
            {
            str.push_back(ch);
            p++;
            }
        else if (ch == '\\')
            {
            p+=2;
            }
        else
            break;
        }
    result = str;
    return p;
}


/**
 * get a word from the buffer
 */
int CssParser::getNumber(int p0, double &result)
{
    int p=p0;
    DOMString str;
    while (p < parselen)
        {
        XMLCh ch = get(p);
        if (ch<'0' || ch>'9')
            break;
        str.push_back(ch);
        p++;
        }
    if (get(p) == '.' && get(p+1)>='0' && get(p+1)<='9')
        {
        p++;
        str.push_back('.');
        while (p < parselen)
            {
            XMLCh ch = get(p);
            if (ch<'0' || ch>'9')
                break;
            str.push_back(ch);
            p++;
            }
        }
    if (p>p0)
        {
        char *start = (char *)str.c_str();
        char *end   = NULL;
        double val = strtod(start, &end);
        if (end > start)
            {
            result = val;
            return p;
            }
        }

    //not a number
    return p0;
}



/**
 * Assume that we are starting on a quote.  Ends on the char
 * after the final '"'
 */
int CssParser::getQuoted(int p0, DOMString &result)
{

    int p = p0;

    XMLCh quoteChar = get(p);
    if (quoteChar != '"' && quoteChar != '\'')
        return p0;

    p++;

    DOMString buf;

    bool done = false;
    while (p<parselen )
        {
        XMLCh ch = get(p);
        if (ch == quoteChar)
            {
            done = true;
            p++;
            break;
            }
        else
            {
            buf.push_back(ch);
            }
        p++;
        }

    if (!done)
        {
        error("unterminated quoted string");
        return -1;
        }

    result.append(buf);

    return p;
}

/**
 * Not in api.  replaces URI return by lexer
 */
int CssParser::getUri(int p0, DOMString &str)
{
    int p = p0;
    if (!match(p, "url("))
        return p0;
    p+=4;
    p = skipwhite(p);
    DOMString buf;
    XMLCh ch;
    while (p < parselen)
        {
        ch = get(p);
        if (isspace(ch) || ch==')')
            break;
        buf.push_back(ch);
        p++;
        }
    p = skipwhite(p);
    ch = get(p);
    if (ch != ')')
        {
        error("no closing ')' on url spec");
        return -1;
        }
    p++;
    str = buf;
    return p;
}

/**
 * Skip to the end of the block
 */
int CssParser::skipBlock(int p0)
{
    int p = p0;
    while (p < parselen)
        {
        XMLCh ch = get(p);
        if (ch == '}')
            {
            p++;
            break;
            }
        else
            {
            p++;
            }
        }
    return p;
}

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
int CssParser::getStyleSheet(int p0)
{
    int p = p0;
    int p2 = p;
    XMLCh ch;

    //# CHARSET   0 or 1
    if (match(p, "@charset"))
        {
        p+=8;
        p = skipwhite(p);
        DOMString str;
        p2 = getQuoted(p, str);
        if (p2<=p)
            {
            error("quoted string required after @charset");
            return -1;
            }
        p = skipwhite(p2);
        ch = get(p);
        if (ch !=';')
            {
            error("';' required after @charset declaration");
            return -1;
            }
        p++;
        p = skipwhite(p);
        }

    //# IMPORT  0 to many
    while (true)
        {
        p2 = getImport(p);
        if (p2<0)
            {
            return -1;
            }
        if (p2<=p)
            break;
        p = p2;
        }

    //# RULESET | MEDIA | PAGE  0 to many
    while (true)
        {
        //Ruleset
        p2 = getRuleSet(p);
        if (p2<0)
            {
            return -1;
            }
        if (p2>p)
            {
            p = p2;
            continue;
            }

        //Media
        p2 = getMedia(p);
        if (p2<0)
            {
            return -1;
            }
        if (p2>p)
            {
            p = p2;
            continue;
            }

        //Page
        p2 = getPage(p);
        if (p2<0)
            {
            return -1;
            }
        if (p2>p)
            {
            p = p2;
            continue;
            }

        //none of the above
        break;
        }

    return p;
}

/**
 * import
 *   : IMPORT_SYM S*
 *     [STRING|URI] S* [ medium [ COMMA S* medium]* ]? ';' S*
 *   ;
 */
int CssParser::getImport(int p0)
{
    int p = p0;
    if (!match(p, "@import"))
        return p0;
    p+=7;
    p = skipwhite(p);

    //# STRING | URI
    DOMString str;
    int p2 = getQuoted(p, str);
    if (p2<0)
        {
        return -1;
        }
    if (p2<=p)
        {
        p2 = getUri(p, str);
        if (p2<0)
            {
            return -1;
            }
        if (p2<=p)
            {
            error("quoted string or URI required after @import");
            return -1;
            }
        }
    p = p2;
    p2 = getMedium(p);
    if (p2<0)
        return -1;

    p = p2;
    p = skipwhite(p);
    XMLCh ch = get(p);
    if (ch != ';')
        {
        error("@import must be terminated with ';'");
        return -1;
        }
    p++;
    return p;
}

/**
 * media
 *   : MEDIA_SYM S* medium [ COMMA S* medium ]* LBRACE S* ruleset* '}' S*
 *   ;
 */
int CssParser::getMedia(int p0)
{
    int p = p0;
    XMLCh ch;
    if (!match(p, "@media"))
        return p0;
    p+=6;
    p = skipwhite(p);

    //# MEDIUM LIST
    int p2 = getMedium(p);
    if (p2<0)
        return -1;
    if (p2<=p)
        {
        error("@media must be followed by medium");
        return -1;
        }
    p = p2;
    while (true)
        {
        ch = get(p);
        if (ch != ',')
            break;
        p2 = getMedium(p);
        if (p2<0)
            return -1;
        if (p2<=p)
            {
            error("',' in medium list must be followed by medium");
            return -1;
            }
        p = p2;
        }

    p = skipwhite(p);
    ch = get(p);
    if (ch!='{')
        {
        error("@media requires '{' for ruleset");
        return -1;
        }
    p++;
    p2 = getRuleSet(p);
    if (p2<0)
        return -1;
    if (p2<=p)
        {
        error("@media requires ruleset after '{'");
        return -1;
        }
    p = p2;
    ch = get(p);
    if (ch != '}')
        {
        error("@media requires '}' after ruleset");
        return -1;
        }
    p++;
    return p0;
}

/**
 * medium
 *   : IDENT S*
 *   ;
 */
int CssParser::getMedium(int p0)
{
    int p = p0;
    p = skipwhite(p);

    DOMString ident;
    int p2 = getWord(p, ident);
    if (p2<0)
        return -1;
    if (p2<=p)
        return p0;
    p = p2;

    return p;
}

/**
 * page
 *   : PAGE_SYM S* pseudo_page? S*
 *     LBRACE S* declaration [ ';' S* declaration ]* '}' S*
 *   ;
 */
int CssParser::getPage(int p0)
{
    int p = p0;

    //# @PAGE
    p = skipwhite(p);
    if (!match(p, "@page"))
        return p0;
    p+= 5;

    //#PSEUDO PAGE 0 or 1
    p = skipwhite(p);
    int p2 = getPseudoPage(p);
    if (p2<0)
        return -1;
    if (p2>p)
        {
        p = p2;
        }

    //# {
    p=skipwhite(p);
    XMLCh ch = get(p);
    if (p != '{')
        {
        error("@page requires '{' before declarations");
        }
    p++;

    //# DECLARATION LIST
    p = skipwhite(p);
    CSSStyleDeclaration declarationList;
    p2 = getDeclaration(p, declarationList);
    if (p2<0)
        return -1;
    if (p2<=p)
        {
        error("@page requires declaration(s) after '{'");
        return -1;
        }
    while (true)
        {
        p = skipwhite(p2);
        ch = get(p);
        if (ch != ';')
            break;
        p++;
        p = skipwhite(p);
        p2 = getDeclaration(p, declarationList);
        if (p2<0)
            return -1;
        if (p2<= p)
            {
            error("@page requires declaration after ';'");
            return -1;
            }
        }

    //# }
    p=skipwhite(p);
    ch = get(p);
    if (p != '}')
        {
        error("@page requires '}' after declarations");
        }
    p++;

    return p;
}

/**
 * pseudo_page
 *   : ':' IDENT
 *   ;
 */
int CssParser::getPseudoPage(int p0)
{
    int p = p0;
    if (!match(p, ":"))
        return p0;
    p++;
    DOMString str;
    int p2 = getWord(p, str);
    if (p2<0)
        return -1;
    if (p2<=p)
        {
        error("pseudo-page requires identifier after ':'");
        return -1;
        }
    p = p2;
    return p;
}

/**
 * ruleset
 *   : selector [ COMMA S* selector ]*
 *     LBRACE S* declaration [ ';' S* declaration ]* '}' S*
 *   ;
 */
int CssParser::getRuleSet(int p0)
{
    int p = p0;
    XMLCh ch;

    //## SELECTOR
    p = skipwhite(p);
    int p2 = getSelector(p);
    if (p2<0)
        return -1;
    if (p2<=p)  //no selector
        {
        if (get(p) != '{')//check for selector-less rule
            return p0;//not me
        }
    p = p2;
    while (true)
        {
        p = skipwhite(p);
        ch = get(p);
        if (ch != ',')
            break;
        p++;
        p = skipwhite(p);
        int p2 = getSelector(p);
        if (p2<0)
            return -1;
        if (p2<=p)
            {
            error("selector required after ',' in list");
            return -1;
            }
        p = p2;
        }

    //## {
    ch = get(p);
    if (ch != '{')
        {
        error("'{' required before declarations of ruleset");
        return -1;
        }
    p++;

    //## DECLARATIONS ( 0 to many )
    CSSStyleDeclaration declarationList;

    p = skipwhite(p);
    p2 = getDeclaration(p, declarationList);
    if (p2<0)
        return -1;
    if (p2>p)
        {
        p = p2;
        while (true)
            {
            p = skipwhite(p);
            ch = get(p);
            if (ch != ';')
                break;
            p++;
            p = skipwhite(p);
            p2 = getDeclaration(p, declarationList);
            if (p2<0)
                return -1;
            if (p2<=p)
                {
                //apparently this is ok
                //error("declaration required after ';' in ruleset");
                //return -1;
                break;
                }
            p = p2;
            }
        }
    //## }
    ch = get(p);
    if (ch != '}')
        {
        error("ruleset requires closing '}'");
        return -1;
        }
    p++;
    p = skipwhite(p);

    return p;
}

/**
 * selector
 *   : simple_selector [ combinator simple_selector ]*
 *   ;
 */
int CssParser::getSelector(int p0)
{
    int p = p0;

    //## SIMPLE SELECTOR
    p = skipwhite(p);
    int p2 = getSimpleSelector(p);
    if (p2<0)
        return -1;
    if (p2<=p)
        return p0; //not me
    p = p2;

    //## COMBINATORS + MORE SELECTORS
    while (true)
        {
        XMLCh ch = get(p);
        bool wasSpace = isspace(ch);
        p = skipwhite(p);
        ch = get(p);
        //# Combinators
        //easier to do here than have a getCombinator()
        int visibleCombinator = false;
        if (ch == '+')
            {
            visibleCombinator = true;
            p++;
            }
        else if (ch == '>')
            {
            visibleCombinator = true;
            p++;
            }
        else if (wasSpace)
            {
            }
        else
            {
            break;
            }
        p = skipwhite(p);
        p2 = getSimpleSelector(p);
        if (p2<0)
            return -1;
        if (p2<=p)
            {
            if (visibleCombinator)
                {
                error("need simple selector after combinator");
                return -1;
                }
            else
                {
                break;
                }
            }
        p = p2;
        }
    return p;
}

/**
 * simple_selector
 *   : element_name [ HASH | class | attrib | pseudo ]*
 *   | [ HASH | class | attrib | pseudo ]+
 *   ;
 */
int CssParser::getSimpleSelector(int p0)
{
    int p = p0;
    int p2;

    DOMString str;

    p = skipwhite(p);

    int selectorItems = 0;

    XMLCh ch = get(p);

    //######################
    //# Note: do NOT skipwhite between items.  Only within the
    //# pseudo function and attrib below
    //######################

    //#Element name 0 or 1
    if (uni_is_letter(ch))
        {
        p2 = getWord(p, str);
        if (p2<0)
            return -1;
        if (p2<=p)
            {
            error("null element name");
            return -1;
            }
        selectorItems++;
        p = p2;
        }
    else if (ch == '*')
        {
        str = "*";
        p++;
        selectorItems++;
        }



    //## HASH, CLASS, ATTRIB, PSEUDO  (0 to many with elem name, 1 to many without)
    while (true)
        {
        XMLCh ch = get(p);

        //# HASH
        if (ch == '#')
            {
            p++;
            p2 = getWord(p, str);
            if (p2<0)
                return -1;
            if (p2<=p)
                {
                error("no name for hash");
                return -1;
                }
            p = p2;
            selectorItems++;
            }

        //# CLASS
        else if (ch == '.')
            {
            p++;
            p2 = getWord(p, str);
            if (p2<0)
                return -1;
            if (p2<=p)
                {
                error("no name for class");
                return -1;
                }
            p = p2;
            selectorItems++;
            }

        //# ATTRIB
        else if (ch == '[')
            {
            p++;
            p = skipwhite(p);
            p2 = getWord(p, str);
            if (p2<0)
                return -1;
            if (p2<=p)
                {
                error("no name for class");
                return -1;
                }
            p = skipwhite(p2);
            bool getRHS=false;
            if (match(p, "="))
                {
                p++;
                getRHS=true;
                }
            else if (match(p, "~="))
                {
                p+=2;
                getRHS=true;
                }
            else if (match(p, "|="))
                {
                p+=2;
                getRHS=true;
                }
            if (getRHS)
                {
                p = skipwhite(p);
                ch = get(p);
                if (uni_is_letter(ch))
                    {
                    p2 = getWord(p, str);
                    if (p2<0)
                        return -1;
                    if (p2<=p)
                        {
                        error("null ident on rhs of attrib");
                        return -1;
                        }
                    p = p2;
                    }
                else if (ch == '\'' || ch =='"')
                    {
                    p2 = getQuoted(p, str);
                    if (p2<0)
                        return -1;
                    if (p2<=p)
                        {
                        error("null literal string on rhs of attrib");
                        return -1;
                        }
                    p = p2;
                    }
                }//getRHS
            p = skipwhite(p);
            ch = get(p);
            if (ch != ']')
                {
                error("attrib needs closing ']'");
                //return -1;
                p = skipBlock(p);
                return p;
                }
            p++;
            selectorItems++;
            }

        //# PSEUDO
        else if (ch == ':')
            {
            p++;
            p2 = getWord(p, str);
            if (p2<0)
                return -1;
            if (p2<=p)
                {
                error("no name for pseudo");
                return -1;
                }
            p = p2;
            selectorItems++;
            ch = get(p);
            if (ch == '(')
                {
                p++;
                p = skipwhite(p);
                ch = get(p);
                if (uni_is_letter(ch))
                    {
                    p2 = getWord(p, str);
                    if (p2<0)
                        return -1;
                    if (p2<=p)
                        {
                        error("null function parameter in pseudo");
                        return -1;
                        }
                    p = skipwhite(p2);
                    ch = get(p);
                    }
                if (ch != ')')
                    {
                    error("function in pseudo needs ')'");
                    return -1;
                    }
                p++;
                }// ch==(   -function-
            }//pseudo

        //# none of the above
        else
            {
            break;
            }

        }//while


    if (selectorItems > 0)
        return p;
    return p0;
}

/**
 * declaration
 *   : property ':' S* expr prio?
 *   | {empty}
 *   ;
 */
int CssParser::getDeclaration(int p0, CSSStyleDeclaration &declarationList)
{
    int p = p0;

    //## PROPERTY
    p = skipwhite(p);
    XMLCh ch = get(p);
    if (!uni_is_letter(ch))
        return p0; //not me
    DOMString propName;
    int p2 = getWord(p, propName);
    if (p2<0)
        return -1;

    //## ':'
    p = skipwhite(p2);
    ch = get(p);
    if (ch != ':')
        {
        error("declaration requires ':' between name and value");
        return -1;
        }
    p++;

    //## EXPR
    p = skipwhite(p);
    p2 = getExpr(p);
    if (p2<0)
        return -1;
    if (p2<=p)
        {
        error("declaration requires value after ':'");
        return -1;
        }
    DOMString propVal;
    for (int i=p ; i<p2 ; i++)  //get our substring
        propVal.push_back(get(i));
    printf("propVal:%s\n", propVal.c_str());
    p = p2;

    //## PRIO (optional)
    p = skipwhite(p);
    DOMString prio;
    p2 = getPrio(p, prio);
    if (p2<0)
        return -1;
    if (p2>p)
        {
        //do something
        p = p2;
        }

    return p;
}

/**
 * prio
 *   : IMPORTANT_SYM S*
 *   ;
 */
int CssParser::getPrio(int p0, DOMString &val)
{
    int p = p0;

    //## '!"
    p = skipwhite(p);
    XMLCh ch = get(p);
    if (ch != '!')
        return p0;
    p++;

    //## "important"
    p = skipwhite(p);
    if (!match(p, "important"))
        {
        error("priority symbol is 'important'");
        return -1;
        }
    p += 9;
    val = "important";
    return p;
}

/**
 * expr
 *   : term [ operator term ]*
 *   ;
 */
int CssParser::getExpr(int p0)
{
    int p = p0;

    //## TERM
    p = skipwhite(p);
    int p2 = getTerm(p);
    if (p2<0)
        return -1;
    if (p2<=p)
        return p0;  //not me
    p = p2;
    while (p < parselen)
        {
        p = skipwhite(p);
        //#Operator.  do this instead of getOperator()
        XMLCh ch = get(p);
        int visibleTerm = false;
        if (ch == '/')
            {
            visibleTerm = true;
            p++;
            }
        else if (ch == ',')
            {
            visibleTerm = true;
            p++;
            }
        else
            {
            //just space.  this is allowable between terms,
            // so we still need to check for another term
            }
        p = skipwhite(p);
        p2 = getTerm(p);
        if (p2<0)
            return -1;
        if (p2<=p)
            {
            if (visibleTerm)
                {
                error("expression requires term after operator");
                return -1;
                }
            else
                {
                break;
                }
            }
        p = p2;
        }

    return p;
}

/**
 * term
 *   : unary_operator?
 *     [ NUMBER S* | PERCENTAGE S* | LENGTH S* | EMS S* | EXS S* | ANGLE S* |
 *       TIME S* | FREQ S* | function ]
 *   | STRING S* | IDENT S* | URI S* | hexcolor
 *   ;
 */
int CssParser::getTerm(int p0)
{
    int p = p0;
    p = skipwhite(p);
    int unitType = CSSPrimitiveValue::CSS_UNKNOWN;
    //# Unary operator
    XMLCh ch = get(p);
    bool hasUnary = false;
    if (ch == '-')
        {
        p++;
        hasUnary = true;
        }
    else if (ch == '+')
        {
        p++;
        hasUnary = true;
        }
    //# NUMERIC
    double numVal;
    int p2 = getNumber(p, numVal);
    if (p2<0)
        return -1;
    if (p2>p)
        {
        p = p2;
        if (match(p, "%"))
            {
            unitType = CSSPrimitiveValue::CSS_PERCENTAGE;
            p++;
            }
        else if (match(p, "em"))
            {
            unitType = CSSPrimitiveValue::CSS_EMS;
            p+=2;
            }
        else if (match(p, "ex"))
            {
            unitType = CSSPrimitiveValue::CSS_EXS;
            p+=2;
            }
        else if (match(p, "px"))
            {
            unitType = CSSPrimitiveValue::CSS_PX;
            p+=2;
            }
        else if (match(p, "cm"))
            {
            unitType = CSSPrimitiveValue::CSS_CM;
            p+=2;
            }
        else if (match(p, "mm"))
            {
            unitType = CSSPrimitiveValue::CSS_MM;
            p+=2;
            }
        else if (match(p, "in"))
            {
            unitType = CSSPrimitiveValue::CSS_IN;
            p+=2;
            }
        else if (match(p, "pt"))
            {
            unitType = CSSPrimitiveValue::CSS_PT;
            p+=2;
            }
        else if (match(p, "pc"))
            {
            unitType = CSSPrimitiveValue::CSS_PC;
            p+=2;
            }
        else if (match(p, "deg"))
            {
            unitType = CSSPrimitiveValue::CSS_DEG;
            p+=3;
            }
        else if (match(p, "rad"))
            {
            unitType = CSSPrimitiveValue::CSS_RAD;
            p+=3;
            }
        else if (match(p, "grad"))
            {
            unitType = CSSPrimitiveValue::CSS_GRAD;
            p+=4;
            }
        else if (match(p, "ms"))
            {
            unitType = CSSPrimitiveValue::CSS_MS;
            p+=2;
            }
        else if (match(p, "s"))
            {
            unitType = CSSPrimitiveValue::CSS_S;
            p+=1;
            }
        else if (match(p, "Hz"))
            {
            unitType = CSSPrimitiveValue::CSS_HZ;
            p+=2;
            }
        else if (match(p, "kHz"))
            {
            unitType = CSSPrimitiveValue::CSS_KHZ;
            p+=2;
            }
        else if (uni_is_letter(get(p)))//some other string
            {
            DOMString suffix;
            p2 = getWord(p, suffix);
            if (p2<0)
                return -1;
            unitType = CSSPrimitiveValue::CSS_DIMENSION;
            p = p2;
            }
        else //plain number
            {
            unitType = CSSPrimitiveValue::CSS_NUMBER;
            }
        return p;
        }

    DOMString str;

    //## URI --do before function, as syntax is similar
    p2 = getUri(p, str);
    if (p2<0)
        return -1;
    if (p2>p)
        {
        if (hasUnary)
            {
            error("+ or - not allowed on URI");
            return -1;
            }
        p = p2;
        unitType = CSSPrimitiveValue::CSS_URI;
        return p;
        }

    //## FUNCTION
    p2 = getFunction(p);
    if (p2<0)
        return -1;
    if (p2>p)
        {
        p = p2;
        return p;
        }

    //## STRING
    ch = get(p);
    if (ch == '"' || ch == '\'')
        {
        p2 = getQuoted(p, str);
        if (p2<0)
            return -1;
        if (p2>p)
            {
            if (hasUnary)
                {
                error("+ or - not allowed on a string");
                return -1;
                }
            p = p2;
            unitType = CSSPrimitiveValue::CSS_STRING;
            return p;
            }
        }

    //## IDENT
    ch = get(p);
    if (uni_is_letter(ch))
        {
        p2 = getWord(p, str);
        if (p2<0)
            return -1;
        if (p2>p)
            {
            if (hasUnary)
                {
                error("+ or - not allowed on an identifier");
                return -1;
                }
            p = p2;
            unitType = CSSPrimitiveValue::CSS_IDENT;
            return p;
            }
        }


    //## HEXCOLOR
    p2 = getHexColor(p);
    if (p2<0)
        return -1;
    if (p2>p)
        {
        if (hasUnary)
            {
            error("+ or - not allowed on hex color");
            return -1;
            }
        p = p2;
        unitType = CSSPrimitiveValue::CSS_RGBCOLOR;
        return p;
        }


    return p0;
}

/**
 * function
 *   : FUNCTION S* expr ')' S*
 *   ;
 */
int CssParser::getFunction(int p0)
{
    int p = p0;

    //## IDENT + (   (both)
    DOMString name;
    p = skipwhite(p);
    int p2 = getWord(p, name);
    if (p2<0)
        return -1;
    if (p2<=p)
        return p0; //not me
    if (name == "uri" || name=="url")
        return p0; //not me
    p = skipwhite(p2);
    XMLCh ch = get(p);
    if (ch != '(')
        return p0; //still not me
    p++;

    //## EXPR
    p = skipwhite(p);
    p2 = getExpr(p);
    if (p2<0)
        return -1;
    if (p2<=p)
        {
        error("function requires expression");
        return -1;
        }
    p = p2;

    //## ')'
    p = skipwhite(p);
    ch = get(p);
    if (ch != ')')
        {
        error("function requires closing ')'");
        return -1;
        }
    p++;
    p = skipwhite(p);

    return p;
}

/**
 * There is a constraint on the color that it must
 * have either 3 or 6 hex-digits (i.e., [0-9a-fA-F])
 * after the "#"; e.g., "#000" is OK, but "#abcd" is not.
 * hexcolor
 *   : HASH S*
 *   ;
 */
int CssParser::getHexColor(int p0)
{
    int p = p0;

    //## '#'
    p = skipwhite(p);
    if (!match(p, "#"))
        return p0;
    p++;

    //## HEX
    DOMString hex;
    long hexVal = 0;
    while (p < parselen)
        {
        XMLCh b = get(p);
        if (b>='0' && b<='9')
            {
            hexVal = (hexVal << 4) + (b - '0');
            hex.push_back(b);
            p++;
            }
        else if (b>='a' && b<='f')
            {
            hexVal = (hexVal << 4) + (b - 'a' + 10);
            hex.push_back(b);
            p++;
            }
        else if (b>='A' && b<='F')
            {
            hexVal = (hexVal << 4) + (b - 'A' + 10);
            hex.push_back(b);
            p++;
            }
        else
            {
            break;
            }
        }

    if (hex.size() != 3 && hex.size() != 6)
        {
        error("exactly 3 or 6 hex digits are required after '#'");
        return -1;
        }

    return p;
}



/**
 *
 */
bool CssParser::parse(const DOMString &str)
{
    /*
    int len = str.size();
    for (int i=0 ; i<len ; i++)
        {
        XMLCh ch = str[i];
        if (ch == '\\' && i<(len-1)) //escape!
            {
            i++;
            }
        else
            parsebuf.push_back(ch);
        }
    */
    parsebuf = str;

    parselen = parsebuf.size();
    //printf("==============================\n%s\n========================\n", str.c_str());

    lastPosition = 0;

    int p = getStyleSheet(0);
    if (p < parselen)
        {
        error("Not everything parsed");
        return false;
        }

    return true;
}


/**
 *
 */
bool CssParser::parseFile(const DOMString &fileName)
{
    DOMString tmp = fileName;
    char *fname = (char *)tmp.c_str();
    FILE *f = fopen(fname, "r");
    if (!f)
        {
        printf("Could not open %s for reading\n", fname);
        return false;
        }

    DOMString str;
    while (!feof(f))
        {
        int ch = fgetc(f);
        if (ch<0)
            break;
        str.push_back((XMLCh)ch);
        }
    fclose(f);

    bool ret = parse(str);

    return ret;
}







} // namespace css
} // namespace dom
} // namespace w3c
} // namespace org


#ifdef TEST

int main(int argc, char **argv)
{
    org::w3c::dom::css::CssParser parser;
    char *fileName;
    fileName = "001.css";
    //fileName = "acid.css";
    //fileName = "base.css";
    //fileName = "inkscape.css";
    //fileName = "meyerweb.css";
    if (!parser.parseFile(fileName))
        {
        printf("Test failed\n");
        return 1;
        }
    return 0;
}

#endif /* TEST */

//#########################################################################
//# E N D    O F    F I L E
//#########################################################################

