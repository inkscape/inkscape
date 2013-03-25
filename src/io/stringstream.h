#ifndef __INKSCAPE_IO_STRINGSTREAM_H__
#define __INKSCAPE_IO_STRINGSTREAM_H__

#include <glibmm/ustring.h>

#include "inkscapestream.h"


namespace Inkscape
{
namespace IO
{


//#########################################################################
//# S T R I N G    I N P U T    S T R E A M
//#########################################################################

/**
 * This class is for reading character from a Glib::ustring
 *
 */
class StringInputStream : public InputStream
{

public:

    StringInputStream(Glib::ustring &sourceString);
    
    virtual ~StringInputStream();
    
    virtual int available();
    
    virtual void close();
    
    virtual int get();
    
private:

    Glib::ustring &buffer;

    long position;

}; // class StringInputStream




//#########################################################################
//# S T R I N G   O U T P U T    S T R E A M
//#########################################################################

/**
 * This class is for sending a stream to a Glib::ustring
 *
 */
class StringOutputStream : public OutputStream
{

public:

    StringOutputStream();
    
    virtual ~StringOutputStream();
    
    virtual void close();
    
    virtual void flush();
    
    virtual int put(gunichar ch);

    virtual Glib::ustring &getString()
        { return buffer; }

    virtual void clear()
        { buffer = ""; }

private:

    Glib::ustring buffer;


}; // class StringOutputStream







} // namespace IO
} // namespace Inkscape



#endif /* __INKSCAPE_IO_STRINGSTREAM_H__ */
