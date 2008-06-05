#ifndef __PEDRODOM_H__
#define __PEDRODOM_H__
/*
 * API for the Pedro mini-DOM parser and tree
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

#include <glib.h>

#include <string>
#include <vector>


namespace Pedro
{

typedef std::string DOMString;
typedef unsigned int XMLCh;


class Namespace
{
public:
    Namespace()
        {}

    Namespace(const DOMString &prefixArg, const DOMString &namespaceURIArg)
        {
        prefix       = prefixArg;
        namespaceURI = namespaceURIArg;
        }

    Namespace(const Namespace &other)
        {
        assign(other);
        }

    Namespace &operator=(const Namespace &other)
        {
        assign(other);
        return *this;
        }

    virtual ~Namespace()
        {}

    virtual DOMString getPrefix()
        { return prefix; }

    virtual DOMString getNamespaceURI()
        { return namespaceURI; }

protected:

    void assign(const Namespace &other)
        {
        prefix       = other.prefix;
        namespaceURI = other.namespaceURI;
        }

    DOMString prefix;
    DOMString namespaceURI;

};

class Attribute
{
public:
    Attribute()
        {}

    Attribute(const DOMString &nameArg, const DOMString &valueArg)
        {
        name  = nameArg;
        value = valueArg;
        }

    Attribute(const Attribute &other)
        {
        assign(other);
        }

    Attribute &operator=(const Attribute &other)
        {
        assign(other);
        return *this;
        }

    virtual ~Attribute()
        {}

    virtual DOMString getName()
        { return name; }

    virtual DOMString getValue()
        { return value; }

protected:

    void assign(const Attribute &other)
        {
        name  = other.name;
        value = other.value;
        }

    DOMString name;
    DOMString value;

};


//#Define a list of elements. (Children, search results, etc)
class Element;
typedef std::vector<Element *> ElementList;



class Element
{
friend class Parser;

public:
    Element()
        {
        parent = NULL;
        }

    Element(const DOMString &nameArg)
        {
        parent = NULL;
        name   = nameArg;
        }

    Element(const DOMString &nameArg, const DOMString &valueArg)
        {
        parent = NULL;
        name   = nameArg;
        value  = valueArg;
        }

    Element(const Element &other)
        {
        assign(other);
        }

    Element &operator=(const Element &other)
        {
        assign(other);
        return *this;
        }

    virtual Element *clone();

    virtual ~Element()
        {
        for (unsigned int i=0 ; i<children.size() ; i++)
            delete children[i];
        }

    virtual DOMString getName()
        { return name; }

    virtual DOMString getValue()
        { return value; }

    Element *getParent()
        { return parent; }

    Element *getFirstChild()
        { return (children.size() == 0) ? NULL : children[0]; }

    ElementList getChildren()
        { return children; }

    ElementList findElements(const DOMString &name);

    DOMString getAttribute(const DOMString &name);

    std::vector<Attribute> &getAttributes()
        { return attributes; } 

    DOMString getTagAttribute(const DOMString &tagName, const DOMString &attrName);

    DOMString getTagValue(const DOMString &tagName);

    void addChild(Element *child);

    void addAttribute(const DOMString &name, const DOMString &value);

    void addNamespace(const DOMString &prefix, const DOMString &namespaceURI);

    bool exists(const DOMString &name)
        { return (findElements(name).size() > 0); }

    /**
     * Prettyprint an XML tree to an output stream.  Elements are indented
     * according to element hierarchy.
     * @param f a stream to receive the output
     * @param elem the element to output
     */
    void writeIndented(FILE *f);

    /**
     * Prettyprint an XML tree to standard output.  This is the equivalent of
     * writeIndented(stdout).
     * @param elem the element to output
     */
    void print();

protected:

    void assign(const Element &other)
        {
        parent     = other.parent;
        children   = other.children;
        attributes = other.attributes;
        namespaces = other.namespaces;
        name       = other.name;
        value      = other.value;
        }

    void findElementsRecursive(std::vector<Element *>&res, const DOMString &name);

    void writeIndentedRecursive(FILE *f, int indent);

    Element *parent;

    ElementList children;

    std::vector<Attribute> attributes;
    std::vector<Namespace> namespaces;

    DOMString name;
    DOMString value;

};





class Parser
{
public:
    /**
     * Constructor
     */
    Parser()
        { init(); }

    virtual ~Parser()
        {}

    /**
     * Parse XML in a char buffer.
     * @param buf a character buffer to parse
     * @param pos position to start parsing
     * @param len number of chars, from pos, to parse.
     * @return a pointer to the root of the XML document;
     */
    Element *parse(const char *buf,int pos,int len);

    /**
     * Parse XML in a char buffer.
     * @param buf a character buffer to parse
     * @param pos position to start parsing
     * @param len number of chars, from pos, to parse.
     * @return a pointer to the root of the XML document;
     */
    Element *parse(const DOMString &buf);

    /**
     * Parse a named XML file.  The file is loaded like a data file;
     * the original format is not preserved.
     * @param fileName the name of the file to read
     * @return a pointer to the root of the XML document;
     */
    Element *parseFile(const DOMString &fileName);

    /**
     * Utility method to preprocess a string for XML
     * output, escaping its entities.
     * @param str the string to encode
     */
    static DOMString encode(const DOMString &str);

    /**
     *  Removes whitespace from beginning and end of a string
     */
    DOMString trim(const DOMString &s);

private:

    void init()
        {
        keepGoing       = true;
        currentNode     = NULL;
        parselen        = 0;
        parsebuf        = NULL;
        currentPosition = 0;
        }

    void getLineAndColumn(long pos, long *lineNr, long *colNr);

    void error(char const *fmt, ...) G_GNUC_PRINTF(2,3);

    int peek(long pos);

    int match(long pos, const char *text);

    int skipwhite(long p);

    int getWord(int p0, DOMString &buf);

    int getQuoted(int p0, DOMString &buf, int do_i_parse);

    int parseVersion(int p0);

    int parseDoctype(int p0);

    int parseElement(int p0, Element *par,int depth);

    Element *parse(XMLCh *buf,int pos,int len);

    bool       keepGoing;
    Element    *currentNode;
    long       parselen;
    XMLCh      *parsebuf;
    DOMString  cdatabuf;
    long       currentPosition;
    int        colNr;

};



}//namespace Pedro


#endif /* __PEDRODOM_H__ */

//########################################################################
//#  E N D    O F    F I L E
//########################################################################

