#ifndef SEEN_INKSCAPE_IO_INKSCAPESTREAM_H
#define SEEN_INKSCAPE_IO_INKSCAPESTREAM_H
/*
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Inkscape.org
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <cstdio>
#include <glibmm/ustring.h>

namespace Inkscape
{
namespace IO
{

class StreamException : public std::exception
{
public:

    StreamException(const char *theReason) throw()
        { reason = theReason; }
    StreamException(Glib::ustring &theReason) throw()
        { reason = theReason; }
    virtual ~StreamException() throw()
        {  }
    char const *what() const throw()
        { return reason.c_str(); }
        
private:
    Glib::ustring reason;

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

    BasicInputStream(InputStream &sourceStream);
    
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
    virtual int put(gunichar ch) = 0;


}; // class OutputStream


/**
 * This is the class that most users should inherit, to provide
 * their own output streams.
 */
class BasicOutputStream : public OutputStream
{

public:

    BasicOutputStream(OutputStream &destinationStream);
    
    virtual ~BasicOutputStream() {}

    virtual void close();
    
    virtual void flush();
    
    virtual int put(gunichar ch);

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
    
    int put(gunichar ch)
        {return  putchar(ch); }

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
    
    virtual gunichar get() = 0;
    
    virtual Glib::ustring readLine() = 0;
    
    virtual Glib::ustring readWord() = 0;
    
    /* Input formatting */
    virtual const Reader& readBool (bool& val ) = 0;
    virtual const Reader& operator>> (bool& val ) = 0;
        
    virtual const Reader& readShort (short &val) = 0;
    virtual const Reader& operator>> (short &val) = 0;
        
    virtual const Reader& readUnsignedShort (unsigned short &val) = 0;
    virtual const Reader& operator>> (unsigned short &val) = 0;
        
    virtual const Reader& readInt (int &val) = 0;
    virtual const Reader& operator>> (int &val) = 0;
        
    virtual const Reader& readUnsignedInt (unsigned int &val) = 0;
    virtual const Reader& operator>> (unsigned int &val) = 0;
        
    virtual const Reader& readLong (long &val) = 0;
    virtual const Reader& operator>> (long &val) = 0;
        
    virtual const Reader& readUnsignedLong (unsigned long &val) = 0;
    virtual const Reader& operator>> (unsigned long &val) = 0;
        
    virtual const Reader& readFloat (float &val) = 0;
    virtual const Reader& operator>> (float &val) = 0;
        
    virtual const Reader& readDouble (double &val) = 0;
    virtual const Reader& operator>> (double &val) = 0;

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
    
    virtual gunichar get();
    
    virtual Glib::ustring readLine();
    
    virtual Glib::ustring readWord();
    
    /* Input formatting */
    virtual const Reader& readBool (bool& val );
    virtual const Reader& operator>> (bool& val )
        { return readBool(val); }
        
    virtual const Reader& readShort (short &val);
    virtual const Reader& operator>> (short &val)
        { return readShort(val); }
        
    virtual const Reader& readUnsignedShort (unsigned short &val);
    virtual const Reader& operator>> (unsigned short &val)
        { return readUnsignedShort(val); }
        
    virtual const Reader& readInt (int &val);
    virtual const Reader& operator>> (int &val)
        { return readInt(val); }
        
    virtual const Reader& readUnsignedInt (unsigned int &val);
    virtual const Reader& operator>> (unsigned int &val)
        { return readUnsignedInt(val); }
        
    virtual const Reader& readLong (long &val);
    virtual const Reader& operator>> (long &val)
        { return readLong(val); }
        
    virtual const Reader& readUnsignedLong (unsigned long &val);
    virtual const Reader& operator>> (unsigned long &val)
        { return readUnsignedLong(val); }
        
    virtual const Reader& readFloat (float &val);
    virtual const Reader& operator>> (float &val)
        { return readFloat(val); }
        
    virtual const Reader& readDouble (double &val);
    virtual const Reader& operator>> (double &val)
        { return readDouble(val); }
 

protected:

    Reader *source;

    BasicReader()
        { source = NULL; }

private:

}; // class BasicReader



/**
 * Class for placing a Reader on an open InputStream
 *
 */
class InputStreamReader : public BasicReader
{
public:

    InputStreamReader(InputStream &inputStreamSource);
    
    /*Overload these 3 for your implementation*/
    virtual int available();
    
    virtual void close();
    
    virtual gunichar get();


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
    
    virtual gunichar get();


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
    
    virtual void put(gunichar ch) = 0;
    
    /* Formatted output */
    virtual Writer& printf(char const *fmt, ...) G_GNUC_PRINTF(2,3) = 0;

    virtual Writer& writeChar(char val) = 0;

    virtual Writer& writeUString(Glib::ustring &val) = 0;

    virtual Writer& writeStdString(std::string &val) = 0;

    virtual Writer& writeString(const char *str) = 0;

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

    BasicWriter(Writer &destinationWriter);

    virtual ~BasicWriter() {}

    /*Overload these 3 for your implementation*/
    virtual void close();
    
    virtual void flush();
    
    virtual void put(gunichar ch);
    
    
    
    /* Formatted output */
    virtual Writer &printf(char const *fmt, ...) G_GNUC_PRINTF(2,3);

    virtual Writer& writeChar(char val);

    virtual Writer& writeUString(Glib::ustring &val);

    virtual Writer& writeStdString(std::string &val);

    virtual Writer& writeString(const char *str);

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
    
private:

}; // class BasicWriter



Writer& operator<< (Writer &writer, char val);

Writer& operator<< (Writer &writer, Glib::ustring &val);

Writer& operator<< (Writer &writer, std::string &val);

Writer& operator<< (Writer &writer, char const *val);

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
    
    virtual void put(gunichar ch);


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

    
    virtual void put(gunichar ch);


private:

    OutputStream *outputStream;

};

//#########################################################################
//# U T I L I T Y
//#########################################################################

void pipeStream(InputStream &source, OutputStream &dest);



} // namespace IO
} // namespace Inkscape


#endif // SEEN_INKSCAPE_IO_INKSCAPESTREAM_H
