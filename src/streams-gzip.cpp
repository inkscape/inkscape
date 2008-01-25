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

#include <string.h>

#include "streams-gzip.h"

namespace Inkscape {

//With some inpsiration and code from libgsf, fastjar, libpng and RFC 1952

static int const GZIP_IS_ASCII	     = 0x01; //file contains text
static int const GZIP_HEADER_CRC     = 0x02; //there is a CRC in the header
static int const GZIP_EXTRA_FIELD    = 0x04; //there is an 'extra' field
static int const GZIP_ORIGINAL_NAME  = 0x08; //the original is stored
static int const GZIP_HAS_COMMENT    = 0x10; //There is a comment in the header
static unsigned int const GZIP_HEADER_FLAGS = (GZIP_IS_ASCII
					       |GZIP_HEADER_CRC
					       |GZIP_EXTRA_FIELD
					       |GZIP_ORIGINAL_NAME
					       |GZIP_HAS_COMMENT);

/**
 * GZipBuffer
 */

void GZipBuffer::consume_header() throw(GZipHeaderException)
{
    unsigned int flags;
    guint8 data[4];

    try {
	_urihandle->read(data, 4);
	check_signature(data);
	check_flags(data);
	flags = data[3];
	_urihandle->read(data, 4);
	//get_modification_time()
	_urihandle->read(data, 1);
	//check_extra_flags();
	_urihandle->read(data, 1);
	//check_OS();
	
	if (flags & GZIP_EXTRA_FIELD) {
	    get_extrafield();
	}
	if (flags & GZIP_ORIGINAL_NAME) {
	    get_filename();
	}
	if (flags & GZIP_HAS_COMMENT) {
	    get_comment();
	}
	if (flags & GZIP_HEADER_CRC) {
	    get_crc();
	}
    }
    catch(std::exception& e) {
	throw GZipHeaderException();
    }
}

void GZipBuffer::check_signature(guint8 *data) throw(GZipHeaderException)
{
    guint8 const signature[2] = {0x1f, 0x8b};
    if (memcmp(data, signature, sizeof(signature)) != 0)
	throw GZipHeaderException();
}

void GZipBuffer::check_flags(guint8 *data) throw(GZipHeaderException)
{
    unsigned int flags = data[3];
    if (data[2] != Z_DEFLATED || (flags & ~GZIP_HEADER_FLAGS) != 0)
	throw GZipHeaderException();
}

gchar *GZipBuffer::get_filename()
{
#ifdef DEBUG_STREAMS
    std::cout<<"Filename is ";
#endif
    return read_string();
}

gchar *GZipBuffer::get_comment()
{
#ifdef DEBUG_STREAMS
    std::cout<<"Comment is "<<std::endl;
#endif
    return read_string();
}

guint16 GZipBuffer::get_crc()
{
    guint16 buf;
    _urihandle->read(&buf, 2);
    return buf;
}

void GZipBuffer::get_extrafield()
{
    guint8 length_data[2];
    _urihandle->read(length_data, 2);
    unsigned int const length = length_data[0] | (length_data[1] << 8);
    guint8 *data = new guint8[length];
    _urihandle->read(data, length);
}

gchar *GZipBuffer::read_string() throw(GZipHeaderException)
{
    GByteArray *gba = g_byte_array_new();
    try {
	guint8 byte[1];
	do {
	    _urihandle->read(byte, 1);
	    g_byte_array_append(gba, byte, sizeof(byte));
#ifdef DEBUG_STREAMS
	    std::cout <<(char)*byte;
#endif
	} while (*byte != 0);
    } catch (std::exception& e) {
	g_byte_array_free(gba, TRUE);
	throw GZipHeaderException();
    }
#ifdef DEBUG_STREAMS
    std::cout<<std::endl;
#endif
    gchar *ret = (gchar *)gba->data;
    g_byte_array_free(gba, FALSE);
    return ret;
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
