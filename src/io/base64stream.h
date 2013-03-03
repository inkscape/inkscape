#ifndef SEEN_INKSCAPE_IO_BASE64STREAM_H
#define SEEN_INKSCAPE_IO_BASE64STREAM_H

/**
 * @file
 * Base64-enabled input and output streams
 *
 * This class allows easy encoding and decoding
 * of Base64 data with a stream interface, hiding
 * the implementation from the user.
 */
/*
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Inkscape.org
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include "inkscapestream.h"



namespace Inkscape
{
namespace IO
{

//#########################################################################
//# B A S E 6 4     I N P U T    S T R E A M
//#########################################################################

/**
 * This class is for decoding a Base-64 encoded InputStream source
 *
 */
class Base64InputStream : public BasicInputStream
{

public:

    Base64InputStream(InputStream &sourceStream);
    
    virtual ~Base64InputStream();
    
    virtual int available();
    
    virtual void close();
    
    virtual int get();
    
private:

    int outBytes[3];

    int outCount;

    int padCount;

    bool done;

}; // class Base64InputStream




//#########################################################################
//# B A S E 6 4   O U T P U T    S T R E A M
//#########################################################################

/**
 * This class is for Base-64 encoding data going to the
 * destination OutputStream
 *
 */
class Base64OutputStream : public BasicOutputStream
{

public:

    Base64OutputStream(OutputStream &destinationStream);
    
    virtual ~Base64OutputStream();
    
    virtual void close();
    
    virtual void flush();
    
    virtual int put(gunichar ch);

    /**
     * Sets the maximum line length for base64 output.  If
     * set to <=0, then there will be no line breaks;
     */
    virtual void setColumnWidth(int val)
        { columnWidth = val; }

private:

    void putCh(int ch);

    int column;

    int columnWidth;

    unsigned long outBuf;

    int bitCount;

}; // class Base64OutputStream







} // namespace IO
} // namespace Inkscape


#endif // SEEN_INKSCAPE_IO_BASE64STREAM_H
