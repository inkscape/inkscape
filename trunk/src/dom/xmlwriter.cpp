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




#include <stdio.h>
#include <stdarg.h>

#include "xmlwriter.h"

namespace org
{
namespace w3c
{
namespace dom
{



//#########################################################################
//#  O U T P U T
//#########################################################################

/**
 *
 */
void XmlWriter::spaces()
{
    for (int i=0 ; i<indent ; i++)
        {
        buf.push_back(' ');
        }
}

/**
 *
 */
void XmlWriter::po(const char *fmt, ...)
{
    char str[257];
    va_list args;
    va_start(args, fmt);
    vsnprintf(str, 256,  fmt, args);
    va_end(args) ;

    buf.append(str);
}


void XmlWriter::pos(const DOMString &str)
{
    buf.append(str);
}

/**
 *
 */
void XmlWriter::write(const NodePtr nodeArg)
{
    NodePtr node = nodeArg;

    indent+=2;

    NamedNodeMap attributes = node->getAttributes();
    int nrAttrs = attributes.getLength();

    //### Start open tag
    spaces();
    po("<");
    pos(node->getNodeName());
    if (nrAttrs>0)
        po("\n");

    //### Attributes
    for (int i=0 ; i<nrAttrs ; i++)
        {
        NodePtr attr = attributes.item(i);
        spaces();
        pos(attr->getNodeName());
        po("=\"");
        pos(attr->getNodeValue());
        po("\"\n");
        }

    //### Finish open tag
    if (nrAttrs>0)
        spaces();
    po(">\n");

    //### Contents
    spaces();
    pos(node->getNodeValue());

    //### Children
    for (NodePtr child = node->getFirstChild() ;
         child.get() ;
         child=child->getNextSibling())
        {
        write(child);
        }

    //### Close tag
    spaces();
    po("</");
    pos(node->getNodeName());
    po(">\n");

    indent-=2;
}


/**
 *
 */
void XmlWriter::writeFile(FILE *f, const NodePtr node)
{
    if (!node)
       {
       po("XmlWriter: NULL document\n");
       return;
       }

    indent = 0;

    //po("Document\n");

    buf = "";

    write(node);

    for (unsigned int i=0 ; i<buf.size() ; i++)
        {
        int ch = buf[i];
        fputc(ch, f);
        }
    fflush(f);

    buf = "";

}



//#########################################################################
//#  C O N S T R U C T O R    /    D E S T R U C T O R
//#########################################################################

/**
 *
 */
XmlWriter::XmlWriter()
{
}

/**
 *
 */
XmlWriter::~XmlWriter()
{
}



}  //namespace dom
}  //namespace w3c
}  //namespace org

//#########################################################################
//#  E N D    O F    F I L E
//#########################################################################

