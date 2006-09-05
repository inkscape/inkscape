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


class TokenExecutor;


//########################################################################
//# S T A C K    I T E M
//########################################################################


/**
 * This represents a single value on the evaluation stack
 */
class StackItem
{
public:

    /**
     *  Constructor
     */
    StackItem()
        { init(); }

    /**
     *  Copy constructor
     */
    StackItem(const StackItem &other)
        { assign(other); }

    /**
     *  Destructor
     */
    virtual ~StackItem()
        {}

    /**
     *
     */
    StackItem &operator=(const StackItem &other)
        { assign(other); return *this; }

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


private:

    void init()
        {
        sval = "";
        ival = 0;
        dval = 0.0;
        }

    void assign(const StackItem &other)
        {
        sval = other.sval; 
        ival = other.ival;
        dval = other.dval;
        }

};



//########################################################################
//# X P A T H    T O K E N
//########################################################################

class Token;
class Stack;

typedef bool (*TokenFunc)(Token &tok, Stack &stack);


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
        TOK_MUL,
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
        { init(); }

    /**
     *  Constructor with a NOP default type
     */
    Token(int typeArg)
        { init(); type = typeArg; }

    /**
     *  Constructor with a NOP default type
     */
    Token(int typeArg, 
          long ivalArg, double dvalArg, const DOMString &svalArg)
        { 
        init();
        type = typeArg,
        ival = ivalArg;
        dval = dvalArg;
        sval = svalArg;
        }

    /**
     * Copy constructor
     */
    Token(const Token &other)
        { assign(other); }

    /**
     * Assignment
     */
    Token &operator=(const Token &other)
        { assign(other); return *this; }

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
     *  Return true if the type is one of the Axis types
     *  above;
     */
    virtual bool isAxis()
        {
        return type>=TOK_AXIS_ANCESTOR_OR_SELF &&
                 type <= TOK_AXIS_SELF;
        }
    /**
     *  Return the string TokenType of this token
     */
    virtual DOMString getTypeString();

    /**
     *  Let this token execute itself on the given stack,
     *  possibly adding Nodes to the node list.
     */
    virtual bool execute(Stack &stack)
        {
        if (tokenFunc)
            return tokenFunc(*this, stack);
        return false;
        }

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

    /**
     *
     */
    static Token create(int type, long ival,
              double dval, const DOMString &sval);

    /**
     *
     */
    static Token create(int type)
        { return create(type, 0, 0.0, ""); }

    /**
     *
     */
    static Token create(int type, long val)
        { return create(type, val, 0.0, ""); }

    /**
     *
     */
    static Token create(int type, double val)
        { return create(type, 0, val, ""); }

    /**
     *
     */
    static Token create(int type, const DOMString &val)
        { return create(type, 0, 0.0, val); }



protected:

    /**
     * The enmerated token type
     */
    int type;

    /**
     * The function that defines the behaviour of this token
     */
    TokenFunc tokenFunc;


private:

    void init()
        {
        tokenFunc = NULL;
        type      = TOK_NOP;
        ival      = 0L;
        dval      = 0.0;
        //sval      = ""; //not necessary
        }

    void assign(const Token &other)
        {
        tokenFunc = other.tokenFunc;
        type      = other.type;
        sval      = other.sval;
        ival      = other.ival;
        dval      = other.dval;
        }


};


//########################################################################
//# S T A C K
//########################################################################

/**
 *
 */
class Stack
{
public:

    //# From 2.3, principal type of child axes
    typedef enum
        {
        AXIS_ATTRIBUTE,
        AXIS_NAMESPACE,
        AXIS_ELEMENT
        } PrincipalNodeType;

    /**
     *  Constructor
     */
    Stack(TokenExecutor &par) : parent(par)
        { clear(); }

    /**
     *  Copy constructor
     */
    Stack(const Stack &other) : parent(other.parent)
        { assign(other); }

    /**
     *  Destructor
     */
    virtual ~Stack()
        {}

    /**
     *
     */
    Stack &operator=(const Stack &other)
        { assign(other); return *this; }

    /**
     *
     */
    void push(StackItem &item)
        { return stackItems.push_back(item); }

    /**
     *
     */
    StackItem pop()
        {
        if (stackItems.size() < 1)
            {
            //TODO: error here
            StackItem item;
            return item;
            }
        std::vector<StackItem>::iterator iter =
                   stackItems.end() - 1;
        StackItem item = *iter;
        stackItems.erase(iter);
        return item;
        }

    /**
     *
     */
    virtual void clear()
        {
        stackItems.clear();
        principalNodeType = AXIS_ELEMENT;
        }

private:

    void assign(const Stack &other)
        {
        principalNodeType = other.principalNodeType;
        stackItems        = other.stackItems;
        }

    int principalNodeType;

    std::vector<StackItem> stackItems;

    TokenExecutor &parent;

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
    TokenList()
        { init(); }

    /**
     *
     */
    TokenList(const TokenList &other)
        { assign(other); }

    /**
     *
     */
    TokenList &operator=(const TokenList &other)
        { assign(other); return *this; }

    /**
     *
     */
    virtual ~TokenList()
        { }


    /**
     *
     */
    virtual void add(const Token &token)
        { tokens.push_back(token); }

    /**
     *
     */
    virtual std::vector<Token> &getTokens()
        { return tokens; }

    /**
     *
     */
    virtual void dump()
        {
        for (unsigned int i=0 ; i<tokens.size() ; i++)
            {
            Token token = tokens[i];
            token.dump();
            }
        }

    /**
     *
     */
    virtual void clear()
       {
       tokens.clear();
       }

private:


    void init()
        {
        clear();
        }

    void assign(const TokenList &other)
        {
        tokens = other.tokens;
        }

    std::vector<Token> tokens;


};




//########################################################################
//# T O K E N    E X E C U T O R
//########################################################################


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
     * Execute a list upon a given node.  For each Axis encountered,
     * get the nodes encountered so far, and execute the rest of the
     * list of tokens upon each of the nodes.
     */
    int execute(std::vector<Token> &tokens,
                int position,
                const NodePtr node,
                NodeList &nodeList);

    /**
     * Execute a token list on the stack
     */
    bool execute(TokenList &list,
                 const NodePtr node,
                 NodeList &result);

private:

    /**
     *
     */
    TokenList tokenList;

    /**
     *
     */
    NodeList nodeList;


};




} // namespace xpath
} // namespace dom
} // namespace w3c
} // namespace org






#endif  /* __XPATHTOKEN_H__ */
//########################################################################
//# E N D    O F    F I L E
//########################################################################

