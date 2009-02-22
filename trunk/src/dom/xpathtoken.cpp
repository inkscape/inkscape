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
 * Copyright (C) 2006-2008 Bob Jamison
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


#include "xpathtoken.h"
#include <stdio.h>



namespace org
{
namespace w3c
{
namespace dom
{
namespace xpath
{



//########################################################################
//# X P A T H    T O K E N
//########################################################################

//########################################################################
//# X P A T H    T O K E N    T Y P E S
//########################################################################



//###########################
//# V A L U E S
//###########################

/**
 * The following functions are called by a token's execute()
 * method.  These functions are what differentiate one token
 * from another and give each its personality.
*/

static bool tokStr(Token &tok, Stack &stack)
{
    StackItem item;
    item.sval = tok.sval;
    stack.push(item);
    return true;
}

static bool tokFloat(Token &tok, Stack &stack)
{
        StackItem item;
        item.dval = tok.dval;
        stack.push(item);
        return true;
}

static bool tokInt(Token &tok, Stack &stack)
{
    StackItem item;
    item.ival = tok.ival;
    stack.push(item);
    return true;
}

static bool tokAnd(Token &/*tok*/, Stack &stack)
{
    StackItem item1 = stack.pop();
    StackItem item2 = stack.pop();
    item1.ival = item1.ival && item2.ival;
    stack.push(item1);
    return true;
}

static bool tokOr(Token &/*tok*/, Stack &stack)
{
    StackItem item1 = stack.pop();
    StackItem item2 = stack.pop();
    item1.ival = item1.ival || item2.ival;
    stack.push(item1);
    return true;
}

static bool tokMod(Token &/*tok*/, Stack &stack)
{
    StackItem item1 = stack.pop();
    StackItem item2 = stack.pop();
    item1.dval = fmod(item1.dval, item2.dval);
    stack.push(item1);
    return true;
}


static bool tokDiv(Token &/*tok*/, Stack &stack)
{
    StackItem item1 = stack.pop();
    StackItem item2 = stack.pop();
    item1.dval /= item2.dval;
    stack.push(item1);
    return true;
}

static bool tokMul(Token &/*tok*/, Stack &stack)
{
    StackItem item1 = stack.pop();
    StackItem item2 = stack.pop();
    item1.dval *= item2.dval;
    stack.push(item1);
    return true;
}

static bool tokPlus(Token &/*tok*/, Stack &stack)
{
    StackItem item1 = stack.pop();
    StackItem item2 = stack.pop();
    item1.dval += item2.dval;
    stack.push(item1);
    return true;
}

static bool tokMinus(Token &/*tok*/, Stack &stack)
{
    StackItem item1 = stack.pop();
    StackItem item2 = stack.pop();
    item1.dval -= item2.dval;
    stack.push(item1);
    return true;
}


static bool tokNeg(Token &/*tok*/, Stack &stack)
{
    StackItem item = stack.pop();
    item.dval = -item.dval;
    item.ival = -item.ival;
    stack.push(item);
    return true;
}


static bool tokEquals(Token &/*tok*/, Stack &stack)
{
    StackItem item1 = stack.pop();
    StackItem item2 = stack.pop();
    item1.ival = (item1.dval == item2.dval);
    stack.push(item1);
    return true;
}


static bool tokNotEquals(Token &/*tok*/, Stack &stack)
{
    StackItem item1 = stack.pop();
    StackItem item2 = stack.pop();
    item1.ival = (item1.dval != item2.dval);
    stack.push(item1);
    return true;
}


static bool tokLessThan(Token &/*tok*/, Stack &stack)
{
    StackItem item1 = stack.pop();
    StackItem item2 = stack.pop();
    item1.ival = (item1.dval < item2.dval);
    stack.push(item1);
    return true;
}


static bool tokLessThanEquals(Token &/*tok*/, Stack &stack)
{
    StackItem item1 = stack.pop();
    StackItem item2 = stack.pop();
    item1.ival = (item1.dval <= item2.dval);
    stack.push(item1);
    return true;
}


static bool tokGreaterThanEquals(Token &/*tok*/, Stack &stack)
{
    StackItem item1 = stack.pop();
    StackItem item2 = stack.pop();
    item1.ival = (item1.dval >= item2.dval);
    stack.push(item1);
    return true;
}


static bool tokGreaterThan(Token &/*tok*/, Stack &stack)
{
    StackItem item1 = stack.pop();
    StackItem item2 = stack.pop();
    item1.ival = (item1.dval > item2.dval);
    stack.push(item1);
    return true;
}






//###########################
//# X P A T H    I T E M S
//###########################

static bool tokAbsolute(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}

static bool tokRelative(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}

static bool tokStep(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}

static bool tokNameTest(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}

static bool tokExpr(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}

static bool tokUnion(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}




//###########################
//# A X I S
//###########################


static bool tokAxisAncestorOrSelf(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokAxisAncestor(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokAxisAttribute(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokAxisChild(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokAxisDescendantOrSelf(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokAxisDescendant(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokAxisFollowingSibling(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokAxisFollowing(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokAxisNamespace(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokAxisParent(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokAxisPrecedingSibling(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokAxisPreceding(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokAxisSelf(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}



//###########################
//# F U N C T I O N S
//###########################


static bool tokFuncLast(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokFuncPosition(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokFuncCount(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokFuncId(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokFuncLocalName(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokFuncNamespaceUri(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokFuncName(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokFuncString(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokFuncConcat(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokFuncStartsWith(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokFuncContains(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokFuncSubstringBefore(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokFuncSubstringAfter(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokFuncSubstring(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokFuncStringLength(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokFuncNormalizeSpace(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokFuncTranslate(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokFuncBoolean(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokFuncNot(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokFuncTrue(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokFuncFalse(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokFuncLang(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokFuncNumber(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokFuncSum(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokFuncFloor(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokFuncCeiling(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}


static bool tokFuncRound(Token &/*tok*/, Stack &/*stack*/)
{
    return true;
}



/**
 * An entry for the token 'personality' table
 */
typedef struct
{
    int ival;
    char const *sval;
    TokenFunc tokenFunc;
} TokenTableEntry;


/**
 * The token personality table
 */
static TokenTableEntry tokenTable[] =
{
    //### primitives
    {
      Token::TOK_NOP,
      "nop",
      NULL
    },
    {
      Token::TOK_STR,
      "str",
      tokStr
    },
    {
      Token::TOK_INT,
      "int",
      tokInt
    },
    {
      Token::TOK_FLOAT,
      "float",
      tokFloat
    },

    //### operators
    {
      Token::TOK_AND,
      "and",
      tokAnd
    },
    {
      Token::TOK_OR,
      "or",
      tokOr
    },
    {
      Token::TOK_MOD,
      "mod",
      tokMod
    },
    {
      Token::TOK_DIV,
      "div",
      tokDiv
    },
    {
      Token::TOK_MUL,
      "multiply",
      tokMul
    },
    {
      Token::TOK_DOUBLE_SLASH,
      "double-slash",
      NULL
    },
    {
      Token::TOK_SLASH,
      "slash",
      NULL
    },
    {
      Token::TOK_PIPE,
      "pipe",
      NULL
    },
    {
      Token::TOK_PLUS,
      "plus",
      tokPlus
    },
    {
      Token::TOK_MINUS,
      "minus",
      tokMinus
    },
    {
      Token::TOK_NEG,
      "neg",
      tokNeg
    },
    {
      Token::TOK_EQUALS,
      "equals",
      tokEquals
    },
    {
      Token::TOK_NOT_EQUALS,
      "not-equals",
      tokNotEquals
    },
    {
      Token::TOK_LESS_THAN_EQUALS,
      "less-than-equals",
      tokLessThanEquals
    },
    {
      Token::TOK_LESS_THAN,
      "less-than",
      tokLessThan
    },
    {
      Token::TOK_GREATER_THAN_EQUALS,
      "greater-than-equals",
      tokGreaterThanEquals
    },
    {
      Token::TOK_GREATER_THAN,
      "greater-than",
      tokGreaterThan
    },

    //### path types
    {
      Token::TOK_ABSOLUTE,
      "absolute",
      tokAbsolute
    },
    {
      Token::TOK_RELATIVE,
      "relative",
      tokRelative
    },
    {
      Token::TOK_STEP,
      "step",
      tokStep
    },
    {
      Token::TOK_NAME_TEST,
      "name-test",
      tokNameTest
    },
    {
      Token::TOK_EXPR,
      "expr",
      tokExpr
    },
    {
      Token::TOK_UNION,
      "union",
      tokUnion
    },

    //### axis types
    {
      Token::TOK_AXIS_ANCESTOR_OR_SELF,
      "axis-ancestor-or-self",
      tokAxisAncestorOrSelf
    },
    {
      Token::TOK_AXIS_ANCESTOR,
      "axis-ancestor",
      tokAxisAncestor
    },
    {
      Token::TOK_AXIS_ATTRIBUTE,
      "axis-attribute",
      tokAxisAttribute
    },
    {
      Token::TOK_AXIS_CHILD,
      "axis-child",
      tokAxisChild
    },
    {
      Token::TOK_AXIS_DESCENDANT_OR_SELF,
      "axis-descendant-or-self",
      tokAxisDescendantOrSelf
    },
    {
      Token::TOK_AXIS_DESCENDANT,
      "axis-descendant",
      tokAxisDescendant
    },
    {
      Token::TOK_AXIS_FOLLOWING_SIBLING,
      "axis-following-sibling",
      tokAxisFollowingSibling
    },
    {
      Token::TOK_AXIS_FOLLOWING,
      "axis-following",
      tokAxisFollowing
    },
    {
      Token::TOK_AXIS_NAMESPACE,
      "axis-namespace",
      tokAxisNamespace
    },
    {
      Token::TOK_AXIS_PARENT,
      "axis-parent",
      tokAxisParent
    },
    {
      Token::TOK_AXIS_PRECEDING_SIBLING,
      "axis-preceding-sibling",
      tokAxisPrecedingSibling
    },
    {
      Token::TOK_AXIS_PRECEDING,
      "axis-preceding",
      tokAxisPreceding
    },
    {
      Token::TOK_AXIS_SELF,
      "axis-self",
      tokAxisSelf
    },

    //### function types
    {
      Token::TOK_FUNC_LAST,
      "func-last",
      tokFuncLast
    },
    {
      Token::TOK_FUNC_POSITION,
      "func-position",
      tokFuncPosition
    },
    {
      Token::TOK_FUNC_COUNT,
      "func-count",
      tokFuncCount
    },
    {
      Token::TOK_FUNC_ID,
      "func-id",
      tokFuncId
    },
    {
      Token::TOK_FUNC_LOCAL_NAME,
      "func-local-name",
      tokFuncLocalName
    },
    {
      Token::TOK_FUNC_NAMESPACE_URI,
      "func-namespace-uri",
      tokFuncNamespaceUri
    },
    {
      Token::TOK_FUNC_NAME,
      "func-name",
      tokFuncName
    },
    {
      Token::TOK_FUNC_STRING,
      "func-string",
      tokFuncString
    },
    {
      Token::TOK_FUNC_CONCAT,
      "func-concat",
      tokFuncConcat
    },
    {
      Token::TOK_FUNC_STARTS_WITH,
      "func-starts-with",
      tokFuncStartsWith
    },
    {
      Token::TOK_FUNC_CONTAINS,
      "func-contains",
      tokFuncContains
    },
    {
      Token::TOK_FUNC_SUBSTRING_BEFORE,
      "func-substring-before",
      tokFuncSubstringBefore
    },
    {
      Token::TOK_FUNC_SUBSTRING_AFTER,
      "func-substring-after",
      tokFuncSubstringAfter
    },
    {
      Token::TOK_FUNC_SUBSTRING,
      "func-substring",
      tokFuncSubstring
    },
    {
      Token::TOK_FUNC_STRING_LENGTH,
      "func-string-length",
      tokFuncStringLength
    },
    {
      Token::TOK_FUNC_NORMALIZE_SPACE,
      "func-normalize-space",
      tokFuncNormalizeSpace
    },
    {
      Token::TOK_FUNC_TRANSLATE,
      "func-translate",
      tokFuncTranslate
    },
    {
      Token::TOK_FUNC_BOOLEAN,
      "func-boolean",
      tokFuncBoolean
    },
    {
      Token::TOK_FUNC_NOT,
      "func-not",
      tokFuncNot
    },
    {
      Token::TOK_FUNC_TRUE,
      "func-true",
      tokFuncTrue
    },
    {
      Token::TOK_FUNC_FALSE,
      "func-false",
      tokFuncFalse
    },
    {
      Token::TOK_FUNC_LANG,
      "func-lang",
      tokFuncLang
    },
    {
      Token::TOK_FUNC_NUMBER,
      "func-number",
      tokFuncNumber
    },
    {
      Token::TOK_FUNC_SUM,
      "func-sum",
      tokFuncSum
    },
    {
      Token::TOK_FUNC_FLOOR,
      "func-floor",
      tokFuncFloor
    },
    {
      Token::TOK_FUNC_CEILING,
      "func-ceiling",
      tokFuncCeiling
    },
    {
      Token::TOK_FUNC_ROUND,
      "func-round",
      tokFuncRound
    },

    { -1,
      (char *)0,
      NULL
    }
};


/**
 *  Return the string TokenType of this token
 *  (in the .cpp file)
 */
DOMString Token::getTypeString()
{
    DOMString ret = "unknown";
    for (TokenTableEntry *entry = tokenTable ; entry->sval ; entry++)
        {
        if (entry->ival == type)
            {
            ret = entry->sval;
            break;
            }
        }
    return ret;
}


/**
 * Create a token of the given type, giving it
 * the data and personalities it needs
 */
Token Token::create(int type, long ival,
           double dval, const DOMString &sval)
{
    Token tok(type, ival, dval, sval);
    for (TokenTableEntry *entry = tokenTable ; entry->sval ; entry++)
        {
        if (entry->ival == type)
            {
            tok.tokenFunc = entry->tokenFunc;
            break;
            }
        }
    
    return tok;
}








//########################################################################
//# X P A T H    E X E C U T O R
//########################################################################

/**
 *
 */
TokenExecutor::TokenExecutor()
{
    reset();
}


/**
 *
 */
TokenExecutor::TokenExecutor(const TokenExecutor &other)
{
    reset();
    assign(other);
}


/**
 *
 */
TokenExecutor::~TokenExecutor()
{
}


/**
 *
 */
void TokenExecutor::assign(const TokenExecutor &other)
{
    tokenList   = other.tokenList;
}


/**
 *
 */
void TokenExecutor::reset()
{
}




/**
 * Execute a list upon a given node.  For each Axis encountered,
 * get the nodes encountered so far, and execute the rest of the
 * list of tokens upon each of the nodes.
 */
int TokenExecutor::execute(std::vector<Token> &tokens,
                            int position,
                            const NodePtr /*node*/,
                            NodeList &result)
{
    Stack stack(*this);

    NodeList contextNodes;

    int length = (int) tokens.size();

    for ( ; position < length ; position ++)
        {
        Token tok = tokens[position];
        if (tok.isAxis())
            {
            int pos2 = 0;
            //Do rest of tokens with the nodes we have found so far
            for (unsigned int i = 0 ; i<contextNodes.getLength() ; i++)
                {
                NodePtr contextNode = contextNodes.item(i);
                pos2 = execute(tokens, position, contextNode, result);
                if (pos2 < 0)
                    {
                    return -1;
                    }
                }
            position = pos2;
            }
        else
            {
            if (!tok.execute(stack))
                {
                return -1;
                } 
            }

        }//for

    return position;
}


/**
 * Execute the tokens in a TokenList upon a given node
 */
bool TokenExecutor::execute(TokenList &tokenList,
                            const NodePtr node,
                            NodeList &result)
{

    NodeList nodelist;

    std::vector<Token> &tokens = tokenList.getTokens();
    if (execute(tokens, 0, node, nodeList) < 0)
        {
        //error message
        return false;
        }

    result = nodeList;

    return true;
}













} // namespace xpath
} // namespace dom
} // namespace w3c
} // namespace org
//########################################################################
//# E N D    O F    F I L E
//########################################################################



