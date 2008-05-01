#ifndef _XMLREADER_H_
#define _XMLREADER_H_

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

#include "dom.h"

namespace org
{
namespace w3c
{
namespace dom
{



class XmlReader
{
public:

    /**
     *
     */
    XmlReader();

    /**
     *
     */
    XmlReader(bool parseAsData);

    /**
     *
     */
    virtual ~XmlReader();

    /**
     *
     */
    org::w3c::dom::DocumentPtr parse(const DOMString &buf,
                            int offset, int length);

    /**
     *
     */
    org::w3c::dom::DocumentPtr parse(const DOMString &buf);

    /**
     *
     */
    org::w3c::dom::DocumentPtr parseFile(const DOMString &fileName);


private:

    void error(const char *format, ...)
    #ifdef G_GNUC_PRINTF
    G_GNUC_PRINTF(2, 3)
    #endif
    ;

    int  get(int ch);
    int  peek(int ch);
    bool match(int pos, char const *str);

    int  skipwhite(int ch);
    int  getWord(int pos, DOMString &result);
    int  getPrefixedWord(int pos,
                  DOMString &prefix,
                  DOMString &shortWord,
                  DOMString &fullWord);
    int  getQuoted(int p, DOMString &result);

    int parseVersion(int pos);
    int parseDoctype(int pos);

    int parseCDATA  (int pos, CDATASectionPtr cdata);
    int parseComment(int pos, CommentPtr comment);
    int parseText(int pos, TextPtr text);

    int parseEntity(int pos, DOMString &buf);

    int parseAttributes(int p0, NodePtr node, bool *quickClose);

    int parseNode(int p0, NodePtr node, int depth);

    bool       keepGoing;
    bool       parseAsData;
    int        pos;   //current parse position
    int        len;   //length of parsed region
    DOMString  parsebuf;

    DOMString  loadFile(const DOMString &fileName);

    int        lineNr;
    int        colNr;

    DocumentPtr document;

};

}  //namespace dom
}  //namespace w3c
}  //namespace org

#endif /*_XMLREADER_H_*/
