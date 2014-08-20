/*
 * Zlib-enabled input and output streams
 *
 * This is a thin wrapper of libz calls, in order
 * to provide a simple interface to our developers
 * for gzip input and output.
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004
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
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace Inkscape
{
namespace IO
{

//#########################################################################
//# G Z I P    I N P U T    S T R E A M
//#########################################################################

#define OUT_SIZE 4000

/**
 *
 */ 
GzipInputStream::GzipInputStream(InputStream &sourceStream)
                    : BasicInputStream(sourceStream),
                      loaded(false),
                      totalIn(0),
                      totalOut(0),
                      outputBuf(NULL),
                      srcBuf(NULL),
                      crc(0),
                      srcCrc(0),
                      srcSiz(0),
                      srcConsumed(0),
                      srcLen(0),
                      outputBufPos(0),
                      outputBufLen(0)
{
    memset( &d_stream, 0, sizeof(d_stream) );
}

/**
 *
 */ 
GzipInputStream::~GzipInputStream()
{
    close();
    if ( srcBuf ) {
      delete[] srcBuf;
      srcBuf = NULL;
    }
    if ( outputBuf ) {
        delete[] outputBuf;
        outputBuf = NULL;
    }
}

/**
 * Returns the number of bytes that can be read (or skipped over) from
 * this input stream without blocking by the next caller of a method for
 * this input stream.
 */ 
int GzipInputStream::available()
{
    if (closed || !outputBuf)
        return 0;
    return outputBufLen - outputBufPos;
}

    
/**
 *  Closes this input stream and releases any system resources
 *  associated with the stream.
 */ 
void GzipInputStream::close()
{
    if (closed)
        return;

    int zerr = inflateEnd(&d_stream);
    if (zerr != Z_OK) {
        printf("inflateEnd: Some kind of problem: %d\n", zerr);
    }

    if ( srcBuf ) {
      delete[] srcBuf;
      srcBuf = NULL;
    }
    if ( outputBuf ) {
        delete[] outputBuf;
        outputBuf = NULL;
    }
    closed = true;
}
    
/**
 * Reads the next byte of data from the input stream.  -1 if EOF
 */ 
int GzipInputStream::get()
{
    int ch = -1;
    if (closed) {
        // leave return value -1
    }
    else if (!loaded && !load()) {
        closed=true;
    } else {
        loaded = true;

        if ( outputBufPos >= outputBufLen ) {
            // time to read more, if we can
            fetchMore();
        }

        if ( outputBufPos < outputBufLen ) {
            ch = (int)outputBuf[outputBufPos++];
        }
    }

    return ch;
}

#define FTEXT 0x01
#define FHCRC 0x02
#define FEXTRA 0x04
#define FNAME 0x08
#define FCOMMENT 0x10

bool GzipInputStream::load()
{
    crc = crc32(0L, Z_NULL, 0);
    
    std::vector<Byte> inputBuf;
    while (true)
        {
        int ch = source.get();
        if (ch<0)
            break;
        inputBuf.push_back(static_cast<Byte>(ch & 0xff));
        }
    long inputBufLen = inputBuf.size();
    
    if (inputBufLen < 19) //header + tail + 1
        {
        return false;
        }

    srcLen = inputBuf.size();
    srcBuf = new Byte [srcLen];
    if (!srcBuf) {
        return false;
    }

    outputBuf = new unsigned char [OUT_SIZE];
    if ( !outputBuf ) {
        delete[] srcBuf;
        srcBuf = NULL;
        return false;
    }
    outputBufLen = 0; // Not filled in yet

    std::vector<unsigned char>::iterator iter;
    Bytef *p = srcBuf;
    for (iter=inputBuf.begin() ; iter != inputBuf.end() ; ++iter)
	{
        *p++ = *iter;
	}

    int headerLen = 10;

    //Magic
    //int val = (int)srcBuf[0];
    ////printf("val:%x\n", val);
    //val = (int)srcBuf[1];
    ////printf("val:%x\n", val);

    ////Method
    //val = (int)srcBuf[2];
    ////printf("val:%x\n", val);

    //flags
    int flags = static_cast<int>(srcBuf[3]);

    ////time
    //val = (int)srcBuf[4];
    //val = (int)srcBuf[5];
    //val = (int)srcBuf[6];
    //val = (int)srcBuf[7];

    ////xflags
    //val = (int)srcBuf[8];
    ////OS
    //val = (int)srcBuf[9];

//     if ( flags & FEXTRA ) {
//         headerLen += 2;
//         int xlen = 
//         TODO deal with optional header parts
//     }
    if ( flags & FNAME ) {
        int cur = 10;
        while ( srcBuf[cur] )
        {
            cur++;
            headerLen++;
        }
        headerLen++;
    }


    srcCrc = ((0x0ff & srcBuf[srcLen - 5]) << 24)
           | ((0x0ff & srcBuf[srcLen - 6]) << 16)
           | ((0x0ff & srcBuf[srcLen - 7]) <<  8)
           | ((0x0ff & srcBuf[srcLen - 8]) <<  0);
    //printf("srcCrc:%lx\n", srcCrc);
    
    srcSiz = ((0x0ff & srcBuf[srcLen - 1]) << 24)
           | ((0x0ff & srcBuf[srcLen - 2]) << 16)
           | ((0x0ff & srcBuf[srcLen - 3]) <<  8)
           | ((0x0ff & srcBuf[srcLen - 4]) <<  0);
    //printf("srcSiz:%lx/%ld\n", srcSiz, srcSiz);
    
    //outputBufLen = srcSiz + srcSiz/100 + 14;
    
    unsigned char *data = srcBuf + headerLen;
    unsigned long dataLen = srcLen - (headerLen + 8);
    //printf("%x %x\n", data[0], data[dataLen-1]);
    
    d_stream.zalloc    = (alloc_func)0;
    d_stream.zfree     = (free_func)0;
    d_stream.opaque    = (voidpf)0;
    d_stream.next_in   = data;
    d_stream.avail_in  = dataLen;
    d_stream.next_out  = outputBuf;
    d_stream.avail_out = OUT_SIZE;
    
    int zerr = inflateInit2(&d_stream, -MAX_WBITS);
    if ( zerr == Z_OK )
    {
        zerr = fetchMore();
    } else {
        printf("inflateInit2: Some kind of problem: %d\n", zerr);
    }

        
    return (zerr == Z_OK) || (zerr == Z_STREAM_END);
}


int GzipInputStream::fetchMore()
{
    // TODO assumes we aren't called till the buffer is empty
    d_stream.next_out  = outputBuf;
    d_stream.avail_out = OUT_SIZE;
    outputBufLen = 0;
    outputBufPos = 0;

    int zerr = inflate( &d_stream, Z_SYNC_FLUSH );
    if ( zerr == Z_OK || zerr == Z_STREAM_END ) {
        outputBufLen = OUT_SIZE - d_stream.avail_out;
        if ( outputBufLen ) {
            crc = crc32(crc, const_cast<const Bytef *>(outputBuf), outputBufLen);
        }
        //printf("crc:%lx\n", crc);
//     } else if ( zerr != Z_STREAM_END ) {
//         // TODO check to be sure this won't happen for partial end reads
//         printf("inflate: Some kind of problem: %d\n", zerr);
    }

    return zerr;
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

    totalIn         = 0;
    totalOut        = 0;
    crc             = crc32(0L, Z_NULL, 0);

    //Gzip header
    destination.put(0x1f);
    destination.put(0x8b);

    //Say it is compressed
    destination.put(Z_DEFLATED);

    //flags
    destination.put(0);

    //time
    destination.put(0);
    destination.put(0);
    destination.put(0);
    destination.put(0);

    //xflags
    destination.put(0);

    //OS code - from zutil.h
    //destination.put(OS_CODE);
    //apparently, we should not explicitly include zutil.h
    destination.put(0);

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

    //# Send the CRC
    uLong outlong = crc;
    for (int n = 0; n < 4; n++)
        {
        destination.put(static_cast<gunichar>(outlong & 0xff));
        outlong >>= 8;
        }
    //# send the file length
    outlong = totalIn & 0xffffffffL;
    for (int n = 0; n < 4; n++)
        {
        destination.put(static_cast<gunichar>(outlong & 0xff));
        outlong >>= 8;
        }

    destination.close();
    closed = true;
}
    
/**
 *  Flushes this output stream and forces any buffered output
 *  bytes to be written out.
 */ 
void GzipOutputStream::flush()
{
    if (closed || inputBuf.empty())
	{
        return;
    }
	
    uLong srclen = inputBuf.size();
    Bytef *srcbuf = new Bytef [srclen];
    if (!srcbuf)
        {
        return;
        }
        
    uLong destlen = srclen;
    Bytef *destbuf = new Bytef [(destlen + (srclen/100) + 13)];
    if (!destbuf)
        {
        delete[] srcbuf;
        return;
        }
        
    std::vector<unsigned char>::iterator iter;
    Bytef *p = srcbuf;
    for (iter=inputBuf.begin() ; iter != inputBuf.end() ; ++iter)
        *p++ = *iter;
        
    crc = crc32(crc, const_cast<const Bytef *>(srcbuf), srclen);
    
    int zerr = compress(destbuf, static_cast<uLongf *>(&destlen), srcbuf, srclen);
    if (zerr != Z_OK)
        {
        printf("Some kind of problem\n");
        }

    totalOut += destlen;
    //skip the redundant zlib header and checksum
    for (uLong i=2; i<destlen-4 ; i++)
        {
        destination.put((int)destbuf[i]);
        }
        
    destination.flush();

    inputBuf.clear();
    delete[] srcbuf;
    delete[] destbuf;
}



/**
 * Writes the specified byte to this output stream.
 */ 
int GzipOutputStream::put(gunichar ch)
{
    if (closed)
        {
        //probably throw an exception here
        return -1;
        }


    //Add char to buffer
    inputBuf.push_back(ch);
    totalIn++;
    return 1;
}



} // namespace IO
} // namespace Inkscape


//#########################################################################
//# E N D    O F    F I L E
//#########################################################################

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
