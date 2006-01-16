/*
 * IO layer : zlib streambuf
 *
 * Authors:
 *   Johan Ceuppens <jceuppen at easynet dot be>
 *
 * Copyright (C) 2004 Johan Ceuppens
 *
 * Released under GNU LGPL, read the file 'COPYING.LIB' for more information
 */

#ifndef __STREAMS_ZLIB_H_
#define __STREAMS_ZLIB_H_

#include "streams-handles.h"

#include <glib/gtypes.h>
#include <glib/garray.h>
#include <zlib.h>
#include <iostream>

namespace Inkscape {

class ZlibBufferException : public std::exception {};

// This is the initial buffersize for the stream and
// zipbuffers (the streambuffers expand as needed).
const unsigned int BUFSIZE_STREAM = 4096; 

/**
 * ZlibBuffer
 */ 

//TODO: unbuffered IO
class ZlibBuffer : public std::streambuf
{
public:

    ZlibBuffer(URIHandle& urih);
    virtual ~ZlibBuffer() {}
    
protected:

    virtual int allocate_buffers();
    virtual int reallocate_buffers(int new_getsize, int new_putsize);
    virtual int underflow();
    virtual int overflow(int c = EOF);
    virtual int flush_output();

    virtual void init_inflation() throw(ZlibBufferException);
    virtual void reset_inflation() throw(ZlibBufferException);
    virtual int consume_and_inflate();
    virtual int do_consume_and_inflate(int nbytes);
    virtual int consume(guint8 *buf, int nbytes);
    virtual int do_consume(guint8 *buf, int nbytes);
    virtual GByteArray *inflate(guint8 *in_buffer, int nbytes);
    virtual GByteArray *do_inflate(guint8 *in_buffer, int nbytes);
    virtual int copy_to_get(guint8 *data, int nbytes);
    virtual int do_copy_to_get(guint8 *data, int nbytes);

    URIHandle *_urihandle;

private:

    ZlibBuffer& operator=(ZlibBuffer const& rhs);
    ZlibBuffer(ZlibBuffer const& rhs);

    z_stream _zs;
    int _putsize, _getsize;//sizes of in and out buffers
    
};

class izstream : public std::istream {
public:

    explicit izstream(std::streambuf& sb) : std::istream(&sb) {}
    ~izstream() { std::ios::init(0); }
    
    std::streambuf *rdbuf() { return std::ios::rdbuf(); }
    std::streambuf *operator ->() { return rdbuf(); }

};

}

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
