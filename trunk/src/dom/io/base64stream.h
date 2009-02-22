#ifndef __DOM_IO_BASE64STREAM_H__
#define __DOM_IO_BASE64STREAM_H__

/**
 * Phoebe DOM Implementation.
 *
 * Base64-enabled input and output streams
 *
 * This class allows easy encoding and decoding
 * of Base64 data with a stream interface, hiding
 * the implementation from the user.
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
//# B A S E 6 4     I N P U T    S T R E A M
//#########################################################################

/**
 * This class is for decoding a Base-64 encoded InputStream source
 *
 */
class Base64InputStream : public BasicInputStream
{

public:

    Base64InputStream(InputStream &sourceStream);

    virtual ~Base64InputStream();

    virtual int available();

    virtual void close();

    virtual int get();

private:

    int outBytes[3];

    int outCount;

    int padCount;

    bool done;

}; // class Base64InputStream




//#########################################################################
//# B A S E 6 4   O U T P U T    S T R E A M
//#########################################################################

/**
 * This class is for Base-64 encoding data going to the
 * destination OutputStream
 *
 */
class Base64OutputStream : public BasicOutputStream
{

public:

    Base64OutputStream(OutputStream &destinationStream);

    virtual ~Base64OutputStream();

    virtual void close();

    virtual void flush();

    virtual int put(XMLCh ch);

    /**
     * Sets the maximum line length for base64 output.  If
     * set to <=0, then there will be no line breaks;
     */
    virtual void setColumnWidth(int val)
        { columnWidth = val; }

private:

    void putCh(int ch);

    int column;

    int columnWidth;

    unsigned long outBuf;

    int bitCount;

}; // class Base64OutputStream







}  //namespace io
}  //namespace dom
}  //namespace w3c
}  //namespace org


#endif /* __INKSCAPE_IO_BASE64STREAM_H__ */
