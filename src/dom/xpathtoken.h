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

#define STACK_SIZE 1024

/**
 * An evaluation stack
 */
class Stack
{
public:

    /**
     * Constructor
     */
    Stack();

    /**
     * Copy constructor
     */
    Stack(const Stack &other);

    /**
     * Destructor
     */
    virtual ~Stack();

    /**
     *  Assign our values to those of the other
     */
    virtual void assign(const Stack &other);

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
     * Set the root node
     */
    virtual void setRootNode(const Node *node);

    /**
     * Get the current node list;
     */
    virtual NodeList &getNodeList();


private:

    Node *root;
    NodeList nodeList;

    StackItem items[STACK_SIZE];
    int size;
};




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
        //function types
        TOK_POSITION
        } TokenType;
    /**
     *  Constructor with a NOP default type
     */
    Token()
        {
        type     = TOK_NOP;
        stype    = "nop";
        ival     = 0L;
        dval     = 0.0;
        }

    /**
     * Copy constructor
     */
    Token(const Token &other)
        {
        type     = other.type;
        stype    = other.stype;
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
     *  Let this token execute itself on the given stack,
     *  possibly adding Nodes to the node list.
     */
    virtual bool execute(Stack &stack)
        { return true; }

    /**
     *  Print the contents of this token
     */
    virtual void dump()
        {
        printf("%s %s %f %ld\n", stype, sval.c_str(), dval, ival);
        }

    //treat the token like an union of string, integer, and double

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

protected:

    /**
     * The enmerated token type
     */
    int type;

    /**
     * String type
     */
    char *stype;

private:


};


//########################################################################
//# X P A T H    T O K E N
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
        stype = "str";
        sval = val;
        }
    virtual bool execute(Stack &stack)
        {
        StackItem item;
        item.sval = sval;
        stack.push(item);
        return true;
        }
};

class TokFloat : public Token
{
public:
    TokFloat(double val)
        {
        type = TOK_FLOAT;
        stype = "float";
        dval = val;
        }
    virtual bool execute(Stack &stack)
        {
        StackItem item;
        item.dval = dval;
        stack.push(item);
        return true;
        }
};

class TokInt : public Token
{
public:
    TokInt(long val)
        {
        type = TOK_INT;
        stype = "int";
        ival = val;
        }
    virtual bool execute(Stack &stack)
        {
        StackItem item;
        item.ival = ival;
        stack.push(item);
        return true;
        }
};

class TokAnd : public Token
{
public:
    TokAnd()
        {
        type = TOK_AND;
        stype = "and";
        }
    virtual bool execute(Stack &stack)
        {
        StackItem item1 = stack.pop();
        StackItem item2 = stack.pop();
        item1.ival = item1.ival && item2.ival;
        stack.push(item1);
        return true;
        }
};

class TokOr : public Token
{
public:
    TokOr()
        {
        type = TOK_OR;
        stype = "or";
        }
    virtual bool execute(Stack &stack)
        {
        StackItem item1 = stack.pop();
        StackItem item2 = stack.pop();
        item1.ival = item1.ival || item2.ival;
        stack.push(item1);
        return true;
        }
};

class TokMod : public Token
{
public:
    TokMod()
        {
        type = TOK_MOD;
        stype = "mod";
        }
    virtual bool execute(Stack &stack)
        {
        StackItem item1 = stack.pop();
        StackItem item2 = stack.pop();
        item1.dval = fmod(item1.dval, item2.dval);
        stack.push(item1);
        return true;
        }
};

class TokDiv : public Token
{
public:
    TokDiv()
        {
        type = TOK_DIV;
        stype = "div";
        }
    virtual bool execute(Stack &stack)
        {
        StackItem item1 = stack.pop();
        StackItem item2 = stack.pop();
        item1.dval /= item2.dval;
        stack.push(item1);
        return true;
        }
};

class TokMul : public Token
{
public:
    TokMul()
        {
        type = TOK_MULTIPLY;
        stype = "mul";
        }
    virtual bool execute(Stack &stack)
        {
        StackItem item1 = stack.pop();
        StackItem item2 = stack.pop();
        item1.dval *= item2.dval;
        stack.push(item1);
        return true;
        }
};

class TokPlus : public Token
{
public:
    TokPlus()
        {
        type = TOK_PLUS;
        stype = "plus";
        }
    virtual bool execute(Stack &stack)
        {
        StackItem item1 = stack.pop();
        StackItem item2 = stack.pop();
        item1.dval += item2.dval;
        stack.push(item1);
        return true;
        }
};

class TokMinus : public Token
{
public:
    TokMinus()
        {
        type = TOK_MINUS;
        stype = "minus";
        }
    virtual bool execute(Stack &stack)
        {
        StackItem item1 = stack.pop();
        StackItem item2 = stack.pop();
        item1.dval -= item2.dval;
        stack.push(item1);
        return true;
        }
};

class TokNeg : public Token
{
public:
    TokNeg()
        {
        type = TOK_NEG;
        stype = "neg";
        }
    virtual bool execute(Stack &stack)
        {
        StackItem item;
        item.dval = -dval;
        item.ival = -ival;
        stack.push(item);
        return true;
        }
};

class TokEquals : public Token
{
public:
    TokEquals()
        {
        type = TOK_EQUALS;
        stype = "equals";
        }
    virtual bool execute(Stack &stack)
        {
        StackItem item1 = stack.pop();
        StackItem item2 = stack.pop();
        item1.ival = (item1.dval == item2.dval);
        stack.push(item1);
        return true;
        }
};

class TokNotEquals : public Token
{
public:
    TokNotEquals()
        {
        type = TOK_NOT_EQUALS;
        stype = "neq";
        }
    virtual bool execute(Stack &stack)
        {
        StackItem item1 = stack.pop();
        StackItem item2 = stack.pop();
        item1.ival = (item1.dval != item2.dval);
        stack.push(item1);
        return true;
        }
};

class TokLessThanEquals : public Token
{
public:
    TokLessThanEquals()
        {
        type = TOK_LESS_THAN_EQUALS;
        stype = "lt_eq";
        }
    virtual bool execute(Stack &stack)
        {
        StackItem item1 = stack.pop();
        StackItem item2 = stack.pop();
        item1.ival = (item1.dval <= item2.dval);
        stack.push(item1);
        return true;
        }
};

class TokLessThan : public Token
{
public:
    TokLessThan()
        {
        type = TOK_LESS_THAN;
        stype = "lt";
        }
    virtual bool execute(Stack &stack)
        {
        StackItem item1 = stack.pop();
        StackItem item2 = stack.pop();
        item1.ival = (item1.dval < item2.dval);
        stack.push(item1);
        return true;
        }
};

class TokGreaterThanEquals : public Token
{
public:
    TokGreaterThanEquals()
        {
        type = TOK_GREATER_THAN_EQUALS;
        stype = "gt";
        }
    virtual bool execute(Stack &stack)
        {
        StackItem item1 = stack.pop();
        StackItem item2 = stack.pop();
        item1.ival = (item1.dval >= item2.dval);
        stack.push(item1);
        return true;
        }
};

class TokGreaterThan : public Token
{
public:
    TokGreaterThan()
        {
        type = TOK_GREATER_THAN;
        stype = "gt_eq";
        }
    virtual bool execute(Stack &stack)
        {
        StackItem item1 = stack.pop();
        StackItem item2 = stack.pop();
        item1.ival = (item1.dval > item2.dval);
        stack.push(item1);
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
        stype = "absolute";
        }
    virtual bool execute(Stack &stack)
        {
        return true;
        }
};

class TokRelative : public Token
{
public:
    TokRelative()
        {
        type = TOK_RELATIVE;
        stype = "relative";
        }
    virtual bool execute(Stack &stack)
        {
        return true;
        }
};

class TokStep : public Token
{
public:
    TokStep()
        {
        type = TOK_STEP;
        stype = "step";
        }
    virtual bool execute(Stack &stack)
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
        stype = "step";
        sval  = name;
        }
    virtual bool execute(Stack &stack)
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
        stype = "token";
        }
    virtual bool execute(Stack &stack)
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
        stype = "union";
        }
    virtual bool execute(Stack &stack)
        {
        return true;
        }
};



//###########################
//# F U N C T I O N S
//###########################

class TokPosition : public Token
{
public:
    TokPosition()
        {
        type = TOK_POSITION;
        stype = "position";
        }
    virtual bool execute(Stack &stack)
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
     *  This method "executes" a list of Tokens in the context of a DOM root
     *  Node, returning a list of Nodes that match the xpath expression.
     */
    NodeList execute(const Node *root);

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

