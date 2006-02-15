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

#include "streams-zlib.h"

namespace Inkscape {

/**
 * ZlibBuffer
 */

ZlibBuffer::ZlibBuffer(URIHandle& urih)
  : _urihandle(&urih), _putsize(BUFSIZE_STREAM), _getsize(BUFSIZE_STREAM)
{
    init_inflation();
}

int ZlibBuffer::allocate_buffers()
{
    if (!eback()) {
	char *buf = new char[_getsize + _putsize];
	setg(buf, buf , buf);
	buf += _getsize;
	setp(buf, buf + _putsize);
	return 1;
    }
    return 0;
}

int ZlibBuffer::reallocate_buffers(int new_getsize, int new_putsize)
{
    char *new_buffer = new char[new_getsize + new_putsize];

    std::memcpy(new_buffer, eback(), _getsize);
    std::memcpy(new_buffer, eback() + _getsize, _putsize);

    setg(new_buffer, new_buffer + (gptr() - eback()),
	 new_buffer + new_getsize);
    new_buffer += new_getsize;
    setp(new_buffer, new_buffer + new_putsize);

    _getsize = new_getsize;
    _putsize = new_putsize;

    return 1;
}

int ZlibBuffer::underflow()
{
    if (eback() == 0 && allocate_buffers() == 0)
    	return EOF;

    if (consume_and_inflate() == EOF)
	return EOF;

    return *(unsigned char *)gptr();
}

int ZlibBuffer::overflow(int c)
{
    if (c == EOF)
	return flush_output();

    if (pbase() == 0 && allocate_buffers() == 0)
	return EOF;

    if (pptr() >= epptr() &&
	flush_output() == EOF)
	return EOF;

    putchar(c);

    if (pptr() >= epptr() &&
	flush_output() == EOF)
	return EOF;

    return c;
}

int ZlibBuffer::consume(guint8 *buf, int nbytes)
{
    return do_consume(buf, nbytes);
}

int ZlibBuffer::do_consume(guint8 *buf, int nbytes)
{
    nbytes = _urihandle->read(buf, nbytes);

    if (nbytes == EOF)
	return EOF;
    else if (nbytes == 0)
	return EOF;

    return nbytes;
}

int ZlibBuffer::do_consume_and_inflate(int nbytes)
{
    guint8 buf[nbytes];
    if (consume(buf, nbytes) == EOF)
	return EOF;

    GByteArray *gba = inflate(buf, nbytes);
    copy_to_get(gba->data, gba->len);

    g_byte_array_free(gba, TRUE);
    return 1;
}

int ZlibBuffer::consume_and_inflate()
{
    return do_consume_and_inflate(BUFSIZE_STREAM);
}

int ZlibBuffer::flush_output()
{
    if (pptr() <= pbase())
	return 0;
    int len = pptr() - pbase();
    int nbytes = _urihandle->write(pbase(), len);
    setp(pbase(), pbase() + BUFSIZE_STREAM);
    if (len == nbytes)
	return 0;
    else
	return EOF;
}

void ZlibBuffer::init_inflation() throw(ZlibBufferException)
{
    memset(&_zs, 0, sizeof(z_stream));

    _zs.zalloc = Z_NULL;
    _zs.zfree = Z_NULL;
    _zs.opaque = Z_NULL;

    if(inflateInit2(&_zs, -15) != Z_OK) {
	throw ZlibBufferException();
    }

}

void ZlibBuffer::reset_inflation() throw(ZlibBufferException)
{
    if (inflateReset(&_zs) != Z_OK)
	throw ZlibBufferException();
}

GByteArray *ZlibBuffer::inflate(guint8 *in_buffer, int nbytes)
{
    return do_inflate(in_buffer, nbytes);
}

GByteArray *ZlibBuffer::do_inflate(guint8 *data, int nbytes)
{
    GByteArray *gba = g_byte_array_new();
    guint8 out_buffer[BUFSIZE_STREAM];

    _zs.avail_in = 0;
    guint32 crc = crc32(0, Z_NULL, 0);

    if (!_zs.avail_in) {
	_zs.avail_in = nbytes;
	_zs.next_in = (Bytef *)data;
	crc = crc32(crc, (Bytef *)data, _zs.avail_in);
    }
    do {
	_zs.next_out = out_buffer;
	_zs.avail_out = BUFSIZE_STREAM;

	int ret = ::inflate(&_zs, Z_NO_FLUSH);
	if (BUFSIZE_STREAM != _zs.avail_out) {
	    unsigned int tmp_len = BUFSIZE_STREAM - _zs.avail_out;
	    g_byte_array_append(gba, out_buffer, tmp_len);
	}
	
	if (ret == Z_STREAM_END) {
	    break;
	}
	if (ret != Z_OK) {
	    std::fprintf(stderr, "decompression error %d\n", ret);
	    break;
	}
    } while (_zs.avail_in);

    return gba;
}

int ZlibBuffer::copy_to_get(guint8 *data, int nbytes)
{
    return do_copy_to_get(data, nbytes);
}

int ZlibBuffer::do_copy_to_get(guint8 *data, int nbytes)
{
    if (nbytes + gptr() - eback() > _getsize)
	reallocate_buffers(nbytes + gptr() - eback() + BUFSIZE_STREAM,
			   _putsize);

    std::memcpy(gptr(), data, nbytes);
    setg(eback(), gptr(), gptr() + nbytes);
    return 1;
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
