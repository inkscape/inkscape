/*
 * Copyright (C) 1999, 2000  Bryan Burns
 * Copyright (C) 2004 Johan Ceuppens
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/*
 * TODO/FIXME: 
 * - configure #ifdefs should be enabled
 * - move to cstdlib instead of stdlib.h etc.
 * - remove exit functions
 * - move to clean C++ code
 * - windowsify
 * - remove a few g_free/g_mallocs
 * - unseekable files
 * - move to LGPL by rewriting macros
 * - crcs for compressed files
 * - put in eof
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif


//#ifdef STDC_HEADERS
//#endif

//#ifdef HAVE_UNISTD_H
//#endif

//#ifdef HAVE_SYS_PARAM_H
//#else
//#define MAXPATHLEN 1024
//#endif

//#ifdef HAVE_DIRENT_H
//#endif

//#ifdef HAVE_FCNTL_H
#include <fcntl.h>
//#endif

#include <cstring>
#include <string>
#include <cstdlib>
#include <glib.h>
#include <zlib.h>

#include "inkjar.h"

#include <fstream>
#ifdef WORDS_BIGENDIAN

#define L2BI(l) ((l & 0xff000000) >> 24) | \
((l & 0x00ff0000) >> 8)  | \
((l & 0x0000ff00) << 8)  | \
((l & 0x000000ff) << 24);

#define L2BS(l) ((l & 0xff00) >> 8) | ((l & 0x00ff) << 8);

#endif

namespace Inkjar {

JarFile::JarFile(gchar const*new_filename) :
    _file(NULL),
    _filename(g_strdup(new_filename)),
    _last_filename(NULL)
{}

//fixme: the following should probably just return a const gchar* and not 
//       use strdup
gchar *JarFile::get_last_filename() const
{
    return (_last_filename != NULL ? g_strdup(_last_filename) : NULL);
}

JarFile::~JarFile()
{ 
    if (_filename != NULL)
	g_free(_filename); 
    if (_last_filename != NULL)
	g_free(_last_filename); 
}

bool JarFile::init_inflation()
{
    memset(&_zs, 0, sizeof(z_stream));
    
    _zs.zalloc = Z_NULL;
    _zs.zfree = Z_NULL;
    _zs.opaque = Z_NULL;
    
    if(inflateInit2(&_zs, -15) != Z_OK) {
	fprintf(stderr,"error initializing inflation!\n");
	return false;
    }

    return true;
}

bool JarFile::open()
{
    if (_file != NULL) {
        fclose(_file);
    }
    if ((_file = fopen(_filename, "r")) == NULL) {
        fprintf(stderr, "open failed.\n");
        return false;
    }
    if (!init_inflation()) {
        return false;
    }
    else {
        return true;
    }
}

bool JarFile::close()
{
    if (_file != NULL && (fclose(_file) == 0)) {
	inflateEnd(&_zs);
	return true;
    }
    return false;
}

bool JarFile::read_signature()
{
    guint8 *bytes = (guint8 *)g_malloc(sizeof(guint8) * 4);
    if (!read(bytes, 4)) {
	g_free(bytes);
	return false;
    }

    guint32 signature = UNPACK_UB4(bytes, 0);
    g_free(bytes);

#ifdef DEBUG
    std::printf("signature is %x\n", signature);
#endif

    if (signature == 0x08074b50) {
    //skip data descriptor
        bytes = (guint8 *)g_malloc(sizeof(guint8) * 12);
        if (!read(bytes, 12)) {
            g_free(bytes);
            return false;
        } else {
            g_free(bytes);
        }
    } else if (signature == 0x02014b50 || signature == 0x04034b50) {
        return true;
    } else {
        return false;
    }
    return false;
}

guint32 JarFile::get_crc(guint8 *bytes, guint16 flags)
{
    guint32 crc = 0;
    //no data descriptor
    if (!(flags & 0x0008)) {
	crc = UNPACK_UB4(bytes, LOC_CRC);
	
#ifdef DEBUG
	std::printf("CRC from file is %x\n", crc);
#endif
    }
    
    return crc;
}

guint8 *JarFile::read_filename(guint16 filename_length)
{
    guint8 *filename = (guint8 *)g_malloc(sizeof(guint8)
					  * (filename_length+1));
    if (!read(filename, filename_length)) {
	g_free(filename);
	return NULL;
    }
    filename[filename_length] = '\0';

#ifdef DEBUG
    std::printf("Filename is %s\n", filename);
#endif

    return filename;
}

bool JarFile::check_compression_method(guint16 method, guint16 flags)
{
    return !(method != 8 && flags & 0x0008);
}

GByteArray *JarFile::get_next_file_contents()
{
    guint8 *bytes;
    GByteArray *gba = g_byte_array_new();

    read_signature();
    
    //get compressed size
    bytes = (guint8 *)g_malloc(sizeof(guint8) * 30);
    if (!read(bytes+4, 26)) {
	g_free(bytes);
	return NULL;
    }
    guint32 compressed_size = UNPACK_UB4(bytes, LOC_CSIZE);
    guint16 filename_length = UNPACK_UB2(bytes, LOC_FNLEN);
    guint16 eflen = UNPACK_UB2(bytes, LOC_EFLEN);
    guint16 flags = UNPACK_UB2(bytes, LOC_EXTRA);
    guint16 method = UNPACK_UB2(bytes, LOC_COMP);

    if (filename_length == 0) {
	g_byte_array_free(gba, TRUE);
	if (_last_filename != NULL)
	    g_free(_last_filename);
	_last_filename = NULL;
	g_free(bytes);
	return NULL;
    }


#ifdef DEBUG    
    std::printf("Compressed size is %u\n", compressed_size);
    std::printf("Filename length is %hu\n", filename_length);
    std::printf("Extra field length is %hu\n", eflen);
    std::printf("Flags are %#hx\n", flags);
    std::printf("Compression method is %#hx\n", method);
#endif
    
    guint32 crc = get_crc(bytes, flags);
    
    gchar *filename = (gchar *)read_filename(filename_length);
    g_free(bytes);
    
    if (filename == NULL) 
	return NULL;
   
    if (_last_filename != NULL)
	g_free(_last_filename);
    _last_filename = filename;

    //check if this is a directory and skip
    
    char *c_ptr;
    if ((c_ptr = std::strrchr(filename, '/')) != NULL) {
	if (*(++c_ptr) == '\0') {
	    return NULL;
	}
    }
   
    if (!check_compression_method(method, flags)) {
	std::fprintf(stderr, "error in jar file\n");
	return NULL;
    }    
    
    if (method == 8 || flags & 0x0008) {
	unsigned int file_length = 0;//uncompressed file length
	fseek(_file, eflen, SEEK_CUR);
	guint8 *file_data = get_compressed_file(compressed_size, file_length, 
						crc, flags);
	if (file_data == NULL) {
	    g_byte_array_free(gba, FALSE);
	    return NULL;
	}
	g_byte_array_append(gba, file_data, file_length);
    } else if (method == 0) {
	guint8 *file_data = get_uncompressed_file(compressed_size, crc, 
						  eflen, flags); 

	if (file_data == NULL) {
	    g_byte_array_free(gba, TRUE);
	    return NULL;
	}
	g_byte_array_append(gba, file_data, compressed_size);
    } else {
	fseek(_file, compressed_size+eflen, SEEK_CUR);
	g_byte_array_free(gba, FALSE);
	return NULL;
    }
        
    
    return gba;
}

guint8 *JarFile::get_uncompressed_file(guint32 compressed_size, guint32 crc, 
				       guint16 eflen, guint16 flags)
{
    GByteArray *gba = g_byte_array_new();
    unsigned int out_a = 0;
    unsigned int in_a = compressed_size;
    guint8 *bytes;
    guint32 crc2 = 0;
    
    crc2 = crc32(crc2, NULL, 0);
    
    bytes = (guint8 *)g_malloc(sizeof(guint8) * RDSZ);
    while(out_a < compressed_size){
	unsigned int nbytes = (in_a > RDSZ ? RDSZ : in_a);
	
	if (!(nbytes = read(bytes, nbytes))) {
	    g_free(bytes);
	    return NULL;
	}
	
	crc2 = crc32(crc2, (Bytef*)bytes, nbytes);
	    
	g_byte_array_append (gba, bytes, nbytes);
	out_a += nbytes;
	in_a -= nbytes;
	    
#ifdef DEBUG    
	std::printf("%u bytes written\n", out_a);
#endif
    }
    fseek(_file, eflen, SEEK_CUR);
    g_free(bytes);

    if (!check_crc(crc, crc2, flags)) {
        bytes = gba->data;
        g_byte_array_free(gba, FALSE);//FALSE argument does not free actual data
	return NULL;
    }
    
    return bytes;
}

int JarFile::read(guint8 *buf, unsigned int count)
{
    size_t nbytes;
    if ((nbytes = fread(buf, 1, count, _file)) != count) {
	fprintf(stderr, "read error\n");
	exit(1);
	return 0;
    }
    return nbytes;
}

/* FIXME: this could probably use ZlibBuffer */
guint8 *JarFile::get_compressed_file(guint32 compressed_size, 
				     unsigned int& file_length,
				     guint32 oldcrc, guint16 flags)
{
    if (compressed_size == 0)
	return NULL;
    
    guint8 in_buffer[RDSZ];
    guint8 out_buffer[RDSZ];
    size_t nbytes;
    unsigned int leftover_in = compressed_size;
    GByteArray *gba = g_byte_array_new();
    
    _zs.avail_in = 0;
    guint32 crc = crc32(0, Z_NULL, 0);
    
    do {		
	if (!_zs.avail_in) {
	    nbytes = fread(in_buffer, 1, 
				 (leftover_in < RDSZ ? leftover_in : RDSZ), _file);

            if(ferror(_file) != 0) {
		fprintf(stderr, "jarfile read error");
	    }

	    _zs.avail_in = nbytes;
	    _zs.next_in = in_buffer;
	    crc = crc32(crc, in_buffer, _zs.avail_in);
	    leftover_in -= RDSZ;
	}
	_zs.next_out = out_buffer;
	_zs.avail_out = RDSZ;
	
	int ret = inflate(&_zs, Z_NO_FLUSH);
	if (RDSZ != _zs.avail_out) {
	    unsigned int tmp_len = RDSZ - _zs.avail_out;
	    guint8 *tmp_bytes = (guint8 *)g_malloc(sizeof(guint8) 
						   * tmp_len);
	    memcpy(tmp_bytes, out_buffer, tmp_len);
	    g_byte_array_append(gba, tmp_bytes, tmp_len);
	}
	
	if (ret == Z_STREAM_END) {
	    break;
	}
	if (ret != Z_OK)
	    std::printf("decompression error %d\n", ret);
    } while (_zs.total_in < compressed_size);
    
    file_length = _zs.total_out;
#ifdef DEBUG
    std::printf("done inflating\n");
    std::printf("%d bytes left over\n", _zs.avail_in);
    std::printf("CRC is %x\n", crc);
#endif
    
    guint8 *ret_bytes;
    if (check_crc(oldcrc, crc, flags) && gba->len > 0)
	ret_bytes = gba->data;
    else 
	ret_bytes = NULL;
    g_byte_array_free(gba, FALSE);

    inflateReset(&_zs); 
    return ret_bytes;
}

bool JarFile::check_crc(guint32 oldcrc, guint32 crc, guint16 flags)
{
    //fixme: does not work yet
	
    if(flags & 0x0008) {
	guint8 *bytes = (guint8 *)g_malloc(sizeof(guint8) * 16);
	if (!read(bytes, 16)) {
	    g_free(bytes);
	    return false;
	}
	
	guint32 signature = UNPACK_UB4(bytes, 0);
	g_free(bytes);
	if(signature != 0x08074b50) {
	    fprintf(stderr, "missing data descriptor!\n");
	}
	
	crc = UNPACK_UB4(bytes, 4);
	
    }
    if (oldcrc != crc) {
#ifdef DEBUG
	std::fprintf(stderr, "Error! CRCs do not match! Got %x, expected %x\n",
		     oldcrc, crc);
#endif
    }
    return true;
}

JarFile::JarFile(JarFile const& rhs)
{
    *this = rhs;
}

JarFile& JarFile::operator=(JarFile const& rhs)
{
    if (this == &rhs)
	return *this;

    _zs = rhs._zs;//fixme
    if (_filename == NULL)
	_filename = NULL;
    else
	_filename = g_strdup(rhs._filename);
    if (_last_filename == NULL)
	_last_filename = NULL;
    else 
	_last_filename = g_strdup(rhs._last_filename);
    _file = rhs._file;
    
    return *this;
}


/////////////////////////
//      JarFileReader  //
/////////////////////////

GByteArray *JarFileReader::get_next_file()
{
    if (_state == CLOSED) {
	_jarfile.open();
	_state = OPEN;
    }
    
    return _jarfile.get_next_file_contents();
}

JarFileReader& JarFileReader::operator=(JarFileReader const& rhs)
{
    if (&rhs == this)
	return *this;

    _jarfile = rhs._jarfile;
    _state = rhs._state;
    
    return *this;
}

/*
 * If the filename gets reset, a jarfile object gets generated again,
 * ready to be opened for reading.
 */
void JarFileReader::set_filename(gchar const *new_filename)
{
    _jarfile.close();
    _jarfile = JarFile(new_filename);
}

void JarFileReader::set_jarfile(JarFile const& new_jarfile)
{
    _jarfile = new_jarfile;
}

JarFileReader::JarFileReader(JarFileReader const& rhs)
{
    *this = rhs;
}

} // namespace Inkjar


#if 0 //testing code
#include "jar.h"
/*
 * This program writes all the files from a jarfile to stdout and inflates
 * where needed.
 */
int main(int argc, char *argv[])
{
    gchar *filename;
    if (argc < 2) {
	filename = "./ide.jar\0";
    } else {
	filename = argv[1];
    }

    Inkjar::JarFileReader jar_file_reader(filename);
    
    for (;;) {
	GByteArray *gba = jar_file_reader.get_next_file();
	if (gba == NULL) {
	    char *c_ptr;
	    gchar *last_filename = jar_file_reader.get_last_filename();
	    if (last_filename == NULL)
		break;
	    if ((c_ptr = std::strrchr(last_filename, '/')) != NULL) {
		if (*(++c_ptr) == '\0') {
		    g_free(last_filename);
		    continue;
		}
	    }
	} else if (gba->len > 0)
	    ::write(1, gba->data, gba->len);
	else
	    break;
    }
    return 0;
}
#endif
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
