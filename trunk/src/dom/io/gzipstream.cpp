/**
 * Zlib-enabled input and output streams
 *
 * This is a thin wrapper of libz calls, in order
 * to provide a simple interface to our developers
 * for gzip input and output.
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2006-2007 Bob Jamison
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



#include "gzipstream.h"

#include "dom/util/ziptool.h"


namespace org
{
namespace w3c
{
namespace dom
{
namespace io
{


//#########################################################################
//# G Z I P    I N P U T    S T R E A M
//#########################################################################

/**
 *
 */
GzipInputStream::GzipInputStream(InputStream &sourceStream)
                    : BasicInputStream(sourceStream)
{
    loaded = false;
    bufPos = 0;
}

/**
 *
 */
GzipInputStream::~GzipInputStream()
{
    close();
}

/**
 * Returns the number of bytes that can be read (or skipped over) from
 * this input stream without blocking by the next caller of a method for
 * this input stream.
 */
int GzipInputStream::available()
{
    if (closed)
        return 0;
    if (!loaded)
        if (!load())
            return 0;
    return (int) buffer.size();
}


/**
 *  Closes this input stream and releases any system resources
 *  associated with the stream.
 */
void GzipInputStream::close()
{
    if (closed)
        return;

    closed = true;
}

/**
 * Reads the next byte of data from the input stream.  -1 if EOF
 */
int GzipInputStream::get()
{
    if (closed)
        return -1;

    if (!loaded)
        if (!load())
            return -1;

    if (bufPos >= buffer.size())
        return -1;
    int ch = (int) buffer[bufPos++];

    return ch;
}

/**
 * Processes input.  Fills read buffer.
 */
bool GzipInputStream::load()
{
    if (closed)
        return false;

    if (loaded)
        return true;

    std::vector<unsigned char> compBuf;
    while (true)
        {
        int ch = source.get();
        if (ch < 0)
            break;
        compBuf.push_back(ch);
        }
    GzipFile gz;
    if (!gz.readBuffer(compBuf))
        {
        return -1;
        }
    buffer = gz.getData();
    bufPos = 0;
    loaded = true;
    return true;
}






//#########################################################################
//# G Z I P   O U T P U T    S T R E A M
//#########################################################################

/**
 *
 */
GzipOutputStream::GzipOutputStream(OutputStream &destinationStream)
                     : BasicOutputStream(destinationStream)
{

    closed = false;
}

/**
 *
 */
GzipOutputStream::~GzipOutputStream()
{
    close();
}

/**
 * Closes this output stream and releases any system resources
 * associated with this stream.
 */
void GzipOutputStream::close()
{
    if (closed)
        return;

    flush();

    closed = true;
}

/**
 *  Flushes this output stream and forces any buffered output
 *  bytes to be written out.
 */
void GzipOutputStream::flush()
{
    if (closed || buffer.size()<1)
        return;

    std::vector<unsigned char> compBuf;
    GzipFile gz;

    gz.writeBuffer(buffer);

    std::vector<unsigned char>::iterator iter;
    for (iter=compBuf.begin() ; iter!=compBuf.end() ; iter++)
        {
        int ch = (int) *iter;
        destination.put(ch);
        }

    buffer.clear();

    //printf("done\n");

}



/**
 * Writes the specified byte to this output stream.
 */
int GzipOutputStream::put(XMLCh ch)
{
    if (closed)
        {
        //probably throw an exception here
        return -1;
        }

    //Add char to buffer
    buffer.push_back(ch);
    return 1;
}



} // namespace io
} // namespace dom
} // namespace w3c
} // namespace org




//#########################################################################
//# E N D    O F    F I L E
//#########################################################################



