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
 * Copyright (C) 2005-2007 Bob Jamison
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

#include "domimpl.h"
#include "events.h"
#include "traversal.h"
#include "lsimpl.h"

#include <stdarg.h>
#include <cstdio>

namespace org
{
namespace w3c
{
namespace dom
{
namespace ls
{



/*#########################################################################
## LSParserImpl
#########################################################################*/


/**
 *
 */
bool LSParserImpl::getBusy()
{
    return false;
}

/**
 *
 */
DocumentPtr LSParserImpl::parse(const LSInput &input)
                                throw(dom::DOMException, LSException)
{

    //#### Check the various inputs of 'input' in order, according
    //# to the L&S spec
    LSReader *lsreader = input.getCharacterStream();
    if (lsreader)
        {
        DOMString buf;
        while (true)
            {
            int ch = lsreader->get();
            if (ch < 0)
                break;
            buf.push_back((XMLCh)ch);
            }
        XmlReader reader;
        DocumentPtr doc = reader.parse(buf);
        return doc;
        }

    LSInputStream *inputStream = input.getByteStream();
    if (inputStream)
        {
        DOMString buf;
        while (true)
            {
            int ch = inputStream->get();
            if (ch < 0)
                break;
            buf.push_back((XMLCh)ch);
            }
        XmlReader reader;
        DocumentPtr doc = reader.parse(buf);
        return doc;
        }

    DOMString stringData = input.getStringData();
    if (stringData.size() > 0)
        {
        XmlReader reader;
        DocumentPtr doc = reader.parse(stringData);
        return doc;
        }

    DOMString systemId = input.getSystemId();
    if (systemId.size() > 0)
        {
        //lets not do this yet
        return NULL;
        }

    DOMString publicId = input.getPublicId();
    if (publicId.size() > 0)
        {
        //lets not do this yet
        return NULL;
        }

    return NULL;
}


/**
 *
 */
DocumentPtr LSParserImpl::parseURI(const DOMString &/*uri*/)
                                   throw(dom::DOMException, LSException)
{
    return NULL;
}

   /**
 *
 */
NodePtr LSParserImpl::parseWithContext(const LSInput &/*input*/,
                                       const NodePtr /*contextArg*/,
                                       unsigned short /*action*/)
                                       throw(dom::DOMException, LSException)
{
    return NULL;
}



//##################
//# Non-API methods
//##################











/*#########################################################################
## LSSerializerImpl
#########################################################################*/


/**
 *
 */
bool LSSerializerImpl::write(
                       const NodePtr nodeArg,
                       const LSOutput &destination)
                       throw (LSException)
{
    outbuf = "";
    indent = 0;

    writeNode(nodeArg);

    //## Check in order specified in the L&S specs
    LSWriter *writer = destination.getCharacterStream();
    if (writer)
        {
        for (unsigned int i=0 ; i<outbuf.size() ; i++)
            {
            int ch = outbuf[i];
            writer->put(ch);
            }
        return true;
        }

    LSOutputStream *outputStream = destination.getByteStream();
    if (outputStream)
        {
        for (unsigned int i=0 ; i<outbuf.size() ; i++)
            {
            int ch = outbuf[i];
            writer->put(ch);
            }
        return true;
        }

    DOMString systemId = destination.getSystemId();
    if (systemId.size() > 0)
        {
        //DO SOMETHING
        return true;
        }

    return false;
}

/**
 *
 */
bool LSSerializerImpl::writeToURI(const NodePtr nodeArg,
                                  const DOMString &uriArg)
                                  throw(LSException)
{
    outbuf = "";
    indent = 0;

    writeNode(nodeArg);

    DOMString uri = uriArg;
    char *fileName = (char *) uri.c_str(); //temporary hack
    FILE *f = fopen(fileName, "rb");
    if (!f)
        return false;
    for (unsigned int i=0 ; i<outbuf.size() ; i++)
        {
        int ch = outbuf[i];
        fputc(ch, f);
        }
    fclose(f);
    return false;
}

/**
 *
 */
DOMString LSSerializerImpl::writeToString(const NodePtr nodeArg)
                                    throw(dom::DOMException, LSException)
{
    outbuf = "";
    indent = 0;

    writeNode(nodeArg);
    return outbuf;
}



//##################
//# Non-API methods
//##################


/**
 *
 */
void LSSerializerImpl::spaces()
{
    for (int i=0 ; i<indent ; i++)
        {
        outbuf.push_back(' ');
        }
}

/**
 *
 */
void LSSerializerImpl::po(char const *fmt, ...)
{
    char str[257];
    va_list args;
    va_start(args, fmt);
    vsnprintf(str, 256,  fmt, args);
    va_end(args) ;

    outbuf.append(str);
}


void LSSerializerImpl::pos(const DOMString &str)
{
    outbuf.append(str);
}

void LSSerializerImpl::poxml(const DOMString &str)
{
    for (unsigned int i=0 ; i<str.size() ; i++)
        {
        XMLCh ch = (XMLCh) str[i];
        if (ch == '&')
            outbuf.append("&ampr;");
        else if (ch == '<')
            outbuf.append("&lt;");
        else if (ch == '>')
            outbuf.append("&gt;");
        else if (ch == '"')
            outbuf.append("&quot;");
        else if (ch == '\'')
            outbuf.append("&apos;");
        else
            outbuf.push_back(ch);
        }
}

/**
 *
 */
void LSSerializerImpl::writeNode(const NodePtr nodeArg)
{
    NodePtr node = nodeArg;

    int type = node->getNodeType();

    switch (type)
        {

        //#############
        //# DOCUMENT
        //#############
        case Node::DOCUMENT_NODE:
            {
            DocumentPtr doc = dynamic_cast<Document *>(node.get());
            writeNode(doc->getDocumentElement());
            }
        break;

        //#############
        //# TEXT
        //#############
        case Node::TEXT_NODE:
            {
            poxml(node->getNodeValue());
            }
        break;


        //#############
        //# CDATA
        //#############
        case Node::CDATA_SECTION_NODE:
            {
            pos("<![CDATA[");
            poxml(node->getNodeValue());
            pos("]]>");
            }
        break;


        //#############
        //# ELEMENT
        //#############
        case Node::ELEMENT_NODE:
            {

            indent+=2;

            NamedNodeMap attributes = node->getAttributes();
            int nrAttrs = attributes.getLength();

            //### Start open tag
            spaces();
            po("<");
            pos(node->getNodeName());
            //if (nrAttrs>0)
            //    pos(newLine);

            //### Attributes
            for (int i=0 ; i<nrAttrs ; i++)
                {
                NodePtr attr = attributes.item(i);
                spaces();
                pos(attr->getNodeName());
                po("=\"");
                pos(attr->getNodeValue());
                po("\"");
                //pos(newLine);
                }

            //### Finish open tag
            //if (nrAttrs>0)
            //    spaces();
            po(">");
            //pos(newLine);

            //### Contents
            //spaces();
            pos(node->getNodeValue());

            //### Children
            for (NodePtr child = node->getFirstChild() ;
                 child.get() ;
                 child=child->getNextSibling())
                {
                writeNode(child);
                }

            //### Close tag
            //spaces();
            po("</");
            pos(node->getNodeName());
            po(">");
            pos(newLine);

            indent-=2;
            }
            break;

        }//switch

}










}  //namespace ls
}  //namespace dom
}  //namespace w3c
}  //namespace org





/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/

