#ifndef __STREAMS_JAR_H_
#define __STREAMS_JAR_H_

#include "streams-zlib.h"

namespace Inkscape {

//#define DEBUG_STREAMS 1;

class JarHeaderException 
{
public:
    const char *what() const throw() { return "Invalid file header in jar"; }
};

/**
 * JarBuffer
 */

class JarBuffer : public ZlibBuffer
{
public:
    
    JarBuffer(URIHandle& urih) //throws JarHeaderException
	: ZlibBuffer(urih), compressed_size(0), compressed_left(0), method(0),
	  flags(0)
    { consume_header(); } 
    virtual ~JarBuffer() {}
    
protected:

    virtual void consume_header() throw(JarHeaderException);
    virtual void check_signature(guint8 *data) throw(JarHeaderException);
    virtual unsigned int get_compressed_size() const { return compressed_size; }
    virtual unsigned int get_compressed_left() const { return compressed_left; }
    virtual GByteArray *inflate(guint8 *data, int nbytes);
    virtual int consume_and_inflate();
    virtual void reset();
    virtual bool is_compressed() const { return (method == 8 || flags & 0x0008);}
    virtual int consume_compressed(int nbytes);
    virtual int consume_uncompressed(int nbytes);
    guint32 unpack_4bytes(guint8 *data, const int offset);
    guint16 unpack_2bytes(guint8 *data, const int offset);

private:
    
    JarBuffer& operator=(JarBuffer const& rhs);
    JarBuffer(JarBuffer const& rhs);

    guint32 compressed_size;
    guint32 compressed_left;
    guint16 method;
    guint16 flags;
    guint16 eflen;
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
