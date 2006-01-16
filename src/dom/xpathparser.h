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
 * Copyright (C) 2005 Bob Jamison
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
   char *sval;
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
    LexTok(int theType, int loc)
        { init(); type = theType; location = loc;}
    LexTok()
        { init(); }
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
            char *tokenStr = "unknown";
            for (LookupEntry *entry = operatorTable; entry->sval ; entry++)
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
            char *tokenStr = "unknown";
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
            char *tokenStr = "unknown";
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
            char *tokenStr = "unknown";
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
        sval     = "";
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
//# G R A M M A T I C A L    T O K E N S
//########################################################################

typedef enum
{
    TOK_NONE,
    TOK_ABSOLUTE,
    TOK_RELATIVE,
    TOK_STEP,
    TOK_EXPR
} TokenTypes;


/**
 *
 */
class Token
{
public:
    Token()
        { init(); }

    Token(const Token &other)
        {
		init();
		type = other.type;
	    }

    Token(int theType)
        {
		init();
		type = theType;
	    }

    ~Token() {}


private:

    void init()
        {
		type = TOK_NONE;
	    }

    int type;


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
    virtual bool getDebug()
        { return debug; }

    /**
     *
     */
    virtual void setDebug(bool val)
        { debug = val; }



    /**
     *  Normally not called directly unless for string parsing testing
     */
    virtual bool parse(const DOMString &str);

    /**
     * Normally not called directly except for testing.
     */
    virtual NodeList execute(const Node *root, std::vector<Token> &toks);

    /**
     * This is the big one. Called by the xpath-dom api to fetch
     * nodes from a DOM tree.
     */
    virtual NodeList evaluate(const Node *root, const DOMString &str);



private:

    //#################################
    //# MESSAGES
    //#################################

    /**
     *
     */
    virtual void trace(const char *fmt, ...);

    /**
     *
     */
    virtual void traceStack(const char *name, int pos, int depth);

    /**
     *
     */
    virtual void error(const char *fmt, ...);

    //#################################
    //# LEXICAL  SCANNING
    //#################################

    /**
     *  Add a lexical token of a given type to the list
     */
    virtual void lexTokAdd(int type, int loc);
    virtual void lexTokAdd(int type, int loc, const DOMString &val);
    virtual void lexTokAdd(int type, int loc, double val);
    virtual void lexTokAdd(int type, int loc, long   val);

    /**
     *
     */
    virtual void lexicalTokenDump();

    /**
     *
     */
    virtual LexTok lexTok(int p);

    /**
     *
     */
    virtual int lexTokType(int p);

    /**
     *
     */
    virtual int peek(int p);

    /**
     *
     */
    virtual int get(int p);

    /**
     *
     */
    virtual int getword(int p, DOMString &str);

    /**
     *
     */
    virtual int match(int p, const char *str);

    /**
     *
     */
    virtual int skipwhite(int p);

    /**
     *
     */
    virtual int getNumber(int p, double &dresult);

    /**
     *
     */
    virtual int getLiteral(int p, DOMString &result);

    /**
     *
     */
    virtual int getNameTest(int p0, DOMString &result);

    /**
     *
     */
    virtual int getNCName(int p0, DOMString &result);




    /**
     *
     */
    virtual int lexicalScan();


    //#################################
    //# GRAMMAR  PARSING
    //#################################

    /**
     * The grammar definitions marked [1]-[39] are directly
     * from the W3C XPath grammar spacification.
     */

    /**
     * [1]
     */
    virtual int getLocationPath(int p0, int depth);

    /**
     * [2]
     */
    virtual int getAbsoluteLocationPath(int p0, int depth);

    /**
     * [3]
     */
    virtual int getRelativeLocationPath(int p0, int depth);

    /**
     * [4]
     */
    virtual int getStep(int p0, int depth);

    /**
     * [5]
     */
    virtual int getAxisSpecifier(int p0, int depth);

    /**
     * [6]
     */
    virtual int getAxisName(int p0, int depth);

    /**
     * [7]
     */
    virtual int getNodeTest(int p0, int depth);

    /**
     * [8]
     */
    virtual int getPredicate(int p0, int depth);

    /**
     * [9]
     */
    virtual int getPredicateExpr(int p0, int depth);

    /**
     * [10]
     */
    virtual int getAbbreviatedAbsoluteLocationPath(int p0, int depth);
    /**
     * [11]
     */
    virtual int getAbbreviatedRelativeLocationPath(int p0, int depth);
    /**
     * [12]
     */
    virtual int getAbbreviatedStep(int p0, int depth);

    /**
     * [13]
     */
    virtual int getAbbreviatedAxisSpecifier(int p0, int depth);

    /**
     * [14]
     */
    virtual int getExpr(int p0, int depth);

    /**
     * [15]
     */
    virtual int getPrimaryExpr(int p0, int depth);

    /**
     * [16]
     */
    virtual int getFunctionCall(int p0, int depth);

    /**
     * [17]
     */
    virtual int getArgument(int p0, int depth);

    /**
     * [18]
     */
    virtual int getUnionExpr(int p0, int depth);

    /**
     * [19]
     */
    virtual int getPathExpr(int p0, int depth);

    /**
     * [20]
     */
    virtual int getFilterExpr(int p0, int depth);

    /**
     * [21]
     */
    virtual int getOrExpr(int p0, int depth);

    /**
     * [22]
     */
    virtual int getAndExpr(int p0, int depth);

    /**
     * [23]
     */
    virtual int getEqualityExpr(int p0, int depth);

    /**
     * [24]
     */
    virtual int getRelationalExpr(int p0, int depth);

    /**
     * [25]
     */
    virtual int getAdditiveExpr(int p0, int depth);

    /**
     * [26]
     */
    virtual int getMultiplicativeExpr(int p0, int depth);

    /**
     * [27]
     */
    virtual int getUnaryExpr(int p0, int depth);

    /**
     * [28]
     */
    virtual int getExprToken(int p0, int depth);

    /**
     * [29]
     */
    virtual int getLiteral(int p0, int depth);

    /**
     * [30]
     */
    virtual int getNumber(int p0, int depth);

    /**
     * [31]
     */
    virtual int getDigits(int p0, int depth);

    /**
     * [32]
     */
    virtual int getOperator(int p0, int depth);

    /**
     * [33]
     */
    virtual int getOperatorName(int p0, int depth);

    /**
     * [34]
     */
    virtual int getMultiplyOperator(int p0, int depth);

    /**
     * [35]
     */
    virtual int getFunctionName(int p0, int depth);

    /**
     * [36]
     */
    virtual int getVariableReference(int p0, int depth);

    /**
     * [37]
     */
    virtual int getNameTest(int p0, int depth);

    /**
     * [38]
     */
    virtual int getNodeType(int p0, int depth);

    /**
     * [39]
     */
    virtual int getExprWhitespace(int p0, int depth);



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
    std::vector<Token> tokens;




};






} // namespace xpath
} // namespace dom
} // namespace w3c
} // namespace org
#endif /* __XPATHPARSER_H__ */
//#########################################################################
//# E N D    O F    F I L E
//#########################################################################








