#include <glib.h>

#include <string>
#include <vector>


namespace MiniDom
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
        prefix       = other.prefix;
        namespaceURI = other.namespaceURI;
        }

    virtual ~Namespace()
        {}

    virtual DOMString getPrefix()
        { return prefix; }

    virtual DOMString getNamespaceURI()
        { return namespaceURI; }

protected:

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
        name  = other.name;
        value = other.value;
        }

    virtual ~Attribute()
        {}

    virtual DOMString getName()
        { return name; }

    virtual DOMString getValue()
        { return value; }

protected:

    DOMString name;
    DOMString value;

};


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
        name  = nameArg;
        value = valueArg;
        }

    Element(const Element &other)
        {
        parent     = other.parent;
        children   = other.children;
        attributes = other.attributes;
        namespaces = other.namespaces;
        name       = other.name;
        value      = other.value;
        }

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

    std::vector<Element *> getChildren()
        { return children; }
        
    int getLine()
        { return line; }

    std::vector<Element *> findElements(const DOMString &name);

    DOMString getAttribute(const DOMString &name);

    void addChild(Element *child);

    void addAttribute(const DOMString &name, const DOMString &value);

    void addNamespace(const DOMString &prefix, const DOMString &namespaceURI);


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


    void findElementsRecursive(std::vector<Element *>&res, const DOMString &name);

    void writeIndentedRecursive(FILE *f, int indent);

    Element *parent;

    std::vector<Element *>children;

    std::vector<Attribute> attributes;
    std::vector<Namespace> namespaces;

    DOMString name;
    DOMString value;
    
    int line;

};





class Parser
{
public:

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
    Element *parseFile(const char *fileName);




private:

    void init()
        {
        keepGoing       = true;
        currentNode     = NULL;
        parselen        = 0;
        parsebuf        = NULL;
        currentPosition = 0;
        }

    int countLines(int begin, int end);

    void getLineAndColumn(int pos, int *lineNr, int *colNr);

    void error(char *fmt, ...) G_GNUC_PRINTF(2,3);

    int peek(int pos);

    int match(int pos, const char *text);

    int skipwhite(int p);

    int getWord(int p0, DOMString &buf);

    int getQuoted(int p0, DOMString &buf, int do_i_parse);

    int parseVersion(int p0);

    int parseDoctype(int p0);

    int parseElement(int p0, Element *par,int depth);

    Element *parse(XMLCh *buf,int pos,int len);

    bool         keepGoing;
    Element      *currentNode;
    int          parselen;
    XMLCh        *parsebuf;
    DOMString    cdatabuf;
    int          currentPosition;
    int          colNr;


};



}//namespace MiniDom


//########################################################################
//#  E N D    O F    F I L E
//########################################################################

