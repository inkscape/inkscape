#include <vector>
#include "streams-jar.h"

namespace Inkscape {

const int LOC_EXTRA = 6;  // extra bytes
const int LOC_COMP  = 8;  // compression method
const int LOC_CSIZE = 18; // compressed size
const int LOC_FNLEN = 26; // filename length
const int LOC_EFLEN = 28; // extra-field length

void JarBuffer::consume_header() throw(JarHeaderException)
{
    try {
	guint8 data[30];
	_urihandle->read(data, 4);
    	check_signature(data);
	_urihandle->read(data+4, 26);
	compressed_size = compressed_left = unpack_4bytes(data, LOC_CSIZE);
	eflen = unpack_2bytes(data, LOC_EFLEN);
	flags = unpack_2bytes(data, LOC_EXTRA);
	method = unpack_2bytes(data, LOC_COMP);
	
#ifdef DEBUG_STREAMS
	std::printf("Compressed size is %u\n", compressed_size);
	std::printf("Extra field length is %hu\n", eflen);
	std::printf("Flags are %#hx\n", flags);
	std::printf("Compression method is %#hx\n", method);
#endif
    }
    catch (std::exception& e) {
	throw JarHeaderException();
    }
}

void JarBuffer::check_signature(guint8 *data) throw(JarHeaderException)
{
    guint32 signature = unpack_4bytes(data, 0);

#ifdef DEBUG_STREAMS
    std::printf("signature is %x\n", signature);
#endif

    if (signature == 0x08074b50) {
	_urihandle->read(data, 12);
    } else if (signature != 0x02014b50 && signature != 0x04034b50) {
	throw JarHeaderException();
    }
}

void JarBuffer::reset()//resets zlib and buffer (also skips archived directories)
{
    bool do_reset = false;
    while (compressed_left == 0) {
	consume_header();
	do_reset = true;
    }

    if (do_reset) {
	reset_inflation();
	setg(eback(), eback(), eback());
    }
}

int JarBuffer::consume_and_inflate()
{
    int nbytes;

    reset();

    nbytes = compressed_left > BUFSIZE_STREAM ? BUFSIZE_STREAM
	    : compressed_left;

    if (is_compressed())
	return consume_compressed(nbytes);
    else
	return consume_uncompressed(nbytes);
}

int JarBuffer::consume_compressed(int nbytes)
{
    int ret=do_consume_and_inflate(nbytes);

    if ( ret == EOF && eflen > 0 ) {
        std::vector<guint8> efbuf(eflen);
	_urihandle->read(&efbuf[0], eflen);
	return 1;
    }

    return ret;
}

int JarBuffer::consume_uncompressed(int nbytes)
{
    std::vector<guint8> data(nbytes);
    int consumed=consume(&data[0], nbytes);
    if ( consumed != EOF ) {
        copy_to_get(&data[0], consumed);
        compressed_left -= consumed;
    }
    return consumed;
}

GByteArray *JarBuffer::inflate(guint8 *data, const int nbytes)
{
    GByteArray *gba = do_inflate(data, nbytes);
    compressed_left -= nbytes;
    return gba;
}

guint32 JarBuffer::unpack_4bytes(guint8 *data, const int offset)
{
    return ((guint32)data[offset]
	    + (((guint32)data[offset + 1]) << 8)
	    + (((guint32)data[offset + 2]) << 16)
	    + (((guint32)data[offset + 3]) << 24));
}

guint16 JarBuffer::unpack_2bytes(guint8 *data, int offset)
{
    return ((guint16)data[offset] + (((guint16)data[offset + 1]) << 8));
}

} // namespace Inkscape

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
