#ifndef __INKJAR_JAR_H_
#define __INKJAR_JAR_H_
/*
 * Copyright (C) 1999  Bryan Burns
 * Copyright (C) 2004 Johan Ceuppens
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if defined(WIN32) || defined(__WIN32__)
# include <zlib.h>
#endif

#ifdef HAVE_ZLIB_H
# include <zlib.h>
#endif

#include <stdint.h>

#include <glib.h>
#include <cstdio>

namespace Inkjar {

unsigned const RDSZ  = 4096;

//#define DEBUG 1 //uncommment for debug messages

enum JarFileReaderState {CLOSED, OPEN};

//fixme: The following will be removed
typedef uint8_t ub1;
typedef uint16_t ub2;
typedef uint32_t ub4;

#define LOC_EXTRA   6  /* extra bytes */
#define LOC_COMP    8  /* compression method */
#define LOC_MODTIME 10 /* last modification time */
#define LOC_MODDATE 12 /* last modification date */
#define LOC_CRC     14 /* CRC */
#define LOC_CSIZE   18 /* compressed size */
#define LOC_USIZE   22 /* uncompressed size */
#define LOC_FNLEN   26 /* filename length */
#define LOC_EFLEN   28 /* extra-field length */

#define CEN_COMP    10 /* compression method */
#define CEN_MODTIME 12
#define CEN_MODDATE 14
#define CEN_CRC     16
#define CEN_CSIZE   20
#define CEN_USIZE   24
#define CEN_FNLEN   28
#define CEN_EFLEN   30
#define CEN_COMLEN  32
#define CEN_OFFSET  42


/* macros */
#define PACK_UB4(d, o, v) d[o] = (ub1)((v) & 0x000000ff); \
                          d[o + 1] = (ub1)(((v) & 0x0000ff00) >> 8); \
                          d[o + 2] = (ub1)(((v) & 0x00ff0000) >> 16); \
                          d[o + 3] = (ub1)(((v) & 0xff000000) >> 24)

#define PACK_UB2(d, o, v) d[o] = (ub1)((v) & 0x00ff); \
                          d[o + 1] = (ub1)(((v) & 0xff00) >> 8)

#define UNPACK_UB4(s, o) (ub4)s[o] + (((ub4)s[o + 1]) << 8) +\
                         (((ub4)s[o + 2]) << 16) + (((ub4)s[o + 3]) << 24)

#define UNPACK_UB2(s, o)  (ub2)s[o] + (((ub2)s[o + 1]) << 8)



/*
 * JarFile:
 * 
 * This is a wrapper class for canonical jarfile functions like reading, 
 * writing, seeking etc. JarFile is a dumb class with no state information.
 *
 * All memory allocations are done with g_malloc.
 */
class JarFile {
public:

    JarFile() : _file(NULL), _filename(NULL), _last_filename(NULL) {}
    virtual ~JarFile();
    JarFile(gchar const *new_filename);
    
    GByteArray *get_next_file_contents();
    gchar *get_last_filename() const;
    bool open();
    bool close();
    int read(guint8 *buf, unsigned int count);

    JarFile(JarFile const &rhs);
    JarFile &operator=(JarFile const &rhs);

private:

    FILE  *_file; // File descriptor
    gchar *_filename;
    z_stream _zs;
    gchar *_last_filename;

    bool init_inflation();
    bool read_signature();
    guint32 get_crc(guint8 *bytes, guint16 flags);
    guint8 *read_filename(guint16 filename_length);
    bool check_compression_method(guint16 method, guint16 flags);
    bool check_crc(guint32 oldcrc, guint32 crc, guint16 flags);
    guint8 *get_compressed_file(guint32 compressed_size,
				unsigned int &file_length,
				guint32 oldcrc, guint16 flags);
    guint8 *get_uncompressed_file(guint32 compressed_szie, guint32 crc, 
				  guint16 eflen, guint16 flags);
}; // class JarFile


/*
 * JarFileReader:
 *
 * This provides some smarter functions for operating on a jarfile object
 * It should be able to grep for files or return the contents of a specific 
 * file.
 */

class JarFileReader {
public:
    
    JarFileReader(gchar const *new_filename) 
	: _state(CLOSED), _jarfile(new_filename) {}
    JarFileReader() : _state(CLOSED) {}
    virtual ~JarFileReader() { if (_state == OPEN) _jarfile.close(); }
    //fixme return types are incorrect
    GByteArray *get_next_file();//fixme clean up return type
    void set_filename(gchar const *new_filename);
    void set_jarfile(JarFile const &new_jarfile);
    gchar *get_last_filename() const { return _jarfile.get_last_filename(); };
    JarFileReader(JarFileReader const &rhs);
    JarFileReader &operator=(JarFileReader const &rhs);
private:
    JarFileReaderState _state;    
    JarFile _jarfile;

}; // class JarFileReader

} // namespace Inkjar
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
