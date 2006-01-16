#ifndef __INKSCAPE_IO_URISTREAM_H__
#define __INKSCAPE_IO_URISTREAM_H__
/**
 * This should be the only way that we provide sources/sinks
 * to any input/output stream.
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Inkscape.org
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include <uri.h>

#include "inkscapestream.h"


namespace Inkscape
{
namespace IO
{

//#########################################################################
//# U R I    I N P U T    S T R E A M   /   R E A D E R
//#########################################################################

/**
 * This class is for receiving a stream of data from a resource
 * defined in a URI
 */
class UriInputStream : public InputStream
{

public:
    UriInputStream(FILE *source, Inkscape::URI &uri) throw(StreamException);

    UriInputStream(Inkscape::URI &source) throw(StreamException);

    virtual ~UriInputStream() throw(StreamException);

    virtual int available() throw(StreamException);

    virtual void close() throw(StreamException);

    virtual int get() throw(StreamException);

private:

    bool closed;

    FILE *inf;           //for file: uris
    unsigned char *data; //for data: uris
    int dataPos;         //  current read position in data field
    int dataLen;         //  length of data buffer

    Inkscape::URI &uri;

    int scheme;

}; // class UriInputStream




/**
 * This class is for receiving a stream of formatted data from a resource
 * defined in a URI
 */
class UriReader : public BasicReader
{

public:

    UriReader(Inkscape::URI &source) throw(StreamException);

    virtual ~UriReader() throw(StreamException);

    virtual int available() throw(StreamException);

    virtual void close() throw(StreamException);

    virtual gunichar get() throw(StreamException);

private:

    UriInputStream *inputStream;

}; // class UriReader



//#########################################################################
//# U R I    O U T P U T    S T R E A M    /    W R I T E R
//#########################################################################

/**
 * This class is for sending a stream to a destination resource
 * defined in a URI
 *
 */
class UriOutputStream : public OutputStream
{

public:

    UriOutputStream(FILE *fp, Inkscape::URI &destination) throw(StreamException);

    UriOutputStream(Inkscape::URI &destination) throw(StreamException);

    virtual ~UriOutputStream() throw(StreamException);

    virtual void close() throw(StreamException);

    virtual void flush() throw(StreamException);

    virtual void put(int ch) throw(StreamException);

private:

    bool closed;
    bool ownsFile;

    FILE *outf;         //for file: uris
    Glib::ustring data; //for data: uris

    Inkscape::URI &uri;

    int scheme;

}; // class UriOutputStream





/**
 * This class is for sending a stream of formatted data to a resource
 * defined in a URI
 */
class UriWriter : public BasicWriter
{

public:

    UriWriter(Inkscape::URI &source) throw(StreamException);

    virtual ~UriWriter() throw(StreamException);

    virtual void close() throw(StreamException);

    virtual void flush() throw(StreamException);

    virtual void put(gunichar ch) throw(StreamException);

private:

    UriOutputStream *outputStream;

}; // class UriReader






} // namespace IO
} // namespace Inkscape


#endif /* __INKSCAPE_IO_URISTREAM_H__ */
