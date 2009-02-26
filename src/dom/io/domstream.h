#ifndef __DOMSTREAM_H__
#define __DOMSTREAM_H__
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
 * Copyright (C) 2006-2007 Bob Jamison
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


#include <cstdio>
#include <dom/dom.h>

namespace org
{
namespace w3c
{
namespace dom
{
namespace io
{



class StreamException
{
public:

    StreamException(const DOMString &theReason) throw()
        { reason = theReason; }
    virtual ~StreamException() throw()
        {  }
    char const *what()
        { return reason.c_str(); }

private:

    DOMString reason;

};

//#########################################################################
//# I N P U T    S T R E A M
//#########################################################################

/**
 * This interface is the base of all input stream classes.  Users who wish
 * to make an InputStream that is part of a chain should inherit from
 * BasicInputStream.  Inherit from this class to make a source endpoint,
 * such as a URI or buffer.
 *
 */
class InputStream
{

public:

    /**
     * Constructor.
     */
    InputStream() {}

    /**
     * Destructor
     */
    virtual ~InputStream() {}

    /**
     * Return the number of bytes that are currently available
     * to be read
     */
    virtual int available() = 0;

    /**
     * Do whatever it takes to 'close' this input stream
     * The most likely implementation of this method will be
     * for endpoints that use a resource for their data.
     */
    virtual void close() = 0;

    /**
     * Read one byte from this input stream.  This is a blocking
     * call.  If no data is currently available, this call will
     * not return until it exists.  If the user does not want
     * their code to block,  then the usual solution is:
     *     if (available() > 0)
     *         myChar = get();
     * This call returns -1 on end-of-file.
     */
    virtual int get() = 0;

}; // class InputStream




/**
 * This is the class that most users should inherit, to provide
 * their own streams.
 *
 */
class BasicInputStream : public InputStream
{

public:

    BasicInputStream(const InputStream &sourceStream);

    virtual ~BasicInputStream() {}

    virtual int available();

    virtual void close();

    virtual int get();

protected:

    bool closed;

    InputStream &source;

private:


}; // class BasicInputStream



/**
 * Convenience class for reading from standard input
 */
class StdInputStream : public InputStream
{
public:

    int available()
        { return 0; }

    void close()
        { /* do nothing */ }

    int get()
        {  return getchar(); }

};






//#########################################################################
//# O U T P U T    S T R E A M
//#########################################################################

/**
 * This interface is the base of all input stream classes.  Users who wish
 * to make an OutputStream that is part of a chain should inherit from
 * BasicOutputStream.  Inherit from this class to make a destination endpoint,
 * such as a URI or buffer.
 */
class OutputStream
{

public:

    /**
     * Constructor.
     */
    OutputStream() {}

    /**
     * Destructor
     */
    virtual ~OutputStream() {}

    /**
     * This call should
     *  1.  flush itself
     *  2.  close itself
     *  3.  close the destination stream
     */
    virtual void close() = 0;

    /**
     * This call should push any pending data it might have to
     * the destination stream.  It should NOT call flush() on
     * the destination stream.
     */
    virtual void flush() = 0;

    /**
     * Send one byte to the destination stream.
     */
    virtual int put(XMLCh ch) = 0;


}; // class OutputStream


/**
 * This is the class that most users should inherit, to provide
 * their own output streams.
 */
class BasicOutputStream : public OutputStream
{

public:

    BasicOutputStream(const OutputStream &destinationStream);

    virtual ~BasicOutputStream() {}

    virtual void close();

    virtual void flush();

    virtual int put(XMLCh ch);

protected:

    bool closed;

    OutputStream &destination;


}; // class BasicOutputStream



/**
 * Convenience class for writing to standard output
 */
class StdOutputStream : public OutputStream
{
public:

    void close()
        { }

    void flush()
        { }

    int put(XMLCh ch)
        {  putchar(ch); return 1; }

};




//#########################################################################
//# R E A D E R
//#########################################################################


/**
 * This interface and its descendants are for unicode character-oriented input
 *
 */
class Reader
{

public:

    /**
     * Constructor.
     */
    Reader() {}

    /**
     * Destructor
     */
    virtual ~Reader() {}


    virtual int available() = 0;

    virtual void close() = 0;

    virtual int get() = 0;

    virtual DOMString readLine() = 0;

    virtual DOMString readWord() = 0;

    /* Input formatting */
    virtual Reader& readBool (bool& val ) = 0;

    virtual Reader& readShort (short &val) = 0;

    virtual Reader& readUnsignedShort (unsigned short &val)  = 0;

    virtual Reader& readInt (int &val)  = 0;

    virtual Reader& readUnsignedInt (unsigned int &val)  = 0;

    virtual Reader& readLong (long &val) = 0;

    virtual Reader& readUnsignedLong (unsigned long &val) = 0;

    virtual Reader& readFloat (float &val) = 0;

    virtual Reader& readDouble (double &val) = 0;

}; // interface Reader



/**
 * This class and its descendants are for unicode character-oriented input
 *
 */
class BasicReader : public Reader
{

public:

    BasicReader(Reader &sourceStream);

    virtual ~BasicReader() {}

    virtual int available();

    virtual void close();

    virtual int get();

    virtual DOMString readLine();

    virtual DOMString readWord();

    /* Input formatting */
    virtual Reader& readBool (bool& val );

    virtual Reader& readShort (short &val) ;

    virtual Reader& readUnsignedShort (unsigned short &val) ;

    virtual Reader& readInt (int &val) ;

    virtual Reader& readUnsignedInt (unsigned int &val) ;

    virtual Reader& readLong (long &val) ;

    virtual Reader& readUnsignedLong (unsigned long &val) ;

    virtual Reader& readFloat (float &val) ;

    virtual Reader& readDouble (double &val) ;

protected:

    Reader *source;

    BasicReader()
        { source = NULL; }

private:

}; // class BasicReader



Reader& operator>> (Reader &reader, bool& val );

Reader& operator>> (Reader &reader, short &val);

Reader& operator>> (Reader &reader, unsigned short &val);

Reader& operator>> (Reader &reader, int &val);

Reader& operator>> (Reader &reader, unsigned int &val);

Reader& operator>> (Reader &reader, long &val);

Reader& operator>> (Reader &reader, unsigned long &val);

Reader& operator>> (Reader &reader, float &val);

Reader& operator>> (Reader &reader, double &val);




/**
 * Class for placing a Reader on an open InputStream
 *
 */
class InputStreamReader : public BasicReader
{
public:

    InputStreamReader(const InputStream &inputStreamSource);

    /*Overload these 3 for your implementation*/
    virtual int available();

    virtual void close();

    virtual int get();


private:

    InputStream &inputStream;


};

/**
 * Convenience class for reading formatted from standard input
 *
 */
class StdReader : public BasicReader
{
public:

    StdReader();

    virtual ~StdReader();

    /*Overload these 3 for your implementation*/
    virtual int available();

    virtual void close();

    virtual int get();


private:

    InputStream *inputStream;


};





//#########################################################################
//# W R I T E R
//#########################################################################

/**
 * This interface and its descendants are for unicode character-oriented output
 *
 */
class Writer
{

public:

    /**
     * Constructor.
     */
    Writer() {}

    /**
     * Destructor
     */
    virtual ~Writer() {}

    virtual void close() = 0;

    virtual void flush() = 0;

    virtual int put(XMLCh ch) = 0;

    /* Formatted output */
    virtual Writer& printf(const DOMString &fmt, ...) = 0;

    virtual Writer& writeChar(char val) = 0;

    virtual Writer& writeString(const DOMString &val) = 0;

    virtual Writer& writeBool (bool val ) = 0;

    virtual Writer& writeShort (short val ) = 0;

    virtual Writer& writeUnsignedShort (unsigned short val ) = 0;

    virtual Writer& writeInt (int val ) = 0;

    virtual Writer& writeUnsignedInt (unsigned int val ) = 0;

    virtual Writer& writeLong (long val ) = 0;

    virtual Writer& writeUnsignedLong (unsigned long val ) = 0;

    virtual Writer& writeFloat (float val ) = 0;

    virtual Writer& writeDouble (double val ) = 0;



}; // interface Writer


/**
 * This class and its descendants are for unicode character-oriented output
 *
 */
class BasicWriter : public Writer
{

public:

    BasicWriter(const Writer &destinationWriter);

    virtual ~BasicWriter() {}

    /*Overload these 3 for your implementation*/
    virtual void close();

    virtual void flush();

    virtual int put(XMLCh ch);



    /* Formatted output */
    virtual Writer &printf(const DOMString &fmt, ...);

    virtual Writer& writeChar(char val);

    virtual Writer& writeString(const DOMString &val);

    virtual Writer& writeBool (bool val );

    virtual Writer& writeShort (short val );

    virtual Writer& writeUnsignedShort (unsigned short val );

    virtual Writer& writeInt (int val );

    virtual Writer& writeUnsignedInt (unsigned int val );

    virtual Writer& writeLong (long val );

    virtual Writer& writeUnsignedLong (unsigned long val );

    virtual Writer& writeFloat (float val );

    virtual Writer& writeDouble (double val );


protected:

    Writer *destination;

    BasicWriter()
        { destination = NULL; }

    //Used for printf() or other things that might
    //require formatting before sending down the stream
    char formatBuf[2048];

private:

}; // class BasicWriter



Writer& operator<< (Writer &writer, char val);

Writer& operator<< (Writer &writer, const DOMString &val);

Writer& operator<< (Writer &writer, bool val);

Writer& operator<< (Writer &writer, short val);

Writer& operator<< (Writer &writer, unsigned short val);

Writer& operator<< (Writer &writer, int val);

Writer& operator<< (Writer &writer, unsigned int val);

Writer& operator<< (Writer &writer, long val);

Writer& operator<< (Writer &writer, unsigned long val);

Writer& operator<< (Writer &writer, float val);

Writer& operator<< (Writer &writer, double val);




/**
 * Class for placing a Writer on an open OutputStream
 *
 */
class OutputStreamWriter : public BasicWriter
{
public:

    OutputStreamWriter(OutputStream &outputStreamDest);

    /*Overload these 3 for your implementation*/
    virtual void close();

    virtual void flush();

    virtual int put(XMLCh ch);


private:

    OutputStream &outputStream;


};


/**
 * Convenience class for writing to standard output
 */
class StdWriter : public BasicWriter
{
public:
    StdWriter();

    virtual ~StdWriter();


    virtual void close();


    virtual void flush();


    virtual int put(XMLCh ch);


private:

    OutputStream *outputStream;

};

//#########################################################################
//# U T I L I T Y
//#########################################################################

void pipeStream(InputStream &source, OutputStream &dest);



}  //namespace io
}  //namespace dom
}  //namespace w3c
}  //namespace org


#endif /* __DOMSTREAM_H__ */

//#########################################################################
//# E N D    O F    F I L E
//#########################################################################
