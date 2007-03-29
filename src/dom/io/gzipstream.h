#ifndef __GZIPSTREAM_H__
#define __GZIPSTREAM_H__
/**
 * Zlib-enabled input and output streams
 *
 * This provides a simple mechanism for reading and
 * writing Gzip files.   We use our own 'ZipTool' class
 * to accomplish this, avoiding a zlib dependency.
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
//# G Z I P    I N P U T    S T R E A M
//#########################################################################

/**
 * This class is for deflating a gzip-compressed InputStream source
 *
 */
class GzipInputStream : public BasicInputStream
{

public:

    GzipInputStream(InputStream &sourceStream);

    virtual ~GzipInputStream();

    virtual int available();

    virtual void close();

    virtual int get();

private:

    bool load();

    bool loaded;

    std::vector<unsigned char> buffer;
    unsigned int bufPos;


}; // class GzipInputStream




//#########################################################################
//# G Z I P    O U T P U T    S T R E A M
//#########################################################################

/**
 * This class is for gzip-compressing data going to the
 * destination OutputStream
 *
 */
class GzipOutputStream : public BasicOutputStream
{

public:

    GzipOutputStream(OutputStream &destinationStream);

    virtual ~GzipOutputStream();

    virtual void close();

    virtual void flush();

    virtual int put(XMLCh ch);

private:

    std::vector<unsigned char> buffer;


}; // class GzipOutputStream







} // namespace io
} // namespace dom
} // namespace w3c
} // namespace org


#endif /* __GZIPSTREAM_H__ */
