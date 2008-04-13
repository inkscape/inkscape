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



#include "xmlreader.h"
#include "ucd.h"
#include "domimpl.h"

#include <stdio.h>
#include <stdarg.h>

namespace org
{
namespace w3c
{
namespace dom
{


//#########################################################################
//# E N T I T Y    T A B L E
//#########################################################################
struct EntityInfo
{
    char *escape;
    int  escapeLength;
    char *value;
};


static EntityInfo entityTable[] =
{
    { "&amp;"  , 5 , "&"  },
    { "&lt;"   , 4 , "<"  },
    { "&gt;"   , 4 , ">"  },
    { "&apos;" , 6 , "'"  },
    { "&quot;" , 6 , "\"" },
    { NULL     , 0 , "\0" }
};



//#########################################################################
//# M E S S A G E S
//#########################################################################


/**
 *
 */
void XmlReader::error(char *fmt, ...)
{
    va_list args;
    fprintf(stderr, "XmlReader:error at line %d, column %d:", lineNr, colNr);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args) ;
    fprintf(stderr, "\n");
}



//#########################################################################
//# U T I L I T Y
//#########################################################################

static void trim(DOMString &str)
{
    int len = str.size();
    if (len<1)
        return;

    int start = 0;
    int end = 0;
    for (start=0 ; start<len ; start++)
        {
        int ch = str[start];
        if (ch<=' ' || ch>126)
            break;
        }
    for (end=len-1 ; end>=0 ; end--)
        {
        int ch = str[end];
        if (ch<=' ' || ch>126)
            break;
        }
    if (start<end)
        {
        str = str.substr(start, end+1);
        }
}

//#########################################################################
//# P A R S I N G
//#########################################################################

/**
 *  Get the character at the position and record the fact
 */
int XmlReader::get(int p)
{
    if (p >= len)
        return -1;
    int ch = parsebuf[p];
    //printf("%c", ch);
    if (ch == '\n' || ch == '\r')
        {
        colNr = 0;
        lineNr++;
        }
    else
        colNr++;
    return ch;
}

/**
 *  Look at the character at the position, but don't note the fact
 */
int XmlReader::peek(int p)
{
    if (p >= len)
        return -1;
    int ch = parsebuf[p];
    return ch;
}


/**
 *  Test if the given substring exists at the given position
 *  in parsebuf.  Use peek() in case of out-of-bounds
 */
bool XmlReader::match(int pos, char const *str)
{
    while (*str)
       {
       if (peek(pos++) != *str++)
           return false;
       }
   return true;
}



/**
 *  Test if the given substring exists at the given position
 *  in a given buffer
 */
/*
static bool bufMatch(const DOMString &buf, int pos, char *str)
{
    while (*str)
       {
       if (buf[pos++] != *str++)
           return false;
       }
   return true;
}
*/


/**
 *
 */
int XmlReader::skipwhite(int p)
{
  while (p < len)
    {
    int b = get(p);
    if (!uni_is_space(b))
        break;
    p++;
    }
  return p;
}

/**
 * modify this to allow all chars for an element or attribute name
 */
int XmlReader::getWord(int p, DOMString &result)
{
    while (p<len)
        {
        int b = get(p);
        if (b<=' ' || b=='/' || b=='>' || b=='=')
            break;
        result.push_back((XMLCh)b);
        p++;
        }
    return p;
}

/**
 * get a name and prefix, if any
 */
int XmlReader::getPrefixedWord(int p, DOMString &prefix,
                DOMString &shortWord, DOMString &fullWord)
{
    while (p<len)
        {
        int b = get(p);
        if (b<=' ' || b=='/' || b=='>' || b=='=')
            break;
        else if (b == ':')
            {
            prefix = shortWord;
            shortWord = "";
            }
        else
            shortWord.push_back((XMLCh)b);
        p++;
        }
    if (prefix.size() > 0)
        fullWord = prefix + ":" + shortWord;
    else
        fullWord = shortWord;
    return p;
}


/**
 * Assume that we are starting on a quote.  Ends on the char
 * after the final '"'
 */
int XmlReader::getQuoted(int p0, DOMString &result)
{

    int p = p0;

    if (peek(p)!='"' && peek(p)!='\'')
        return p0;

    int b = get(p++); //go to next char

    DOMString buf;

    while (p<len )
        {
        b = get(p++);
        if (b=='"' || b=='\'')
            break;
        else if (b=='&')
            {
            p = parseEntity(p, result);
            if (p < 0)
                return p0;
            }
        else
            {
            buf.push_back((XMLCh)b);
            }
        }

    //printf("quoted text:'%s'\n", buf.c_str());

    result.append(buf);

    return p;
}



/**
 * Parse a <!xml> tag.  Node may be null.  Assumes current char is '<'
 * ends on char after '>'
 */
int XmlReader::parseVersion(int p0)
{
    int p = p0;

    if (!match(p, "<?xml"))
        return p0;

    p     += 5;
    colNr += 5;

    bool quickCloseDummy;
    NodePtr node = new NodeImpl();
    int p2 = parseAttributes(p, node, &quickCloseDummy);
    if (p2 < p)
        {
        //smart ptr!!do not delete node;
        return p0;
        }
    p = p2;

    //get the attributes that we need
    NamedNodeMap attributes = node->getAttributes();
    NodePtr attr = attributes.getNamedItem("version");
    if (attr.get())
        document->setXmlVersion(attr->getNodeValue());
    attr = attributes.getNamedItem("encoding");
    if (attr.get())
        { /*document->setXmlEncoding(attr->getNodeValue());*/ }
    attr = attributes.getNamedItem("standalone");
    if (attr.get())
        document->setXmlStandalone((attr->getNodeValue() == "yes"));

    //#now we should be pointing at '?>'
    if (!match(p, "?>"))
        {
        return p0;
        }

    //skip over '?>'
    get(p++);
    get(p++);

    return p;
}


/**
 *  Parse a <!DOCTYPE> tag.  doctype may be null.  Expects '<'
 *  on start.  Ends pointing at char after '>'
 */
int XmlReader::parseDoctype(int p0)
{
    int p = p0;

    if (!match(p, "<!DOCTYPE"))
        return p0;

    p     += 9;
    colNr += 9;

    DocumentTypePtr doctype = document->getDoctype();
    if (!doctype)
        return p0;


    //### get the root name of the document
    p = skipwhite(p);
    DOMString rootName;
    int p2 = getWord(p, rootName);
    if (p2 <= p)
        return p0;
    p = p2;
    //printf("doctype root '%s'\n", rootName.c_str());


    while (p < len)
        {
        p = skipwhite(p);
        if (peek(p) == '>')
            break;
        else if (peek(p) == '[') //just ignore 'internal' [] stuff
            {
            while (p < len)
                {
                int ch = get(p++);
                if (ch == ']')
                    break;
                }
            p++;
            }
        else if (match(p, "PUBLIC"))
            {
            p     += 6;
            colNr += 6;
            p = skipwhite(p);
            DOMString pubIdLiteral;
            int p2 = getQuoted(p, pubIdLiteral);
            if (p2 <= p)
                return p0;
            p = p2;
            p = skipwhite(p);
            DOMString systemLiteral;
            p2 = getQuoted(p, systemLiteral);
            if (p2 <= p)
                return p0;
            p = p2;
            //printf("PUBLIC \"%s\" \"%s\" \n",
            //     pubIdLiteral.c_str(), systemLiteral.c_str());
            }
        else if (match(p, "SYSTEM"))
            {
            p     += 6;
            colNr += 6;
            p = skipwhite(p);
            DOMString systemLiteral;
            int p2 = getQuoted(p, systemLiteral);
            if (p2 <= p)
                return p0;
            p = p2;
            //printf("SYSTEM \"%s\" \n", systemLiteral.c_str());
            }
        }


    //skip over '>'
    get(p++);

    return p;
}



/**
 *  Expects '<' on startup, ends on char after '>'
 */
int XmlReader::parseComment(int p0, CommentPtr comment)
{
    int p = p0;

    if (!match(p, "<!--"))
        return p0;

    colNr += 4;
    p     += 4;

    DOMString buf;

    while (p<len-3)
        {
        if (match(p, "-->"))
            {
            p     += 3;
            colNr += 3;
            break;
            }
        int ch = get(p++);
        buf.push_back((XMLCh)ch);
        }

    comment->setNodeValue(buf);

    return p;
}



/**
 *
 */
int XmlReader::parseCDATA(int p0, CDATASectionPtr cdata)
{

    int p = p0;

    if (!match(p, "<![CDATA["))
        return p0;

    colNr += 9;
    p     += 9;

    DOMString buf;

    while (p<len)
        {
        if (match(p, "]]>"))
            {
            p     +=3;
            colNr += 3;
            break;
            }
        int ch = get(p++);
        buf.push_back((XMLCh)ch);
        }

    /*printf("Got CDATA:%s\n",buf.c_str());*/
    cdata->setNodeValue(buf);

    return p;
}



/**
 *
 */
int XmlReader::parseText(int p0, TextPtr text)
{

    int p = p0;

    DOMString buf;

    while (p<len)
        {
        if (peek(p) == '&')
            {
            p = parseEntity(p, buf);
            if (p < 0) //error?
                return p0;
            }
        else if (peek(p) == '<')
            {
            break;
            }
        else
            {
            int ch = get(p++);
            buf.push_back((XMLCh)ch);
            }
        }

    /*printf("Got Text:%s\n",buf.c_str());*/
    text->setNodeValue(buf);

    return p;
}





/**
 * Parses attributes of a node.  Should end pointing at either the
 * '?' of a version or doctype tag, or a '>' of a normal tag
 */
int XmlReader::parseAttributes(int p0, NodePtr node, bool *quickClose)
{
    *quickClose = false;

    int p = p0;

    NamedNodeMap attributes;

    while (p<len)
        {
        /*printf("ch:%c\n",ch);*/
        p  = skipwhite(p);
        int ch = get(p);

        /*printf("ch:%c\n",ch);*/
        if (ch == '?'  ||  ch == '>')//done
            break;
        else if (ch=='/' && p<len+1)
            {
            p++;
            p = skipwhite(p);
            ch = peek(p);
            if (ch == '>')
                {
                p++;
                *quickClose = true;
                /*printf("quick close\n");*/
                return p;
                }
            }
        DOMString shortName;
        DOMString prefix;
        DOMString qualifiedName;
        int p2 = getPrefixedWord(p, prefix, shortName, qualifiedName);
        if (p2 <= p)
            break;

        /*printf("name:%s",buf);*/
        p = p2;
        p = skipwhite(p);
        ch = get(p);
        /*printf("ch:%c\n",ch);*/
        if (ch != '=')
            break;
        p++;
        p = skipwhite(p);
        /*ch = parsebuf[p];*/
        /*printf("ch:%c\n",ch);*/
        DOMString attrValue;
        p2 = getQuoted(p, attrValue);
        p  = p2;
        /*printf("name:'%s'   value:'%s'\n",buf,buf2);*/

        DOMString namespaceURI = "";
        if (prefix == "xmlns" || shortName == "xmlns")
            namespaceURI = XMLNSNAME;

        //## Now let us make the attribute and give it to the node
        AttrPtr attr = document->createAttributeNS(namespaceURI, qualifiedName);
        attr->setValue(attrValue);
        node->getAttributes().setNamedItemNS(attr);

        }//while p<len

    return p;
}

/**
 * Appends the value of an entity to the buffer
 */
int XmlReader::parseEntity(int p0, DOMString &buf)
{
    int p = p0;
    for (EntityInfo *info = entityTable ; info->escape ; info++)
        {
        if (match(p, info->escape))
            {
            p     += info->escapeLength;
            colNr += info->escapeLength;
            buf   += info->value;
            return p;
            }
        }

    error("unterminated entity");
    return -1;
}


//#########################################################################
//# P A R S E    A    N O D E
//#########################################################################

/**
 *  Parse as a document, preserving the original structure as much as
 *  possible
 */
int XmlReader::parseNode(int p0, NodePtr node, int depth)
{

    int p = p0;


    //### OPEN TAG
    int ch = get(p++);
    if (ch !=  '<')
        return p0;

    p = skipwhite(p);
    DOMString openTagName;
    DOMString openTagNamePrefix;
    DOMString openTagQualifiedName;
    int p2 = getPrefixedWord(p,openTagNamePrefix,
                    openTagName, openTagQualifiedName);
    if (p2 <= p)
        return p0;
    p = p2;
    p = skipwhite(p);

    //printf("qualifiedName:%s\n", openTagQualifiedName.c_str());
    DOMString namespaceURI = node->lookupNamespaceURI(openTagNamePrefix);
    document->renameNode(node, namespaceURI, openTagQualifiedName);

    //### ATTRIBUTES
    bool quickClose;
    p = parseAttributes(p, node, &quickClose);
    if (quickClose)  //trivial tag:  <name/>
        return p;

    p++; //skip over '>'


    DOMString nodeValue;

    /* ### Get intervening data ### */
    while (p<len && keepGoing)
        {
        //### COMMENT
        if (match(p, "<!--"))
            {
            CommentPtr comment = document->createComment("");
            p2 = parseComment(p, comment);
            if (p2 <= p)
                return p0;
            p = p2;
            if (parseAsData)
                { //throw away
                //delete comment;
                }
            else
                {
                node->appendChild(comment);
                }
            }
        //### VERSION
        else if (match(p, "<?xml"))
            {
            p2 = parseVersion(p);
            if (p2 <= p)
                return p0;
            }
        //### DOCTYPE
        else if (match(p, "<!DOCTYPE"))
            {
            p2 = parseDoctype(p);
            if (p2 <= p)
                return p0;
            }
        //### CDATA
        else if (match(p, "<![CDATA["))
            {
            CDATASectionPtr cdata = document->createCDATASection("");
            p2 = parseCDATA(p, cdata);
            if (p2 <= p)
                return p0;
            p = p2;
            if (parseAsData)
                {
                nodeValue += cdata->getNodeValue();
                //delete cdata;
                }
            else
                {
                node->appendChild(cdata);
                }
            }
         //### OPEN OR CLOSE TAG
        else if (peek(p) == '<')
            {
            p2 = skipwhite(p+1);
            if (peek(p2) =='/')
                {
                p = p2;
                break;
                }
            else
                {
                /*Add element to tree*/
                ElementPtr elem = document->createElement(""); //fill in name later
                node->appendChild(elem);
                p2 = parseNode(p, elem, depth+1);
                if (p2 <= p)
                    {
                    /*printf("problem on element:%ls.  p2:%d p:%d\n",n->name, p2, p);*/
                    return p0;
                    }
                p = p2;
                }
            }
        //### TEXT
        else
            {
            TextPtr text = document->createTextNode("");
            p2 = parseText(p, text);
            if (p2 <= p)
                return p0;
            p = p2;
            if (parseAsData)
                {
                nodeValue += text->getNodeValue();
                //delete text;
                }
            else
                {
                node->appendChild(text);
                }
            }

        }//while (p<len)

    //printf("%d : nodeValue:'%s'\n", p, nodeValue.c_str());
    trim(nodeValue);
    node->setNodeValue(nodeValue);

    //### get close tag.  we should be pointing at '/'
    p = skipwhite(p);
    ch = get(p);
    if (ch != '/')
        {
        error("no / on end tag");
        return p0;
        }
    p++;

    //### get word after '/'
    p = skipwhite(p);
    DOMString closeTagName;
    DOMString closeTagNamePrefix;
    DOMString closeTagQualifiedName;
    p = getPrefixedWord(p, closeTagNamePrefix, closeTagName,
                        closeTagQualifiedName);
    if (openTagQualifiedName != closeTagQualifiedName)
        {
        error("Mismatched closing tag.  Expected </%s>. Got '%s'.",
              openTagQualifiedName.c_str(), closeTagQualifiedName.c_str());
        return p0;
        }
    p = skipwhite(p);
    if (parsebuf[p] != '>')
        {
        error("no > on end tag");
        return p0;
        }
    p++;
    /*printf("close element:%ls\n",buf);*/
    return p;
}


/**
 *
 */
org::w3c::dom::DocumentPtr
XmlReader::parse(const DOMString &buf, int bufferOffset, int parseLen)
{
    len      = parseLen;
    parsebuf = buf;

    keepGoing = true;

    DOMImplementationSourceImpl source;
    DOMImplementation *domImpl = source.getDOMImplementation("");

    document = domImpl->createDocument("", "", NULL);
    //document = new svg::SVGDocumentImpl(domImpl, "", "", NULL);

    int p  = bufferOffset;
    int p2 = 0;

    while (p<len && keepGoing)
        {
        p = skipwhite(p);
        //### COMMENT
        if (match(p, "<!--"))
            {
            CommentPtr comment = document->createComment("");
            p2 = parseComment(p, comment);
            if (p2 <= p)
                return document;
            p = p2;
            if (parseAsData)
                { //throw away
                //delete comment;
                }
            else
                {
                document->appendChild(comment);
                }
            }
        //### VERSION
        else if (match(p, "<?xml"))
            {
            p2 = parseVersion(p);
            if (p2 <= p)
                return document;
            p = p2;
            }
        //### DOCTYPE
        else if (match(p, "<!DOCTYPE"))
            {
            p2 = parseDoctype(p);
            if (p2 <= p)
                return document;
            p = p2;
            }
        else
            {
            break;
            }
        }

    p = skipwhite(p);
    p = parseNode(p, document->getDocumentElement(), 0);

    keepGoing = false;

    return document;
}


/**
 *
 */
org::w3c::dom::DocumentPtr
XmlReader::parse(const DOMString &str)
{

    DocumentPtr doc = parse(str, 0, str.size());
    doc->normalizeDocument();

    return doc;
}

/**
 *
 */
org::w3c::dom::DocumentPtr
XmlReader::parseFile(char *fileName)
{

    DOMString buf = loadFile(fileName);

    DocumentPtr doc = parse(buf, 0, buf.size());

    return doc;
}



//#########################################################################
//# S T R E A M    R E A D I N G
//#########################################################################

/**
 *
 */
org::w3c::dom::DOMString
XmlReader::loadFile(char *fileName)
{

    if (!fileName)
        return NULL;
    FILE *f = fopen(fileName, "rb");
    if (!f)
        return NULL;

    DOMString buf;
    while (!feof(f))
        {
        int ch = fgetc(f);
        if (ch<0)
            break;
        buf.push_back((XMLCh)ch);
        }
    fclose(f);

    return buf;
}


//#########################################################################
//# C O N S T R U C T O R    /    D E S T R U C T O R
//#########################################################################


/**
 *
 */
XmlReader::XmlReader()
{
    len         = 0;
    lineNr      = 1;
    colNr       = 0;
    parseAsData = false;
    keepGoing   = false;
}

/**
 *
 */
XmlReader::XmlReader(bool parseAsDataArg)
{
    len         = 0;
    lineNr      = 1;
    colNr       = 0;
    parseAsData = parseAsDataArg;
    keepGoing   = false;
}



/**
 *
 */
XmlReader::~XmlReader()
{
}


}  //namespace dom
}  //namespace w3c
}  //namespace org


//#########################################################################
//# E N D    O F    F I L E
//#########################################################################

