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
 *  
 * =======================================================================
 * NOTES
 * 
 *      
 */


#include "svgreader.h"
#include "dom/cssreader.h"
#include "dom/ucd.h"
#include "xmlreader.h"

#include <stdarg.h>


namespace org
{
namespace w3c
{
namespace dom
{
namespace svg
{


//#########################################################################
//# M E S S A G E S
//#########################################################################


/**
 *
 */
void SVGReader::error(char const *fmt, ...)
{
    va_list args;
    fprintf(stderr, "SVGReader:error: ");
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args) ;
    fprintf(stderr, "\n");
}


/**
 *
 */
void SVGReader::trace(char const *fmt, ...)
{
    va_list args;
    fprintf(stdout, "SVGReader: ");
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args) ;
    fprintf(stdout, "\n");
}



//#########################################################################
//# P A R S I N G
//#########################################################################



/**
 *  Get the character at the position and record the fact
 */
XMLCh SVGReader::get(int p)
{
    if (p >= parselen)
        return 0;
    XMLCh ch = parsebuf[p];
    //printf("%c", ch);
    lastPosition = p;
    return ch;
}



/**
 *  Test if the given substring exists at the given position
 *  in parsebuf.  Use get() in case of out-of-bounds
 */
bool SVGReader::match(int pos, char const *str)
{
    while (*str)
       {
       if (get(pos++) != (XMLCh) *str++)
           return false;
       }
   return true;
}

/**
 *
 */
int SVGReader::skipwhite(int p)
{
  while (p < parselen)
    {
    //# XML COMMENT
    if (match(p, "<!--"))
        {
        p+=4;
        bool done=false;
        while (p<parselen)
            {
            if (match(p, "-->"))
                {
                p+=3;
                done=true;
                break;
                }
            p++;
            }
        lastPosition = p;
        if (!done)
            {
            error("unterminated <!-- .. --> comment");
            return -1;
            }
        }
    //# C comment
    else if (match(p, "/*"))
        {
        p+=2;
        bool done=false;
        while (p<parselen)
            {
            if (match(p, "*/"))
                {
                p+=2;
                done=true;
                break;
                }
            p++;
            }
        lastPosition = p;
        if (!done)
            {
            error("unterminated /* .. */ comment");
            return -1;
            }
        }
    else if (!uni_is_space(get(p)))
        break;
    else
        p++;
    }
  lastPosition = p;
  return p;
}

/**
 * get a word from the buffer
 */
int SVGReader::getWord(int p, DOMString &result)
{
    XMLCh ch = get(p);
    if (!uni_is_letter(ch))
        return p;
    DOMString str;
    str.push_back(ch);
    p++;

    while (p < parselen)
        {
        ch = get(p);
        if (uni_is_letter_or_digit(ch) || ch=='-' || ch=='_')
            {
            str.push_back(ch);
            p++;
            }
        else if (ch == '\\')
            {
            p+=2;
            }
        else
            break;
        }
    result = str;
    return p;
}


# if 0
/**
 * get a word from the buffer
 */
int SVGReader::getNumber(int p0, double &result)
{
    int p=p0;

    DOMString str;

    //allow sign
    if (get(p) == '-')
        {
        p++;
        }

    while (p < parselen)
        {
        XMLCh ch = get(p);
        if (ch<'0' || ch>'9')
            break;
        str.push_back(ch);
        p++;
        }
    if (get(p) == '.' && get(p+1)>='0' && get(p+1)<='9')
        {
        p++;
        str.push_back('.');
        while (p < parselen)
            {
            XMLCh ch = get(p);
            if (ch<'0' || ch>'9')
                break;
            str.push_back(ch);
            p++;
            }
        }
    if (p>p0)
        {
        char *start = (char *)str.c_str();
        char *end   = NULL;
        double val = strtod(start, &end);
        if (end > start)
            {
            result = val;
            return p;
            }
        }

    //not a number
    return p0;
}
#endif


/**
 * get a word from the buffer
 */
int SVGReader::getNumber(int p0, double &result)
{
    int p=p0;

    char buf[64];

    int i;
    for (i=0 ; i<63 && p<parselen ; i++)
        {
        buf[i] = (char) get(p++);
        }
    buf[i] = '\0';

    char *start = buf;
    char *end   = NULL;
    double val = strtod(start, &end);
    if (end > start)
        {
        result = val;
        int count = (int)(end - start);
        p = p0 + count;
        return p;
        }

    //not a number
    return p0;
}


bool SVGReader::parseTransform(const DOMString &str)
{
    parsebuf = str;
    parselen = str.size();

    //printf("transform:%s\n", str.c_str());

    SVGTransformList transformList;

    int p = 0;

    while (p < parselen)
        {
        p = skipwhite(p);
        DOMString name;
        int p2 = getWord(p, name);
        if (p2<0)
            return false;
        if (p2<=p)
            {
            error("transform: need transform name");
            //return false;
            break;
            }
        p = p2;
        //printf("transform name:%s\n", name.c_str());

        //######### MATRIX
        if (name == "matrix")
            {
            p = skipwhite(p);
            if (get(p++) != '(')
                {
                error("matrix transform needs opening '('");
                return false;
                }
            int nrVals = 0;
            double vals[6];
            bool seenBrace = false;
            while (p < parselen && nrVals < 6)
                {
                p = skipwhite(p);
                double val = 0.0;
                p2 = getNumber(p, val);
                if (p2<0)
                    return false;
                if (p2<=p)
                    {
                    error("matrix() expected number");
                    return false;
                    }
                vals[nrVals++] = val;
                p = skipwhite(p2);
                XMLCh ch = get(p);
                if (ch == ',')
                    {
                    p++;
                    p = skipwhite(p);
                    ch = get(p);
                    }
                if (ch == ')')
                    {
                    seenBrace = true;
                    p++;
                    break;
                    }
                }
            if (!seenBrace)
                {
                error("matrix() needs closing brace");
                return false;
                }
            if (nrVals != 6)
                {
                error("matrix() requires exactly 6 arguments");
                return false;
                }
            //We got our arguments
            //printf("translate: %f %f %f %f %f %f\n",
            //      vals[0], vals[1], vals[2], vals[3], vals[4], vals[5]);
            SVGMatrix matrix(vals[0], vals[1], vals[2],
                             vals[3], vals[4], vals[5]);
            SVGTransform transform;
            transform.setMatrix(matrix);
            transformList.appendItem(transform);
            }

        //######### TRANSLATE
        else if (name == "translate")
            {
            p = skipwhite(p);
            if (get(p++) != '(')
                {
                error("matrix transform needs opening '('");
                return false;
                }
            p = skipwhite(p);
            double x = 0.0;
            p2 = getNumber(p, x);
            if (p2<0)
                return false;
            if (p2<=p)
                {
                error("translate() expected 'x' value");
                return false;
                }
            p = skipwhite(p2);
            if (get(p) == ',')
                {
                p++;
                p = skipwhite(p);
                }
            double y = 0.0;
            p2 = getNumber(p, y);
            if (p2<0)
                return false;
            if (p2<=p) //no y specified. use default
                y = 0.0;
            p = skipwhite(p2);
            if (get(p++) != ')')
                {
                error("translate() needs closing ')'");
                return false;
                }
            //printf("translate: %f %f\n", x, y);
            SVGTransform transform;
            transform.setTranslate(x, y);
            transformList.appendItem(transform);
            }

        //######### SCALE
        else if (name == "scale")
            {
            p = skipwhite(p);
            if (get(p++) != '(')
                {
                error("scale transform needs opening '('");
                return false;
                }
            p = skipwhite(p);
            double x = 0.0;
            p2 = getNumber(p, x);
            if (p2<0)
                return false;
            if (p2<=p)
                {
                error("scale() expected 'x' value");
                return false;
                }
            p = skipwhite(p2);
            if (get(p) == ',')
                {
                p++;
                p = skipwhite(p);
                }
            double y = 0.0;
            p2 = getNumber(p, y);
            if (p2<0)
                return false;
            if (p2<=p) //no y specified. use default
                y = x; // y is same as x.  uniform scaling
            p = skipwhite(p2);
            if (get(p++) != ')')
                {
                error("scale() needs closing ')'");
                return false;
                }
            //printf("scale: %f %f\n", x, y);
            SVGTransform transform;
            transform.setScale(x, y);
            transformList.appendItem(transform);
            }

        //######### ROTATE
        else if (name == "rotate")
            {
            p = skipwhite(p);
            if (get(p++) != '(')
                {
                error("rotate transform needs opening '('");
                return false;
                }
            p = skipwhite(p);
            double angle = 0.0;
            p2 = getNumber(p, angle);
            if (p2<0)
                return false;
            if (p2<=p)
                {
                error("rotate() expected 'angle' value");
                return false;
                }
            p = skipwhite(p2);
            if (get(p) == ',')
                {
                p++;
                p = skipwhite(p);
                }
            double cx = 0.0;
            double cy = 0.0;
            p2 = getNumber(p, cx);
            if (p2>p)
                {
                p = skipwhite(p2);
                if (get(p) == ',')
                    {
                    p++;
                    p = skipwhite(p);
                    }
                p2 = getNumber(p, cy);
                if (p2<0)
                    return false;
                if (p2<=p)
                    {
                    error("rotate() arguments should be either rotate(angle) or rotate(angle, cx, cy)");
                    return false;
                    }
                p = skipwhite(p2);
                }
            if (get(p++) != ')')
                {
                error("rotate() needs closing ')'");
                return false;
                }
            //printf("rotate: %f %f %f\n", angle, cx, cy);
            SVGTransform transform;
            transform.setRotate(angle, cx, cy);
            transformList.appendItem(transform);
            }

        //######### SKEWX
        else if (name == "skewX")
            {
            p = skipwhite(p);
            if (get(p++) != '(')
                {
                error("skewX transform needs opening '('");
                return false;
                }
            p = skipwhite(p);
            double x = 0.0;
            p2 = getNumber(p, x);
            if (p2<0)
                return false;
            if (p2<=p)
                {
                error("skewX() expected 'x' value");
                return false;
                }
            p = skipwhite(p2);
            if (get(p++) != ')')
                {
                error("skewX() needs closing ')'");
                return false;
                }
            //printf("skewX: %f\n", x);
            SVGTransform transform;
            transform.setSkewX(x);
            transformList.appendItem(transform);
            }

        //######### SKEWY
        else if (name == "skewY")
            {
            p = skipwhite(p);
            if (get(p++) != '(')
                {
                error("skewY transform needs opening '('");
                return false;
                }
            p = skipwhite(p);
            double y = 0.0;
            p2 = getNumber(p, y);
            if (p2<0)
                return false;
            if (p2<=p)
                {
                error("skewY() expected 'y' value");
                return false;
                }
            p = skipwhite(p2);
            if (get(p++) != ')')
                {
                error("skewY() needs closing ')'");
                return false;
                }
            //printf("skewY: %f\n", y);
            SVGTransform transform;
            transform.setSkewY(y);
            transformList.appendItem(transform);
            }

        //### NONE OF THE ABOVE
        else
            {
            error("unknown transform type:'%s'", name.c_str());
            }

        p = skipwhite(p);
        XMLCh ch = get(p);
        if (ch == ',')
            {
            p++;
            p = skipwhite(p);
            }

        }//WHILE p<parselen

    return true;
}


/**
 *
 */
bool SVGReader::parseElement(SVGElementImplPtr parent,
                             ElementImplPtr sourceElem)
{
    if (!parent)
        {
        error("NULL dest element");
        return false;
        }
    if (!sourceElem)
        {
        error("NULL source element");
        return false;
        }

    DOMString namespaceURI = sourceElem->getNamespaceURI();
    //printf("namespaceURI:%s\n", namespaceURI.c_str());
    DOMString tagName      = sourceElem->getTagName();
    printf("tag name:%s\n", tagName.c_str());
    ElementPtr newElement = doc->createElementNS(namespaceURI, tagName);
    if (!newElement)
        {
		return false;
		}
	NamedNodeMap &attrs = sourceElem->getAttributes();
	for (unsigned int i=0 ; i<attrs.getLength() ; i++)
	    {
	    NodePtr n = attrs.item(i);
	    newElement->setAttribute(n->getNodeName(), n->getNodeValue());//should be exception here
		}
	parent->appendChild(newElement);


    NodeList children = sourceElem->getChildNodes();
    int nodeCount = children.getLength();
    for (int i=0 ; i<nodeCount ; i++)
        {
        NodePtr child = children.item(i);
        int typ = child->getNodeType();
        if (typ == Node::TEXT_NODE)
            {
            NodePtr newNode = doc->createTextNode(child->getNodeValue());
            parent->appendChild(newNode);
            }
        else if (typ == Node::CDATA_SECTION_NODE)
            {
            NodePtr newNode = doc->createCDATASection(child->getNodeValue());
            parent->appendChild(newNode);
            }
        else if (newElement.get() && typ == Node::ELEMENT_NODE)
            {
            //ElementImplPtr childElement = dynamic_cast<ElementImpl *>(child.get());
            //parseElement(newElement, childElement);
            }
        }
    return true;
}


/**
 *
 */
SVGDocumentPtr SVGReader::parse(const DocumentPtr src)
{
    if (!src)
        {
        error("NULL source document");
        return NULL;
        }

    DOMImplementationImpl impl;
    doc = new SVGDocumentImpl(&impl, SVG_NAMESPACE, "svg" , NULL);

    SVGElementImplPtr destElem = dynamic_pointer_cast<SVGElementImpl, SVGElement>(doc->getRootElement());
    ElementImplPtr    srcElem  = dynamic_pointer_cast<ElementImpl, Element>(src->getDocumentElement());
    if (!parseElement(destElem, srcElem))
        {
        return NULL;
        }

    return doc;
}



/**
 *
 */
SVGDocumentPtr SVGReader::parse(const DOMString &buf)
{
    /* remember, smartptrs are null-testable*/
    SVGDocumentPtr svgdoc;
    XmlReader parser;
    DocumentPtr doc = parser.parse(buf);
    if (!doc)
        {
        return svgdoc;
        }
    svgdoc = parse(doc);
    return svgdoc;
}



/**
 *
 */
SVGDocumentPtr SVGReader::parseFile(const DOMString &fileName)
{
    /* remember, smartptrs are null-testable*/
    SVGDocumentPtr svgdoc;
    XmlReader parser;
    DocumentPtr doc = parser.parseFile(fileName);
    if (!doc)
        {
        error("Could not load xml doc");
        return svgdoc;
        }
    svgdoc = parse(doc);
    return svgdoc;
}




}  //namespace svg
}  //namespace dom
}  //namespace w3c
}  //namespace org

/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/

