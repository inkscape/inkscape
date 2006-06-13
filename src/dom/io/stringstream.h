#ifndef __STRINGSTREAM_H__
#define __STRINGSTREAM_H__
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


#include "domstream.h"


namespace org
{
namespace w3c
{
namespace dom
{
namespace io
{


//#########################################################################
//# S T R I N G    I N P U T    S T R E A M
//#########################################################################

/**
 * This class is for reading character from a DOMString
 *
 */
class StringInputStream : public InputStream
{

public:

    StringInputStream(const DOMString &sourceString);

    virtual ~StringInputStream();

    virtual int available();

    virtual void close();

    virtual int get();

private:

    DOMString &buffer;

    long position;

}; // class StringInputStream




//#########################################################################
//# S T R I N G   O U T P U T    S T R E A M
//#########################################################################

/**
 * This class is for sending a stream to a  DOMString
 *
 */
class StringOutputStream : public OutputStream
{

public:

    StringOutputStream();

    virtual ~StringOutputStream();

    virtual void close();

    virtual void flush();

    virtual int put(XMLCh ch);

    virtual DOMString &getString()
        { return buffer; }

    virtual void clear()
        { buffer = ""; }

private:

    DOMString buffer;


}; // class StringOutputStream







}  //namespace io
}  //namespace dom
}  //namespace w3c
}  //namespace org



#endif /* __STRINGSTREAM_H__ */
