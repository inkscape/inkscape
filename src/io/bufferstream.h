#ifndef SEEN_BUFFERSTREAM_H
#define SEEN_BUFFERSTREAM_H
/**
 * @file
 * Phoebe DOM Implementation.
 *
 * This is a C++ approximation of the W3C DOM model, which follows
 * fairly closely the specifications in the various .idl files, copies of
 * which are provided for reference.  Most important is this one:
 *
 * http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/idl-definitions.html
 */
/*
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

#include <vector>
#include "inkscapestream.h"


namespace Inkscape
{
namespace IO
{

//#########################################################################
//# S T R I N G    I N P U T    S T R E A M
//#########################################################################

/**
 * This class is for reading character from a DOMString
 *
 */
class BufferInputStream : public InputStream
{

public:

    BufferInputStream(const std::vector<unsigned char> &sourceBuffer);
    virtual ~BufferInputStream();
    virtual int available();
    virtual void close();
    virtual int get();

private:
    const std::vector<unsigned char> &buffer;
    long position;
    bool closed;

}; // class BufferInputStream




//#########################################################################
//# B U F F E R     O U T P U T    S T R E A M
//#########################################################################

/**
 * This class is for sending a stream to a character buffer
 *
 */
class BufferOutputStream : public OutputStream
{

public:

    BufferOutputStream();
    virtual ~BufferOutputStream();
    virtual void close();
    virtual void flush();
    virtual int put(gunichar ch);
    virtual std::vector<unsigned char> &getBuffer()
        { return buffer; }

    virtual void clear()
        { buffer.clear(); }

private:
    std::vector<unsigned char> buffer;
    bool closed;

}; // class BufferOutputStream



}  //namespace IO
}  //namespace Inkscape



#endif // SEEN_BUFFERSTREAM_H
