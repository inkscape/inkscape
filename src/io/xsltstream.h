#ifndef SEEN_INKSCAPE_IO_XSLTSTREAM_H
#define SEEN_INKSCAPE_IO_XSLTSTREAM_H
/**
 * @file
 * Xslt-enabled input and output streams.
 */
/*
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
    XsltStyleSheet(InputStream &source);

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

    XsltInputStream(InputStream &xmlSource, XsltStyleSheet &stylesheet);
    
    virtual ~XsltInputStream();
    
    virtual int available();
    
    virtual void close();
    
    virtual int get();
    

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

    XsltOutputStream(OutputStream &destination, XsltStyleSheet &stylesheet);
    
    virtual ~XsltOutputStream();
    
    virtual void close();
    
    virtual void flush();
    
    virtual int put(gunichar ch);

private:

    XsltStyleSheet &stylesheet;

    Glib::ustring outbuf;
    
    bool flushed;

}; // class UriOutputStream



} // namespace IO
} // namespace Inkscape


#endif /* __INKSCAPE_IO_XSLTSTREAM_H__ */
