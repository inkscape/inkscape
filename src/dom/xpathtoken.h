#ifndef __XPATHTOKEN_H__
#define __XPATHTOKEN_H__

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
 * Copyright (C) 2006 Bob Jamison
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

#include <math.h>

#include <vector>

namespace org
{
namespace w3c
{
namespace dom
{
namespace xpath
{

typedef org::w3c::dom::DOMString DOMString;

class Axis
{
public:
    /**
     *  Constructor
     */
    Axis();

    /**
     *  Constructor
     */
    Axis(int tokPos);

    /**
     *  Copy constructor
     */
    Axis(const Axis &other);

    /**
     *  Destructor
     */
    virtual ~Axis();

    /**
     *
     */
    Axis &operator=(const Axis &other);

    /**
     *
     */
    void init();

    /**
     *
     */
    void assign(const Axis &other);

    /**
     *
     */
    void setPosition(unsigned int val);

    /**
     *
     */
    unsigned int getPosition();

    /**
     *
     */
    void setNode(const Node *node);

    /**
     *
     */
    Node *getNode();

private:

    int tokenPosition;

    Node *node;
};


/**
 * This represents a single value on the evaluation stack
 */
class StackItem
{
public:

    /**
     *  Constructor
     */
    StackItem();

    /**
     *  Copy constructor
     */
    StackItem(const StackItem &other);

    /**
     *  Destructor
     */
    virtual ~StackItem();

    /**
     *
     */
    StackItem &operator=(const StackItem &other);

    /**
     *
     */
    void assign(const StackItem &other);


    //treat the stack item like an union of string, integer, and double

    /**
     * String value
     */
    DOMString sval;

    /**
     * Integer value
     */
    long      ival;

    /**
     * Double value;
     */
    double    dval;

};

class TokenList;

//########################################################################
//# T O K E N    E X E C U T O R
//########################################################################

#define STACK_SIZE 1024

/**
 * A token evaluator, with stack and axis context
 */
class TokenExecutor
{
public:

    /**
     * Constructor
     */
    TokenExecutor();

    /**
     * Copy constructor
     */
    TokenExecutor(const TokenExecutor &other);

    /**
     * Destructor
     */
    virtual ~TokenExecutor();

    /**
     *  Assign our values to those of the other
     */
    virtual void assign(const TokenExecutor &other);

    /**
     * Reset the stack to its original settings
     */
    virtual void reset();

    /**
     * Push a stack item onto the stack
     */
    virtual void push(StackItem &item);

    /**
     * Pop a stack item from the stack
     */
    virtual StackItem pop();

    /**
     * Execute a token list on the stack
     */
    NodeList execute(const TokenList &list, const Node *node);

    /**
     *
     */
    Axis axis;

    /**
     *
     */
    std::vector<Axis> axisStack;

private:

    /**
     * Contains the StackItem stack;
     */
    StackItem stack[STACK_SIZE];

    /**
     * Marks the head of the stack, for push() and pop()
     */
    int stackSize;

    /**
     *  Current list of nodes found by the expression
     */
    NodeList nodeList;


};



//########################################################################
//# X P A T H    T O K E N
//########################################################################



/**
 *  This is a pseudocode-type class that executes itself on a stack,
 *  much like stack-oriented languages such as FORTH or Postscript.
 *  Each token can pop zero or more tokens off the stack, and push
 *  zero or one token back onto it.  When a list of tokens is completed,
 *  a single stack value should be left on the stack.
 */
class Token
{
public:

    /**
     * Token types.  Look in xpathtoken.cpp's function table
     * to see how these types map to their respective
     * functionalities
     */
    typedef enum
        {
        //primitives
        TOK_NOP = 0,
        TOK_STR,
        TOK_INT,
        TOK_FLOAT,
        //operators
        TOK_AND,
        TOK_OR,
        TOK_MOD,
        TOK_DIV,
        TOK_MULTIPLY,
        TOK_DOUBLE_SLASH,
        TOK_SLASH,
        TOK_PIPE,
        TOK_PLUS,
        TOK_MINUS,
        TOK_NEG,
        TOK_EQUALS,
        TOK_NOT_EQUALS,
        TOK_LESS_THAN_EQUALS,
        TOK_LESS_THAN,
        TOK_GREATER_THAN_EQUALS,
        TOK_GREATER_THAN,
        //path types
        TOK_ABSOLUTE,
        TOK_RELATIVE,
        TOK_STEP,
        TOK_NAME_TEST,
        TOK_EXPR,
        TOK_UNION,
        //axis types
        TOK_AXIS_ANCESTOR_OR_SELF,
        TOK_AXIS_ANCESTOR,
        TOK_AXIS_ATTRIBUTE,
        TOK_AXIS_CHILD,
        TOK_AXIS_DESCENDANT_OR_SELF,
        TOK_AXIS_DESCENDANT,
        TOK_AXIS_FOLLOWING_SIBLING,
        TOK_AXIS_FOLLOWING,
        TOK_AXIS_NAMESPACE,
        TOK_AXIS_PARENT,
        TOK_AXIS_PRECEDING_SIBLING,
        TOK_AXIS_PRECEDING,
        TOK_AXIS_SELF,
        //function types
        TOK_FUNC_LAST,
        TOK_FUNC_POSITION,
        TOK_FUNC_COUNT,
        TOK_FUNC_ID,
        TOK_FUNC_LOCAL_NAME,
        TOK_FUNC_NAMESPACE_URI,
        TOK_FUNC_NAME,
        TOK_FUNC_STRING,
        TOK_FUNC_CONCAT,
        TOK_FUNC_STARTS_WITH,
        TOK_FUNC_CONTAINS,
        TOK_FUNC_SUBSTRING_BEFORE,
        TOK_FUNC_SUBSTRING_AFTER,
        TOK_FUNC_SUBSTRING,
        TOK_FUNC_STRING_LENGTH,
        TOK_FUNC_NORMALIZE_SPACE,
        TOK_FUNC_TRANSLATE,
        TOK_FUNC_BOOLEAN,
        TOK_FUNC_NOT,
        TOK_FUNC_TRUE,
        TOK_FUNC_FALSE,
        TOK_FUNC_LANG,
        TOK_FUNC_NUMBER,
        TOK_FUNC_SUM,
        TOK_FUNC_FLOOR,
        TOK_FUNC_CEILING,
        TOK_FUNC_ROUND,
        } TokenType;




    /**
     *  Constructor with a NOP default type
     */
    Token()
        {
        type     = TOK_NOP;
        ival     = 0L;
        dval     = 0.0;
        }

    /**
     * Copy constructor
     */
    Token(const Token &other)
        {
        type     = other.type;
        sval     = other.sval;
        ival     = other.ival;
        dval     = other.dval;
        }

    /**
     * Destructor
     */
    virtual ~Token()
        {}

    /**
     *  Return the enumerated TokenType of this token
     */
    virtual int getType()
        { return type; }
    /**
     *  Return the string TokenType of this token
     */
    virtual DOMString getTypeString();

    /**
     *  Let this token execute itself on the given stack,
     *  possibly adding Nodes to the node list.
     */
    virtual bool execute(TokenExecutor &stack)
        { return true; }

    /**
     *  Print the contents of this token
     */
    virtual void dump()
        {
        printf("%s %s %f %ld\n",
            getTypeString().c_str(), sval.c_str(), dval, ival);
        }

    //treat the token like an union of string, integer, and double

    /**
     * String value
     */
    DOMString sval;

    /**
     * Integer value
     */
    long ival;

    /**
     * Double value;
     */
    double dval;

protected:

    /**
     * The enmerated token type
     */
    int type;


private:


};


//########################################################################
//# X P A T H    T O K E N    T Y P E S
//########################################################################



//###########################
//# V A L U E S
//###########################

class TokStr : public Token
{
public:
    TokStr(const DOMString &val)
        {
        type = TOK_STR;
        sval = val;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        StackItem item;
        item.sval = sval;
        exec.push(item);
        return true;
        }
};

class TokFloat : public Token
{
public:
    TokFloat(double val)
        {
        type = TOK_FLOAT;
        dval = val;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        StackItem item;
        item.dval = dval;
        exec.push(item);
        return true;
        }
};

class TokInt : public Token
{
public:
    TokInt(long val)
        {
        type = TOK_INT;
        ival = val;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        StackItem item;
        item.ival = ival;
        exec.push(item);
        return true;
        }
};

class TokAnd : public Token
{
public:
    TokAnd()
        {
        type = TOK_AND;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        StackItem item1 = exec.pop();
        StackItem item2 = exec.pop();
        item1.ival = item1.ival && item2.ival;
        exec.push(item1);
        return true;
        }
};

class TokOr : public Token
{
public:
    TokOr()
        {
        type = TOK_OR;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        StackItem item1 = exec.pop();
        StackItem item2 = exec.pop();
        item1.ival = item1.ival || item2.ival;
        exec.push(item1);
        return true;
        }
};

class TokMod : public Token
{
public:
    TokMod()
        {
        type = TOK_MOD;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        StackItem item1 = exec.pop();
        StackItem item2 = exec.pop();
        item1.dval = fmod(item1.dval, item2.dval);
        exec.push(item1);
        return true;
        }
};

class TokDiv : public Token
{
public:
    TokDiv()
        {
        type = TOK_DIV;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        StackItem item1 = exec.pop();
        StackItem item2 = exec.pop();
        item1.dval /= item2.dval;
        exec.push(item1);
        return true;
        }
};

class TokMul : public Token
{
public:
    TokMul()
        {
        type = TOK_MULTIPLY;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        StackItem item1 = exec.pop();
        StackItem item2 = exec.pop();
        item1.dval *= item2.dval;
        exec.push(item1);
        return true;
        }
};

class TokPlus : public Token
{
public:
    TokPlus()
        {
        type = TOK_PLUS;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        StackItem item1 = exec.pop();
        StackItem item2 = exec.pop();
        item1.dval += item2.dval;
        exec.push(item1);
        return true;
        }
};

class TokMinus : public Token
{
public:
    TokMinus()
        {
        type = TOK_MINUS;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        StackItem item1 = exec.pop();
        StackItem item2 = exec.pop();
        item1.dval -= item2.dval;
        exec.push(item1);
        return true;
        }
};

class TokNeg : public Token
{
public:
    TokNeg()
        {
        type = TOK_NEG;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        StackItem item = exec.pop();
        item.dval = -item.dval;
        item.ival = -item.ival;
        exec.push(item);
        return true;
        }
};

class TokEquals : public Token
{
public:
    TokEquals()
        {
        type = TOK_EQUALS;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        StackItem item1 = exec.pop();
        StackItem item2 = exec.pop();
        item1.ival = (item1.dval == item2.dval);
        exec.push(item1);
        return true;
        }
};

class TokNotEquals : public Token
{
public:
    TokNotEquals()
        {
        type = TOK_NOT_EQUALS;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        StackItem item1 = exec.pop();
        StackItem item2 = exec.pop();
        item1.ival = (item1.dval != item2.dval);
        exec.push(item1);
        return true;
        }
};

class TokLessThanEquals : public Token
{
public:
    TokLessThanEquals()
        {
        type = TOK_LESS_THAN_EQUALS;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        StackItem item1 = exec.pop();
        StackItem item2 = exec.pop();
        item1.ival = (item1.dval <= item2.dval);
        exec.push(item1);
        return true;
        }
};

class TokLessThan : public Token
{
public:
    TokLessThan()
        {
        type = TOK_LESS_THAN;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        StackItem item1 = exec.pop();
        StackItem item2 = exec.pop();
        item1.ival = (item1.dval < item2.dval);
        exec.push(item1);
        return true;
        }
};

class TokGreaterThanEquals : public Token
{
public:
    TokGreaterThanEquals()
        {
        type = TOK_GREATER_THAN_EQUALS;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        StackItem item1 = exec.pop();
        StackItem item2 = exec.pop();
        item1.ival = (item1.dval >= item2.dval);
        exec.push(item1);
        return true;
        }
};

class TokGreaterThan : public Token
{
public:
    TokGreaterThan()
        {
        type = TOK_GREATER_THAN;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        StackItem item1 = exec.pop();
        StackItem item2 = exec.pop();
        item1.ival = (item1.dval > item2.dval);
        exec.push(item1);
        return true;
        }
};


//###########################
//# X P A T H    I T E M S
//###########################

class TokAbsolute : public Token
{
public:
    TokAbsolute()
        {
        type = TOK_ABSOLUTE;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        Node *n = exec.axis.getNode();
        while (n->getParentNode())
             n = n->getParentNode();
        exec.axis.setNode(n);
        return true;
        }
};

class TokRelative : public Token
{
public:
    TokRelative()
        {
        type = TOK_RELATIVE;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        ///exec.axis.currentNode = stack.rootNode;
        return true;
        }
};

class TokStep : public Token
{
public:
    TokStep()
        {
        type = TOK_STEP;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokNameTest : public Token
{
public:
    TokNameTest(const DOMString &name)
        {
        type  = TOK_NAME_TEST;
        sval  = name;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokExpr : public Token
{
public:
    TokExpr()
        {
        type = TOK_EXPR;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokUnion : public Token
{
public:
    TokUnion()
        {
        type = TOK_UNION;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};




//###########################
//# A X I S
//###########################


class TokAxisAncestorOrSelf : public Token
{
public:
    TokAxisAncestorOrSelf()
        {
        type = TOK_AXIS_ANCESTOR_OR_SELF;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokAxisAncestor : public Token
{
public:
    TokAxisAncestor()
        {
        type = TOK_AXIS_ANCESTOR;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokAxisAttribute : public Token
{
public:
    TokAxisAttribute()
        {
        type = TOK_AXIS_ATTRIBUTE;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokAxisChild : public Token
{
public:
    TokAxisChild()
        {
        type = TOK_AXIS_CHILD;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokAxisDescendantOrSelf : public Token
{
public:
    TokAxisDescendantOrSelf()
        {
        type = TOK_AXIS_DESCENDANT_OR_SELF;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokAxisDescendant : public Token
{
public:
    TokAxisDescendant()
        {
        type = TOK_AXIS_DESCENDANT;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokAxisFollowingSibling : public Token
{
public:
    TokAxisFollowingSibling()
        {
        type = TOK_AXIS_FOLLOWING_SIBLING;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokAxisFollowing : public Token
{
public:
    TokAxisFollowing()
        {
        type = TOK_AXIS_FOLLOWING;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokAxisNamespace : public Token
{
public:
    TokAxisNamespace()
        {
        type = TOK_AXIS_NAMESPACE;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokAxisParent : public Token
{
public:
    TokAxisParent()
        {
        type = TOK_AXIS_PARENT;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokAxisPrecedingSibling : public Token
{
public:
    TokAxisPrecedingSibling()
        {
        type = TOK_AXIS_PRECEDING_SIBLING;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokAxisPreceding : public Token
{
public:
    TokAxisPreceding()
        {
        type = TOK_AXIS_PRECEDING;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokAxisSelf : public Token
{
public:
    TokAxisSelf()
        {
        type = TOK_AXIS_SELF;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};



//###########################
//# F U N C T I O N S
//###########################

class TokFuncLast : public Token
{
public:
    TokFuncLast()
        {
        type = TOK_FUNC_LAST;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokFuncPosition : public Token
{
public:
    TokFuncPosition()
        {
        type = TOK_FUNC_POSITION;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokFuncCount : public Token
{
public:
    TokFuncCount()
        {
        type = TOK_FUNC_COUNT;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokFuncId : public Token
{
public:
    TokFuncId()
        {
        type = TOK_FUNC_ID;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokFuncLocalName : public Token
{
public:
    TokFuncLocalName()
        {
        type = TOK_FUNC_LOCAL_NAME;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokFuncNamespaceUri : public Token
{
public:
    TokFuncNamespaceUri()
        {
        type = TOK_FUNC_NAMESPACE_URI;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokFuncName : public Token
{
public:
    TokFuncName()
        {
        type = TOK_FUNC_NAME;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokFuncString : public Token
{
public:
    TokFuncString()
        {
        type = TOK_FUNC_STRING;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokFuncConcat : public Token
{
public:
    TokFuncConcat()
        {
        type = TOK_FUNC_CONCAT;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokFuncStartsWith : public Token
{
public:
    TokFuncStartsWith()
        {
        type = TOK_FUNC_STARTS_WITH;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokFuncContains : public Token
{
public:
    TokFuncContains()
        {
        type = TOK_FUNC_CONTAINS;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokFuncSubstringBefore : public Token
{
public:
    TokFuncSubstringBefore()
        {
        type = TOK_FUNC_SUBSTRING_BEFORE;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokFuncSubstringAfter : public Token
{
public:
    TokFuncSubstringAfter()
        {
        type = TOK_FUNC_SUBSTRING_AFTER;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokFuncSubstring : public Token
{
public:
    TokFuncSubstring()
        {
        type = TOK_FUNC_SUBSTRING;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokFuncStringLength : public Token
{
public:
    TokFuncStringLength()
        {
        type = TOK_FUNC_STRING_LENGTH;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokFuncNormalizeSpace : public Token
{
public:
    TokFuncNormalizeSpace()
        {
        type = TOK_FUNC_NORMALIZE_SPACE;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokFuncTranslate : public Token
{
public:
    TokFuncTranslate()
        {
        type = TOK_FUNC_TRANSLATE;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokFuncBoolean : public Token
{
public:
    TokFuncBoolean()
        {
        type = TOK_FUNC_BOOLEAN;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokFuncNot : public Token
{
public:
    TokFuncNot()
        {
        type = TOK_FUNC_NOT;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokFuncTrue : public Token
{
public:
    TokFuncTrue()
        {
        type = TOK_FUNC_TRUE;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokFuncFalse : public Token
{
public:
    TokFuncFalse()
        {
        type = TOK_FUNC_FALSE;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokFuncLang : public Token
{
public:
    TokFuncLang()
        {
        type = TOK_FUNC_LANG;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokFuncNumber : public Token
{
public:
    TokFuncNumber()
        {
        type = TOK_FUNC_NUMBER;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokFuncSum : public Token
{
public:
    TokFuncSum()
        {
        type = TOK_FUNC_SUM;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokFuncFloor : public Token
{
public:
    TokFuncFloor()
        {
        type = TOK_FUNC_FLOOR;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokFuncCeiling : public Token
{
public:
    TokFuncCeiling()
        {
        type = TOK_FUNC_CEILING;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};

class TokFuncRound : public Token
{
public:
    TokFuncRound()
        {
        type = TOK_FUNC_ROUND;
        }
    virtual bool execute(TokenExecutor &exec)
        {
        return true;
        }
};






//########################################################################
//# T O K E N    L I S T
//########################################################################

/**
 *
 */
class TokenList
{
public:

    /**
     *
     */
    TokenList();

    /**
     *
     */
    TokenList(const TokenList &other);

    /**
     *
     */
    TokenList &operator=(const TokenList &other);

    /**
     *
     */
    void assign(const TokenList &other);

    /**
     *
     */
    virtual ~TokenList();

    /**
     *
     */
    virtual void clear();

    /**
     *
     */
    virtual void add(Token *tok);

    /**
     *
     */
    virtual unsigned int size() const;

    /**
     *
     */
    virtual void dump();

private:


    std::vector<Token *> tokens;


};







} // namespace xpath
} // namespace dom
} // namespace w3c
} // namespace org






#endif  /* __XPATHTOKEN_H__ */
//########################################################################
//# E N D    O F    F I L E
//########################################################################

