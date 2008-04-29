#ifndef __URISTREAM_H__
#define __URISTREAM_H__

/**
 * Phoebe DOM Implementation.
 *
 * This is a C++ approximation of the W3C DOM model, which follows
 * fairly closely the specifications in the various .idl files, copies of
 * which are provided for reference.  Most important is this one:
 *
 * http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/idl-definitions.html
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2005-2008 Bob Jamison
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * This should be the only way that we provide sources/sinks
 * to any input/output stream.
 *
 */


#include "../uri.h"
#include "domstream.h"
#include "httpclient.h"


namespace org
{
namespace w3c
{
namespace dom
{
namespace io
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

    UriInputStream(const URI &source) throw(StreamException);

    virtual ~UriInputStream() throw(StreamException);

    virtual int available() throw(StreamException);

    virtual void close() throw(StreamException);

    virtual int get() throw(StreamException);

private:

    void init() throw(StreamException);//common code called by constructor

    bool closed;

    FILE *inf;           //for file: uris
    DOMString data;      //for data: uris
    int dataPos;         //  current read position in data field
    int dataLen;         //  length of data buffer

    URI uri;

    int scheme;

    HttpClient httpClient;

}; // class UriInputStream




/**
 * This class is for receiving a stream of formatted data from a resource
 * defined in a URI
 */
class UriReader : public Reader
{

public:

    UriReader(const URI &source) throw(StreamException);

    virtual ~UriReader() throw(StreamException);

    virtual int available() throw(StreamException);

    virtual void close() throw(StreamException);

    virtual int get() throw(StreamException);

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

    UriOutputStream(const URI &destination) throw(StreamException);

    virtual ~UriOutputStream() throw(StreamException);

    virtual void close() throw(StreamException);

    virtual void flush() throw(StreamException);

    virtual int put(XMLCh ch) throw(StreamException);

private:

    void init() throw(StreamException); //common code called by constructor

    bool closed;
    bool ownsFile;

    FILE *outf;     //for file: uris
    DOMString data; //for data: uris

    URI uri;

    int scheme;

    HttpClient httpClient;

}; // class UriOutputStream





/**
 * This class is for sending a stream of formatted data to a resource
 * defined in a URI
 */
class UriWriter : public Writer
{

public:

    UriWriter(const URI &source) throw(StreamException);

    virtual ~UriWriter() throw(StreamException);

    virtual void close() throw(StreamException);

    virtual void flush() throw(StreamException);

    virtual int put(XMLCh ch) throw(StreamException);

private:

    UriOutputStream *outputStream;

}; // class UriReader






}  //namespace io
}  //namespace dom
}  //namespace w3c
}  //namespace org

/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/


#endif /* __URISTREAM_H__ */
