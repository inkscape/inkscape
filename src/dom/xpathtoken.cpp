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
//# X P A T H    S T A C K    I T E M
//########################################################################

/**
 *
 */
StackItem::StackItem()
{
    ival = 0L;
    dval = 0.0;
}


/**
 *
 */
StackItem::StackItem(const StackItem &other)
{
    assign(other);
}


/**
 *
 */
StackItem::~StackItem()
{
}


/**
 *
 */
StackItem &StackItem::operator=(const StackItem &other)
{
    assign(other);
    return *this;
}

/**
 *
 */
void StackItem::assign(const StackItem &other)
{
    sval = other.sval;
    ival = other.ival;
    dval = other.dval;
}


//########################################################################
//# X P A T H    S T A C K
//########################################################################

/**
 *
 */
Stack::Stack()
{
    size = 0;
}


/**
 *
 */
Stack::Stack(const Stack &other)
{
    assign(other);
}


/**
 *
 */
Stack::~Stack()
{
}


/**
 *
 */
void Stack::assign(const Stack &other)
{
    root = other.root;
    nodeList = other.nodeList;
    size = other.size;
    for (int i=0 ; i<size ; i++)
        items[i] = other.items[i];
}


/**
 *
 */
void Stack::reset()
{
    root = NULL;
    NodeList n; /*no "clear" in api*/
    nodeList = n;
    size = 0;
}




/**
 *
 */
void Stack::push(StackItem &item)
{
    if (size>=STACK_SIZE)
        {
        return;
        }
    items[size++] = item;
}

/**
 *
 */
StackItem Stack::pop()
{
    if (size<1)
        {
        StackItem item;
        return item;
        }
    return items[--size];
}

/**
 * Set the root node
 */
void Stack::setRootNode(const Node *node)
{
    root = (Node *)node;
}


/**
 * Get the current node list;
 */
NodeList &Stack::getNodeList()
{
    return nodeList;
}


//########################################################################
//# T O K E N    L I S T
//########################################################################

/**
 *
 */
TokenList::TokenList()
{
}


/**
 *
 */
TokenList::TokenList(const TokenList &other)
{
    assign(other);
}

/**
 *
 */
TokenList &TokenList::operator=(const TokenList &other)
{
    assign(other);
    return *this;
}

/**
 *
 */
void TokenList::assign(const TokenList &other)
{
    tokens = other.tokens;
}

/**
 *
 */
TokenList::~TokenList()
{
    clear();
}

/**
 *
 */
void TokenList::clear()
{
    std::vector<Token *>::iterator iter;
    for (iter = tokens.begin() ; iter!= tokens.end() ; iter++)
        {
        delete (*iter);
        }
    tokens.clear();
}

/**
 *
 */
void TokenList::add(Token *tok)
{
    tokens.push_back(tok);
}

/**
 *  This method "executes" a list of Tokens in the context of a DOM root
 *  Node, returning a list of Nodes that match the xpath expression.
 */
NodeList TokenList::execute(const Node *root)
{
    NodeList list;

    if (!root)
        return list;

    Stack stack;
    stack.setRootNode(root);

    //### Execute the token list
    std::vector<Token *>::iterator iter;
    for (iter = tokens.begin() ; iter != tokens.end() ; iter++)
        {
        Token *tok = *iter;
        tok->execute(stack);
        }

    list = stack.getNodeList();

    return list;
}


/**
 *
 */
void TokenList::dump()
{
    std::vector<Token *>::iterator iter;
    printf("############# TOKENS\n");
    for (iter = tokens.begin() ; iter != tokens.end() ; iter++)
        {
        Token *tok = *iter;
        tok->dump();
        }
}





} // namespace xpath
} // namespace dom
} // namespace w3c
} // namespace org
//########################################################################
//# E N D    O F    F I L E
//########################################################################



