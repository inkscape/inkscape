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
 * Copyright (C) 2005 Bob Jamison
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

/**
 * Our base String stream classes.  We implement these to
 * be based on DOMString
 *
 */


#include "stringstream.h"

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
 *
 */
StringInputStream::StringInputStream(const DOMString &sourceString)
                      : buffer((DOMString &)sourceString)
{
    position = 0;
}

/**
 *
 */
StringInputStream::~StringInputStream()
{

}

/**
 * Returns the number of bytes that can be read (or skipped over) from
 * this input stream without blocking by the next caller of a method for
 * this input stream.
 */
int StringInputStream::available()
{
    return buffer.size() - position;
}


/**
 *  Closes this input stream and releases any system resources
 *  associated with the stream.
 */
void StringInputStream::close()
{
}

/**
 * Reads the next byte of data from the input stream.  -1 if EOF
 */
int StringInputStream::get()
{
    if (position >= (int)buffer.size())
        return -1;
    int ch = (int) buffer[position++];
    return ch;
}




//#########################################################################
//# S T R I N G     O U T P U T    S T R E A M
//#########################################################################

/**
 *
 */
StringOutputStream::StringOutputStream()
{
}

/**
 *
 */
StringOutputStream::~StringOutputStream()
{
}

/**
 * Closes this output stream and releases any system resources
 * associated with this stream.
 */
void StringOutputStream::close()
{
}

/**
 *  Flushes this output stream and forces any buffered output
 *  bytes to be written out.
 */
void StringOutputStream::flush()
{
    //nothing to do
}

/**
 * Writes the specified byte to this output stream.
 */
int StringOutputStream::put(XMLCh ch)
{
    buffer.push_back(ch);
    return 1;
}




}  //namespace io
}  //namespace dom
}  //namespace w3c
}  //namespace org

//#########################################################################
//# E N D    O F    F I L E
//#########################################################################
