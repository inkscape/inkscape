#ifndef SEEN_INKSCAPE_IO_GZIPSTREAM_H
#define SEEN_INKSCAPE_IO_GZIPSTREAM_H
/**
 * @file
 * Zlib-enabled input and output streams.
 *
 * This is a thin wrapper of libz calls, in order
 * to provide a simple interface to our developers
 * for gzip input and output.
 */
/*
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Inkscape.org
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <vector>
#include "inkscapestream.h"
#include <zlib.h>

namespace Inkscape
{
namespace IO
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
    int fetchMore();

    bool loaded;
    
    long totalIn;
    long totalOut;

    unsigned char *outputBuf;
    unsigned char *srcBuf;

    unsigned long crc;
    unsigned long srcCrc;
    unsigned long srcSiz;
    unsigned long srcConsumed;
    unsigned long srcLen;
    long outputBufPos;
    long outputBufLen;

    z_stream d_stream;
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
    
    virtual int put(gunichar ch);

private:

    std::vector<unsigned char> inputBuf;

    long totalIn;
    long totalOut;
    unsigned long crc;

}; // class GzipOutputStream







} // namespace IO
} // namespace Inkscape


#endif /* __INKSCAPE_IO_GZIPSTREAM_H__ */
