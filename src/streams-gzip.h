/*
 * IO layer : gzip streambuf and streams
 *
 * Authors:
 *   Johan Ceuppens <jceuppen at easynet dot be>
 *
 * Copyright (C) 2004 Johan Ceuppens
 *
 * Released under GNU LGPL, read the file 'COPYING.LIB' for more information
 */

#ifndef __STREAMS_GZIP_H_
#define __STREAMS_GZIP_H_

#include "streams-zlib.h"

namespace Inkscape {

class GZipHeaderException : public ZlibBufferException
{
public:
    const char *what() const throw() { return "Invalid gzip file"; }
};

/**
 * GZipBuffer
 */

class GZipBuffer : public ZlibBuffer
{
public:
    
    GZipBuffer(URIHandle& urih) //throws GZipHeaderException
	: ZlibBuffer(urih)
    { consume_header(); } 
    ~GZipBuffer() {}
    
private:
    
    void consume_header() throw(GZipHeaderException);
    void check_signature(guint8 *data) throw(GZipHeaderException);
    void check_flags(guint8 *data) throw(GZipHeaderException);
    gchar *get_filename();
    gchar *get_comment();
    guint16 get_crc();
    void get_extrafield();
    gchar *read_string() throw(GZipHeaderException);
    
    GZipBuffer& operator=(GZipBuffer const& rhs);
    GZipBuffer(GZipBuffer const& rhs);

};

} // namespace Inkscape
#endif // header guard

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
