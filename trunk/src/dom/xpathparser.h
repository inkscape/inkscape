#ifndef __XPATHPARSER_H__
#define __XPATHPARSER_H__

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
 * Copyright (C) 2005-2007 Bob Jamison
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


#include <stdio.h>
#include <stdarg.h>

#include <string>
#include <vector>

#include "dom.h"
#include "xpathtoken.h"

namespace org
{
namespace w3c
{
namespace dom
{
namespace xpath
{

typedef dom::DOMString DOMString;
typedef dom::Node Node;
typedef dom::NodeList  NodeList;



//########################################################################
//# L E X I C A L    D E F I N I T I O N S
//########################################################################


typedef struct
{
   int   ival;
   char const *sval;
} LookupEntry;



//Note:  in the following definitions, where the starts of
//strings are similar, put the longer definitions first

/**
 *
 */
typedef enum
{
    COMMENT,
    TEXT,
    PROCESSING_INSTRUCTION,
    NODE
} NodeType;


static LookupEntry nodeTypeTable [] =
{
    {  COMMENT,                "comment"                },
    {  TEXT,                   "text"                   },
    {  PROCESSING_INSTRUCTION, "processing-instruction" },
    {  NODE,                   "node"                   },
    { -1,                      NULL                     }
};


/**
 *
 */
typedef enum
{
    ANCESTOR_OR_SELF,
    ANCESTOR,
    ATTRIBUTE,
    CHILD,
    DESCENDANT_OR_SELF,
    DESCENDANT,
    FOLLOWING_SIBLING,
    FOLLOWING,
    NAMESPACE,
    PARENT,
    PRECEDING_SIBLING,
    PRECEDING,
    SELF
} AxisNameType;


static LookupEntry axisNameTable [] =
{
    {  ANCESTOR_OR_SELF,    "ancestor-or-self"  },
    {  ANCESTOR,            "ancestor"          },
    {  ATTRIBUTE,           "attribute"         },
    {  CHILD,               "child"             },
    {  DESCENDANT_OR_SELF,  "descendant-or-self"},
    {  DESCENDANT,          "descendant"        },
    {  FOLLOWING_SIBLING,   "following-sibling" },
    {  FOLLOWING,           "following"         },
    {  NAMESPACE,           "namespace"         },
    {  PARENT,              "parent"            },
    {  PRECEDING_SIBLING,   "preceding-sibling" },
    {  PRECEDING,           "preceding"         },
    {  SELF,                "self"              },
    { -1,                   NULL                }
};


/**
 *
 */
typedef enum
{
    NONE = 0,
    CHAR, //default if none of the below
    //Expr tokens
    LPAREN,
    RPAREN,
    LBRACKET,
    RBRACKET,
    DOUBLE_DOT,
    DOT,
    AMPR,
    COMMA,
    DOUBLE_COLON,
    NAME_TEST,
    NODE_TYPE,
    OPERATOR,
    FUNCTION_NAME,
    AXIS_NAME,
    LITERAL,
    NUMBER,
    VARIABLE_REFERENCE,
    //Operator tokens
    AND,
    OR,
    MOD,
    DIV,
    MULTIPLY,
    DOUBLE_SLASH,
    SLASH,
    PIPE,
    PLUS,
    MINUS,
    EQUALS,
    NOT_EQUALS,
    LESS_THAN_EQUALS,
    LESS_THAN,
    GREATER_THAN_EQUALS,
    GREATER_THAN
} LexTokType;


/*
* Be VERY careful that this table matches the LexicalTokenType enum
* declaration above.
*/
static LookupEntry exprTokenTable [] =
{
    {  NONE,                "xxNONExx"          },
    {  CHAR,                "CHAR"              },
    //Expr tokens
    {  LPAREN,              "("                 },
    {  RPAREN,              ")"                 },
    {  LBRACKET,            "["                 },
    {  RBRACKET,            "]"                 },
    {  DOUBLE_DOT,          ".."                },
    {  DOT,                 "."                 },
    {  AMPR,                "@"                 },
    {  COMMA,               ","                 },
    {  DOUBLE_COLON,        "::"                },
    {  NAME_TEST,           "NameTest"          },
    {  NODE_TYPE,           "NodeType"          },
    {  OPERATOR,            "Operator"          },
    {  FUNCTION_NAME,       "FunctionName"      },
    {  AXIS_NAME,           "AxisName"          },
    {  LITERAL,             "Literal"           },
    {  NUMBER,              "Number"            },
    {  VARIABLE_REFERENCE,  "VariableReference" },
    { -1,                   NULL                }
};

static LookupEntry operatorTable [] =
{
    {  NONE,                "xxNONExx"          },
    //Operator tokens
    {  AND,                 "and"               },
    {  OR,                  "or"                },
    {  MOD,                 "mod"               },
    {  DIV,                 "div"               },
    {  MULTIPLY,            "*"                 },
    {  DOUBLE_SLASH,        "//"                },
    {  SLASH,               "/"                 },
    {  PIPE,                "|"                 },
    {  PLUS,                "+"                 },
    {  MINUS,               "-"                 },
    {  EQUALS,              "="                 },
    {  NOT_EQUALS,          "!="                },
    {  LESS_THAN_EQUALS,    "<="                },
    {  LESS_THAN,           "<"                 },
    {  GREATER_THAN_EQUALS, ">="                },
    {  GREATER_THAN,        ">"                 },
    { -1,                   NULL                }
};


/**
 *
 */
class LexTok
{
public:
    LexTok(const LexTok &tok)
        {
        type     = tok.type;
        location = tok.location;
        sval     = tok.sval;
        dval     = tok.dval;
        ival     = tok.ival;
        }
    LexTok()
        { init(); }
    LexTok(int theType, int loc)
        { init(); type = theType; location = loc;}
    LexTok(int theType, int loc, const DOMString &val)
        { init(); type = theType; location = loc; sval = val; }
    LexTok(int theType, int loc, double val)
        { init(); type = theType; location = loc; dval = val; }
    LexTok(int theType, int loc, long val)
        { init(); type = theType; location = loc; ival = val; }

    void print()
        {
        if (type == OPERATOR)
            {
            char const *tokenStr = "unknown";
            for (LookupEntry const *entry = operatorTable; entry->sval ; entry++)
                {
                if (entry->ival == ival)
                    {
                    tokenStr = entry->sval;
                    break;
                    }
                }
            printf("(%s)\n", tokenStr);
            }
        else if (type == NODE_TYPE)
            {
            char const *tokenStr = "unknown";
            for (LookupEntry *entry = nodeTypeTable; entry->sval ; entry++)
                {
                if (entry->ival == ival)
                    {
                    tokenStr = entry->sval;
                    break;
                    }
                }
            printf("{{%s}}\n", tokenStr);
            }
        else if (type == AXIS_NAME)
            {
            char const *tokenStr = "unknown";
            for (LookupEntry *entry = axisNameTable; entry->sval ; entry++)
                {
                if (entry->ival == ival)
                    {
                    tokenStr = entry->sval;
                    break;
                    }
                }
            printf("{%s}\n", tokenStr);
            }
        else if (type == CHAR)
            printf("'%c'\n", (char)ival);
        else if (type == NAME_TEST)
            printf("\"%s\"\n", sval.c_str());
        else if (type == LITERAL)
            printf("L'%s'\n", sval.c_str());
        else if (type == FUNCTION_NAME)
            printf("%s()\n", sval.c_str());
        else if (type == NUMBER)
            printf("#%f\n", dval);
        else
            {
            char const *tokenStr = "unknown";
            for (LookupEntry *entry = exprTokenTable; entry->sval ; entry++)
                {
                if (entry->ival == type)
                    {
                    tokenStr = entry->sval;
                    break;
                    }
                }
            printf("%s\n", tokenStr);
            //printf("%s [%s/%f/%ld]\n", tokenStr, sval.c_str(), dval, ival);
            }
        }

    int getType()
	{ return type; }
    int getLocation()
        { return location; }
    DOMString &getStringValue()
        { return sval; }
    double getDoubleValue()
        { return dval; }
    long getIntValue()
        { return ival; }

private:
    void  init()
        {
        type     = NONE;
        location = 0;
        dval     = 0.0;
        ival     = 0;
        }

    int       type;
    int       location;
    DOMString sval;
    double    dval;
    long      ival;
};





//########################################################################
//# P A R S E R
//########################################################################

class XPathParser
{
public:

    //#################################
    //# CONSTRUCTOR
    //#################################

    /**
     *
     */
    XPathParser()
        {
            debug = false;
        }

    /**
     *
     */
    virtual ~XPathParser() {}

    /**
     *
     */
    bool getDebug()
        { return debug; }

    /**
     *
     */
    void setDebug(bool val)
        { debug = val; }



    /**
     *  Normally not called directly unless for string parsing testing
     */
    bool parse(const DOMString &str);

    /**
     * This is the big one. Called by the xpath-dom api to fetch
     * nodes from a DOM tree.
     */
    NodeList evaluate(const NodePtr root, const DOMString &str);



private:

    //#################################
    //# MESSAGES
    //#################################

    /**
     *
     */
    void trace(const char *fmt, ...)
    #ifdef G_GNUC_PRINTF
    G_GNUC_PRINTF(2, 3)
    #endif
    ;

    /**
     *
     */
    void traceStack(const char *name, int pos, int depth);

    /**
     *
     */
    void error(const char *fmt, ...)
    #ifdef G_GNUC_PRINTF
    G_GNUC_PRINTF(2, 3)
    #endif
    ;

    //#################################
    //# LEXICAL  SCANNING
    //#################################

    /**
     *  Add a lexical token of a given type to the list
     */
    void lexTokAdd(int type, int loc);
    void lexTokAdd(int type, int loc, const DOMString &val);
    void lexTokAdd(int type, int loc, double val);
    void lexTokAdd(int type, int loc, long   val);

    /**
     *
     */
    void lexicalTokenDump();

    /**
     *
     */
    LexTok lexTok(int p);

    /**
     *
     */
    int lexTokType(int p);

    /**
     *
     */
    int peek(int p);

    /**
     *
     */
    int get(int p);

    /**
     *
     */
    int getword(int p, DOMString &str);

    /**
     *
     */
    int match(int p, const char *str);

    /**
     *
     */
    int skipwhite(int p);

    /**
     *
     */
    int getNumber(int p, double &dresult);

    /**
     *
     */
    int getLiteral(int p, DOMString &result);

    /**
     *
     */
    int getNameTest(int p0, DOMString &result);

    /**
     *
     */
    int getNCName(int p0, DOMString &result);




    /**
     *
     */
    int lexicalScan();


    //#################################
    //# GRAMMAR  PARSING
    //#################################

    /**
     * Add a newly derived token to the token list;
     */
    void tokAdd(const Token &token);

    void tokAdd(int type);

    void tokAdd(int type, long val);

    void tokAdd(int type, double val);

    void tokAdd(int type, const DOMString &val);


    /**
     * The grammar definitions marked [1]-[39] are directly
     * from the W3C XPath grammar spacification.
     */

    /**
     * [1]
     */
    int getLocationPath(int p0, int depth);

    /**
     * [2]
     */
    int getAbsoluteLocationPath(int p0, int depth);

    /**
     * [3]
     */
    int getRelativeLocationPath(int p0, int depth);

    /**
     * [4]
     */
    int getStep(int p0, int depth);

    /**
     * [5]
     */
    int getAxisSpecifier(int p0, int depth);

    /**
     * [6]
     */
    int getAxisName(int p0, int depth);

    /**
     * [7]
     */
    int getNodeTest(int p0, int depth);

    /**
     * [8]
     */
    int getPredicate(int p0, int depth);

    /**
     * [9]
     */
    int getPredicateExpr(int p0, int depth);

    /**
     * [10]
     */
    int getAbbreviatedAbsoluteLocationPath(int p0, int depth);
    /**
     * [11]
     */
    int getAbbreviatedRelativeLocationPath(int p0, int depth);
    /**
     * [12]
     */
    int getAbbreviatedStep(int p0, int depth);

    /**
     * [13]
     */
    int getAbbreviatedAxisSpecifier(int p0, int depth);

    /**
     * [14]
     */
    int getExpr(int p0, int depth);

    /**
     * [15]
     */
    int getPrimaryExpr(int p0, int depth);

    /**
     * [16]
     */
    int getFunctionCall(int p0, int depth);

    /**
     * [17]
     */
    int getArgument(int p0, int depth);

    /**
     * [18]
     */
    int getUnionExpr(int p0, int depth);

    /**
     * [19]
     */
    int getPathExpr(int p0, int depth);

    /**
     * [20]
     */
    int getFilterExpr(int p0, int depth);

    /**
     * [21]
     */
    int getOrExpr(int p0, int depth);

    /**
     * [22]
     */
    int getAndExpr(int p0, int depth);

    /**
     * [23]
     */
    int getEqualityExpr(int p0, int depth);

    /**
     * [24]
     */
    int getRelationalExpr(int p0, int depth);

    /**
     * [25]
     */
    int getAdditiveExpr(int p0, int depth);

    /**
     * [26]
     */
    int getMultiplicativeExpr(int p0, int depth);

    /**
     * [27]
     */
    int getUnaryExpr(int p0, int depth);

    /**
     * [28]
     */
    int getExprToken(int p0, int depth);

    /**
     * [29]
     */
    int getLiteral(int p0, int depth);

    /**
     * [30]
     */
    int getNumber(int p0, int depth);

    /**
     * [31]
     */
    int getDigits(int p0, int depth);

    /**
     * [32]
     */
    int getOperator(int p0, int depth);

    /**
     * [33]
     */
    int getOperatorName(int p0, int depth);

    /**
     * [34]
     */
    int getMultiplyOperator(int p0, int depth);

    /**
     * [35]
     */
    int getFunctionName(int p0, int depth);

    /**
     * [36]
     */
    int getVariableReference(int p0, int depth);

    /**
     * [37]
     */
    int getNameTest(int p0, int depth);

    /**
     * [38]
     */
    int getNodeType(int p0, int depth);

    /**
     * [39]
     */
    int getExprWhitespace(int p0, int depth);



    //#################################
    //# DATA ITEMS
    //#################################

    /**
     *
     */
    bool debug;

    /**
     *
     */
    char *parsebuf;

    /**
     *
     */
    int parselen;

    /**
     *
     */
    int position;

    /**
     *
     */
    DOMString numberString;

    /**
     *
     */
    double number;


    /**
     *  The result of the first lexical scan
     */
    std::vector<LexTok> lexicalTokens;

    /**
     *  The result of parsing.  If parsing was successful, then
     *  this is executable via execute()
     */
    TokenList tokens;




};






} // namespace xpath
} // namespace dom
} // namespace w3c
} // namespace org
#endif /* __XPATHPARSER_H__ */
//#########################################################################
//# E N D    O F    F I L E
//#########################################################################








