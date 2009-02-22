#ifndef __LS_H__
#define __LS_H__
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
 * Copyright (C) 2005 Bob Jamison
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


#include "dom.h"
#include "events.h"
#include "traversal.h"

#include "io/domstream.h"

namespace org
{
namespace w3c
{
namespace dom
{
namespace ls
{



//Local definitions
//The idl said Object.  Since this is undefined, we will
//use our own class which is designed to be a bit similar to
//java.io streams

typedef dom::io::InputStream  LSInputStream;
typedef dom::io::OutputStream LSOutputStream;
typedef dom::io::Reader       LSReader;
typedef dom::io::Writer       LSWriter;


//local definitions
typedef dom::DOMString DOMString;
typedef dom::DOMConfiguration DOMConfiguration;
typedef dom::Node Node;
typedef dom::NodePtr NodePtr;
typedef dom::Document Document;
typedef dom::DocumentPtr DocumentPtr;
typedef dom::Element Element;
typedef dom::ElementPtr ElementPtr;


//forward declarations
class LSParser;
class LSSerializer;
class LSInput;
class LSOutput;
class LSParserFilter;
class LSSerializerFilter;



/*#########################################################################
## LSException
#########################################################################*/

/**
 *  Maybe this should inherit from DOMException?
 */
class LSException
{

public:

    LSException(const DOMString &reasonMsg)
        { msg = reasonMsg; }

    LSException(short theCode)
        {
        code = theCode;
        }

    virtual ~LSException() throw()
       {}

    /**
     *
     */
    unsigned short code;

    /**
     *
     */
    DOMString msg;

    /**
     * Get a string, translated from the code.
     * Like std::exception. Not in spec.
     */
    const char *what()
        { return msg.c_str(); }



};


/**
 * LSExceptionCode
 */
typedef enum
    {
    PARSE_ERR                      = 81,
    SERIALIZE_ERR                  = 82
    } XPathExceptionCode;


/*#########################################################################
## LSParserFilter
#########################################################################*/

/**
 *
 */
class LSParserFilter
{
public:

    // Constants returned by startElement and acceptNode
    typedef enum
        {
        FILTER_ACCEPT                  = 1,
        FILTER_REJECT                  = 2,
        FILTER_SKIP                    = 3,
        FILTER_INTERRUPT               = 4
        } ReturnValues;


    /**
     *
     */
    virtual unsigned short startElement(const ElementPtr elementArg) =0;

    /**
     *
     */
    virtual unsigned short acceptNode(const NodePtr nodeArg) =0;

    /**
     *
     */
    virtual unsigned long getWhatToShow() =0;

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~LSParserFilter() {}



};

/*#########################################################################
## LSInput
#########################################################################*/

/**
 *
 */
class LSInput
{
public:

    /**
     *
     */
    virtual LSReader *getCharacterStream() const
        { return characterStream; }

    /**
     *
     */
    virtual void setCharacterStream(const LSReader *val)
        { characterStream = (LSReader *)val; }

    /**
     *
     */
    virtual LSInputStream *getByteStream() const
        { return byteStream; }

    /**
     *
     */
    virtual void setByteStream(const LSInputStream *val)
        { byteStream =  (LSInputStream *)val; }

    /**
     *
     */
    virtual DOMString getStringData() const
        { return stringData; }

    /**
     *
     */
    virtual void setStringData(const DOMString &val)
        { stringData = val; }

    /**
     *
     */
    virtual DOMString getSystemId() const
        { return systemId; }

    /**
     *
     */
    virtual void setSystemId(const DOMString &val)
        { systemId = val; }

    /**
     *
     */
    virtual DOMString getPublicId() const
        { return publicId; }

    /**
     *
     */
    virtual void setPublicId(const DOMString &val)
        { publicId = val; }

    /**
     *
     */
    virtual DOMString getBaseURI() const
        { return baseURI; }

    /**
     *
     */
    virtual void setBaseURI(const DOMString &val)
        { baseURI = val; }

    /**
     *
     */
    virtual DOMString getEncoding() const
        { return encoding; }

    /**
     *
     */
    virtual void setEncoding(const DOMString &val)
        { encoding = val; }

    /**
     *
     */
    virtual bool getCertifiedText() const
        { return certifiedText; }

    /**
     *
     */
    virtual void setCertifiedText(bool val)
        { certifiedText = val; }

    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    LSInput()
        {
        characterStream = NULL;
        byteStream      = NULL;
        stringData      = "";
        systemId        = "";
        publicId        = "";
        baseURI         = "";
        encoding        = "";
        certifiedText   = false;
        }



    /**
     *
     */
    LSInput(const LSInput &other)
        {
        characterStream = other.characterStream;
        byteStream      = other.byteStream;
        stringData      = other.stringData;
        systemId        = other.systemId;
        publicId        = other.publicId;
        baseURI         = other.baseURI;
        encoding        = other.encoding;
        certifiedText   = other.certifiedText;
        }

    /**
     *
     */
    virtual ~LSInput()
        {}

private:

    LSReader      *characterStream;
    LSInputStream *byteStream;
    DOMString     stringData;
    DOMString     systemId;
    DOMString     publicId;
    DOMString     baseURI;
    DOMString     encoding;
    bool          certifiedText;


};


/*#########################################################################
## LSParser
#########################################################################*/

/**
 *
 */
class LSParser
{
public:


    /**
     *
     */
    virtual DOMConfiguration *getDomConfig()
        { return NULL; }

    /**
     *
     */
    virtual LSParserFilter *getFilter()
        { return filter; }

    /**
     *
     */
    virtual void setFilter(const LSParserFilter *val)
        { filter = (LSParserFilter *)val; }

    /**
     *
     */
    virtual bool getAsync()
        { return false; }

    /**
     *
     */
    virtual bool getBusy()
        { return false; }

    /**
     *
     */
    virtual DocumentPtr parse(const LSInput &/*input*/)
                              throw(dom::DOMException, LSException)
        { return NULL; }


    /**
     *
     */
    virtual DocumentPtr parseURI(const DOMString &/*uri*/)
                                 throw(dom::DOMException, LSException)
        { return NULL; }

    typedef enum
        {
        ACTION_APPEND_AS_CHILDREN      = 1,
        ACTION_REPLACE_CHILDREN        = 2,
        ACTION_INSERT_BEFORE           = 3,
        ACTION_INSERT_AFTER            = 4,
        ACTION_REPLACE                 = 5
        } ActionTypes;


    /**
     *
     */
    virtual NodePtr parseWithContext(const LSInput &/*input*/,
                                     const NodePtr /*contextArg*/,
                                     unsigned short /*action*/)
                                     throw(dom::DOMException, LSException)
        { return NULL; }

    /**
     *
     */
    virtual void abort()
        {}



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    LSParser()
        {
        filter = NULL;
        }

    /**
     *
     */
    LSParser(const LSParser &other)
        {
        filter = other.filter;
        }

    /**
     *
     */
    virtual ~LSParser() {}

protected:

    LSParserFilter *filter;
};



/*#########################################################################
## LSResourceResolver
#########################################################################*/

/**
 *
 */
class LSResourceResolver
{
public:

    /**
     *
     */
    virtual LSInput resolveResource(const DOMString &/*type*/,
                                    const DOMString &/*namespaceURI*/,
                                    const DOMString &/*publicId*/,
                                    const DOMString &/*systemId*/,
                                    const DOMString &/*baseURI*/)
        {
        LSInput input;
        //do something
        return input;
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    LSResourceResolver() {}

    /**
     *
     */
    LSResourceResolver(const LSResourceResolver &/*other*/)
        {
        }

    /**
     *
     */
    virtual ~LSResourceResolver() {}



};

/*#########################################################################
## LSOutput
#########################################################################*/

/**
 *
 */
class LSOutput
{
public:

    /**
     *
     */
    virtual LSWriter *getCharacterStream() const
        { return characterStream; }

    /**
     *
     */
    virtual void setCharacterStream(const LSWriter *val)
        { characterStream = (LSWriter *)val; }

    /**
     *
     */
    virtual LSOutputStream *getByteStream() const
        { return byteStream; }

    /**
     *
     */
    virtual void setByteStream(const LSOutputStream *val)
        { byteStream = (LSOutputStream *) val; }

    /**
     *
     */
    virtual DOMString getSystemId() const
        { return systemId; }

    /**
     *
     */
    virtual void setSystemId(const DOMString &val)
        { systemId = val; }

    /**
     *
     */
    virtual DOMString getEncoding() const
        { return encoding; }

    /**
     *
     */
    virtual void setEncoding(const DOMString &val)
        { encoding = val; }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    LSOutput()
        {
        characterStream = NULL;
        byteStream      = NULL;
        systemId        = "";
        encoding        = "";
        }


    /**
     *
     */
    LSOutput(const LSOutput &other)
        {
        characterStream = other.characterStream;
        byteStream      = other.byteStream;
        systemId        = other.systemId;
        encoding        = other.encoding;
        }

    /**
     *
     */
    virtual ~LSOutput()
        {}

private:

    LSWriter       *characterStream;
    LSOutputStream *byteStream;
    DOMString      systemId;
    DOMString      encoding;

};


/*#########################################################################
## LSSerializer
#########################################################################*/

/**
 *
 */
class LSSerializer
{
public:

    /**
     *
     */
    virtual DOMConfiguration *getDomConfig()
        { return NULL; }

    /**
     *
     */
    virtual DOMString getNewLine()
        { return newLine; }
    /**
     *
     */
    virtual void setNewLine(const DOMString &val)
        { newLine = val; }

    /**
     *
     */
    virtual LSSerializerFilter *getFilter()
        { return filter; }

    /**
     *
     */
    virtual void setFilter(const LSSerializerFilter *val)
        { filter = (LSSerializerFilter *)val; }

    /**
     *
     */
    virtual bool write(const NodePtr /*nodeArg*/,
                       const LSOutput &/*destination*/)
                       throw (LSException)
        { return false; }

    /**
     *
     */
    virtual bool writeToURI(const NodePtr /*nodeArg*/,
                            const DOMString &/*uri*/)
                            throw(LSException)
        { return false; }

    /**
     *
     */
    virtual DOMString writeToString(const NodePtr /*nodeArg*/)
                                    throw(dom::DOMException, LSException)
        {
        DOMString str;
        return str;
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    LSSerializer()
       {
       filter  = NULL;
       newLine = "\n";
       }

    /**
     *
     */
    LSSerializer(const LSSerializer &other)
       {
       filter  = other.filter;
       newLine = other.newLine;
       }

    /**
     *
     */
    virtual ~LSSerializer() {}

protected:

    LSSerializerFilter *filter;
    DOMString newLine;

};

/*#########################################################################
## LSProgressEvent
#########################################################################*/

/**
 *
 */
class LSProgressEvent : virtual public events::Event
{
public:

    /**
     *
     */
    virtual LSInput &getInput()
        {
        return input;
        }

    /**
     *
     */
    virtual unsigned long getPosition()
        { return position; }

    /**
     *
     */
    virtual unsigned long getTotalSize()
        { return totalSize; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    LSProgressEvent(const LSInput &inputArg, unsigned long positionArg,
                    unsigned long totalSizeArg) : input((LSInput &)inputArg)
        {
        position  = positionArg;
        totalSize = totalSizeArg;
        }


    /**
     *
     */
    LSProgressEvent(const LSProgressEvent &other)
                : events::Event(other) , input(other.input)
        {
        position  = other.position;
        totalSize = other.totalSize;
        }


    /**
     *
     */
    virtual ~LSProgressEvent() {}

protected:

    LSInput &input;
    unsigned long position;
    unsigned long totalSize;

};

/*#########################################################################
## LSLoadEvent
#########################################################################*/

/**
 *
 */
class LSLoadEvent : public events::Event
{
public:

    /**
     *
     */
    virtual DocumentPtr getNewDocument()
        { return newDocument; }

    /**
     *
     */
    virtual LSInput &getInput()
        { return input; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    LSLoadEvent(const LSInput &inputArg,
	            const DocumentPtr docArg)
                  : input((LSInput &)inputArg)
        { newDocument = docArg; }

    /**
     *
     */
    LSLoadEvent(const LSLoadEvent &other) 
	       : events::Event(other) , input(other.input)
        {
        newDocument = other.newDocument;
        }

    /**
     *
     */
    virtual ~LSLoadEvent() {}

protected:

    DocumentPtr newDocument;

    LSInput &input;


};



/*#########################################################################
## LSSerializerFilter
#########################################################################*/

/**
 *
 */
class LSSerializerFilter : virtual public traversal::NodeFilter
{
public:

    /**
     *
     */
    virtual unsigned long  getWhatToShow() =0;

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~LSSerializerFilter() {}
};




/*#########################################################################
## DOMImplementationLS
#########################################################################*/

/**
 *
 */
class DOMImplementationLS
{
public:

    typedef enum
        {
        MODE_SYNCHRONOUS               = 1,
        MODE_ASYNCHRONOUS              = 2
        } DOMImplementationLSMode;

    /**
     * To use, for this and subclasses:
     *  LSParser &parser = myImplementation.createLSParser(mode, schemaType);
     */
    virtual LSParser &createLSParser(unsigned short mode,
                                    const DOMString &schemaType)
                                    throw (dom::DOMException) =0;

    /**
     * To use, for this and subclasses:
     *  LSSerializer &serializer = myImplementation.createLSSerializer();
     *
     */
    virtual LSSerializer &createLSSerializer() =0;

    /**
     *
     */
    virtual LSInput createLSInput() =0;

    /**
     *
     */
    virtual LSOutput createLSOutput() =0;

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~DOMImplementationLS() {}
};




}  //namespace ls
}  //namespace dom
}  //namespace w3c
}  //namespace org


#endif // __LS_H__

/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/

