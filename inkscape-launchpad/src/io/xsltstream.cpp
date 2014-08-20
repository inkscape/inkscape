/*
 * XSL Transforming input and output classes
 *
 * Authors:
 *   Bob Jamison <ishmalius@gmail.com>
 *
 * Copyright (C) 2004-2008 Inkscape.org
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include "xsltstream.h"
#include "stringstream.h"
#include <libxslt/transform.h>




namespace Inkscape
{
namespace IO
{

//#########################################################################
//# X S L T    S T Y L E S H E E T
//#########################################################################

/**
 *
 */
XsltStyleSheet::XsltStyleSheet(InputStream &xsltSource)
              
                   : stylesheet(NULL)
{
    if (!read(xsltSource)) {
        throw StreamException("read failed");
        }
}

/**
 *
 */
XsltStyleSheet::XsltStyleSheet()
                    : stylesheet(NULL)
{
}



/**
 *
 */
bool XsltStyleSheet::read(InputStream &xsltSource)
{
    StringOutputStream outs;
    pipeStream(xsltSource, outs);
    std::string strBuf = outs.getString().raw();
    xmlDocPtr doc = xmlParseMemory(strBuf.c_str(), strBuf.size());
    stylesheet = xsltParseStylesheetDoc(doc);
    //following not necessary.  handled by xsltFreeStylesheet(stylesheet);
    //xmlFreeDoc(doc);
    if (!stylesheet)
        return false;
    return true;
}


/**
 *
 */ 
XsltStyleSheet::~XsltStyleSheet()
{
    if (stylesheet)
        xsltFreeStylesheet(stylesheet);
}



//#########################################################################
//# X S L T    I N P U T    S T R E A M
//#########################################################################


/**
 *
 */ 
XsltInputStream::XsltInputStream(InputStream &xmlSource, XsltStyleSheet &sheet)
                        : BasicInputStream(xmlSource), stylesheet(sheet)
{
    //Load the data
    StringOutputStream outs;
    pipeStream(source, outs);
    std::string strBuf = outs.getString().raw();
    
    //Do the processing
    const char *params[1];
    params[0] = NULL;
    xmlDocPtr srcDoc = xmlParseMemory(strBuf.c_str(), strBuf.size());
    xmlDocPtr resDoc = xsltApplyStylesheet(stylesheet.stylesheet, srcDoc, params);
    xmlDocDumpFormatMemory(resDoc, &outbuf, &outsize, 1);
    outpos = 0;
        
    //Free our mem
    xmlFreeDoc(resDoc);
    xmlFreeDoc(srcDoc);
}

/**
 *
 */ 
XsltInputStream::~XsltInputStream()
{
    xmlFree(outbuf);
}

/**
 * Returns the number of bytes that can be read (or skipped over) from
 * this input stream without blocking by the next caller of a method for
 * this input stream.
 */ 
int XsltInputStream::available()
{
    return outsize - outpos;
}

    
/**
 *  Closes this input stream and releases any system resources
 *  associated with the stream.
 */ 
void XsltInputStream::close()
{
    closed = true;
}
    
/**
 * Reads the next byte of data from the input stream.  -1 if EOF
 */ 
int XsltInputStream::get()
{
    if (closed)
        return -1;
    if (outpos >= outsize)
        return -1;
    int ch = (int) outbuf[outpos++];
    return ch;
}
   





//#########################################################################
//#  X S L T     O U T P U T    S T R E A M
//#########################################################################

/**
 *
 */ 
XsltOutputStream::XsltOutputStream(OutputStream &dest, XsltStyleSheet &sheet)
                     : BasicOutputStream(dest), stylesheet(sheet)
{
    flushed = false;
}

/**
 *
 */ 
XsltOutputStream::~XsltOutputStream()
{
    //do not automatically close
}

/**
 * Closes this output stream and releases any system resources
 * associated with this stream.
 */ 
void XsltOutputStream::close()
{
    flush();
    destination.close();
}
    
/**
 *  Flushes this output stream and forces any buffered output
 *  bytes to be written out.
 */ 
void XsltOutputStream::flush()
{
    if (flushed)
        {
        destination.flush();
        return;
        }
        
    //Do the processing
    xmlChar *resbuf;
    int resSize;
    const char *params[1];
    params[0] = NULL;
    xmlDocPtr srcDoc = xmlParseMemory(outbuf.raw().c_str(), outbuf.size());
    xmlDocPtr resDoc = xsltApplyStylesheet(stylesheet.stylesheet, srcDoc, params);
    xmlDocDumpFormatMemory(resDoc, &resbuf, &resSize, 1);
    /*
    xmlErrorPtr err = xmlGetLastError();
    if (err)
        {
        throw StreamException(err->message);
        }
    */

    for (int i=0 ; i<resSize ; i++)
        {
        char ch = resbuf[i];
        destination.put(ch);
        }
        
    //Free our mem
    xmlFree(resbuf);
    xmlFreeDoc(resDoc);
    xmlFreeDoc(srcDoc);
    destination.flush();
    flushed = true;
}
    
/**
 * Writes the specified byte to this output stream.
 */ 
int XsltOutputStream::put(gunichar ch)
{
    outbuf.push_back(ch);
	return 1;
}





} // namespace IO
} // namespace Inkscape


//#########################################################################
//# E N D    O F    F I L E
//#########################################################################
