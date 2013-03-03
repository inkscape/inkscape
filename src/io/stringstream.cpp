/*
 * Our base String stream classes.  We implement these to
 * be based on Glib::ustring
 *
 * Authors:
 *   Bob Jamison <rjamison@titan.com>
 *
 * Copyright (C) 2004 Inkscape.org
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include "stringstream.h"

namespace Inkscape
{
namespace IO
{


//#########################################################################
//# S T R I N G    I N P U T    S T R E A M
//#########################################################################


/**
 *
 */ 
StringInputStream::StringInputStream(Glib::ustring &sourceString)
                      : buffer(sourceString)
{
    position = 0;
}

/**
 *
 */ 
StringInputStream::~StringInputStream()
{

}

/**
 * Returns the number of bytes that can be read (or skipped over) from
 * this input stream without blocking by the next caller of a method for
 * this input stream.
 */ 
int StringInputStream::available()
{
    return buffer.size() - position;
}

    
/**
 *  Closes this input stream and releases any system resources
 *  associated with the stream.
 */ 
void StringInputStream::close()
{
}
    
/**
 * Reads the next byte of data from the input stream.  -1 if EOF
 */ 
int StringInputStream::get()
{
    if (position >= (int)buffer.size())
        return -1;
    int ch = (int) buffer[position++];
    return ch;
}
   



//#########################################################################
//# S T R I N G     O U T P U T    S T R E A M
//#########################################################################

/**
 *
 */ 
StringOutputStream::StringOutputStream()
{
}

/**
 *
 */ 
StringOutputStream::~StringOutputStream()
{
}

/**
 * Closes this output stream and releases any system resources
 * associated with this stream.
 */ 
void StringOutputStream::close()
{
}
    
/**
 *  Flushes this output stream and forces any buffered output
 *  bytes to be written out.
 */ 
void StringOutputStream::flush()
{
    //nothing to do
}
    
/**
 * Writes the specified byte to this output stream.
 */ 
int StringOutputStream::put(gunichar ch)
{
    buffer.push_back(ch);
	return 1;
}


} // namespace IO
} // namespace Inkscape


//#########################################################################
//# E N D    O F    F I L E
//#########################################################################
