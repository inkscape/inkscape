/*
 * IO layer : handles for URIs
 *
 * Authors:
 *   Johan Ceuppens <jceuppen at easynet dot be>
 *
 * Copyright (C) 2004 Johan Ceuppens
 *
 * Released under GNU LGPL, read the file 'COPYING.LIB' for more information
 */

#include "streams-handles.h"
#include "uri.h"

#include <iostream>

namespace Inkscape {

/**
 * FileHandle
 */

int FileHandle::open(URI const& uri, char const* mode)
{
    if (sys_open(uri, mode) == 0)
	return 0;
    else
	return 1;
}

FILE *FileHandle::sys_open(URI const& uri, char const* mode)
{    
    gchar *filename = uri.toNativeFilename();

    if ((fp = std::fopen(filename, mode)) == 0) {
	error("fopen");
    }
#ifdef DEBUG_STREAMS
    std::cout<<"file opened fp="<<fp<<std::endl;
#endif
    return fp;
}

void FileHandle::close()
{
    sys_close();
}

void FileHandle::sys_close()
{
    fclose(fp);
}

int FileHandle::read(void *buf, int buflen)
{
    return sys_read(buf, buflen);
}

int FileHandle::sys_read (void *buf, int buflen) throw(ReadException)
{
    int nbytes = 0;
    if ((nbytes = std::fread(buf, 1, buflen, fp)) < 0) {
	if (ferror(fp)) {
	    error("fread");
	    throw ReadException();
	}
    }
    if (nbytes == 0)
	return EOF;
    else
	return nbytes;
}

int FileHandle::write (void const *buf, int buflen)
{
    return sys_write(buf, buflen);
}

int FileHandle::sys_write (void const *buf, int buflen) throw(WriteException)
{
    int nbytes = 0;
    if ((nbytes = std::fwrite(buf, 1, buflen, fp)) < 0) {
	error("fwrite");
	throw WriteException();
    }
    
    return nbytes;    
}

int FileHandle::seek(long offset, int whence)
{
    return sys_seek(offset, whence);
}

int FileHandle::sys_seek(long offset, int whence)
{
    int result;
    if ((result = fseek(fp, offset, whence)) < 0) {
	error("fseek");
    }
    return result;
}
void FileHandle::error(char const *errstr)
{
    std::cerr<<"error FileHandle: "<<errstr<<std::endl;
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
