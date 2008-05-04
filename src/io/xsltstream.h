#ifndef __INKSCAPE_IO_XSLTSTREAM_H__
#define __INKSCAPE_IO_XSLTSTREAM_H__
/**
 * Xslt-enabled input and output streams
 *
 *
 * Authors:
 *   Bob Jamison <ishmalius@gmail.com>
 *
 * Copyright (C) 2004-2008 Inkscape.org
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include "inkscapestream.h"

#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>


namespace Inkscape
{
namespace IO
{

//#########################################################################
//# X S L T    S T Y L E S H E E T
//#########################################################################
/**
 * This is a container for reusing a loaded stylesheet
 */
class XsltStyleSheet
{

public:

    /**
     * Constructor with loading
     */
    XsltStyleSheet(InputStream &source)  throw (StreamException);

    /**
     * Simple constructor, no loading
     */
    XsltStyleSheet();

    /**
     * Loader
     */
    bool read(InputStream &source);

    /**
     * Destructor
     */
    virtual ~XsltStyleSheet();
    
    xsltStylesheetPtr stylesheet;


}; // class XsltStyleSheet


//#########################################################################
//# X S L T    I N P U T    S T R E A M
//#########################################################################

/**
 * This class is for transforming stream input by a given stylesheet
 */
class XsltInputStream : public BasicInputStream
{

public:

    XsltInputStream(InputStream &xmlSource, XsltStyleSheet &stylesheet)
                        throw (StreamException);
    
    virtual ~XsltInputStream() throw (StreamException);
    
    virtual int available() throw (StreamException);
    
    virtual void close() throw (StreamException);
    
    virtual int get() throw (StreamException);
    

private:

    XsltStyleSheet &stylesheet;

    xmlChar *outbuf;
    int outsize;
    int outpos;

}; // class UriInputStream




//#########################################################################
//# X S L T    O U T P U T    S T R E A M
//#########################################################################

/**
 * This class is for transforming stream output by a given stylesheet
 */
class XsltOutputStream : public BasicOutputStream
{

public:

    XsltOutputStream(OutputStream &destination, XsltStyleSheet &stylesheet)
                             throw (StreamException);
    
    virtual ~XsltOutputStream() throw (StreamException);
    
    virtual void close() throw (StreamException);
    
    virtual void flush() throw (StreamException);
    
    virtual void put(int ch) throw (StreamException);

private:

    XsltStyleSheet &stylesheet;

    Glib::ustring outbuf;
    
    bool flushed;

}; // class UriOutputStream



} // namespace IO
} // namespace Inkscape


#endif /* __INKSCAPE_IO_XSLTSTREAM_H__ */
