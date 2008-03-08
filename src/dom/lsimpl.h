#ifndef __LSIMPL_H__
#define __LSIMPL_H__
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
 * Copyright (C) 2005-2007 Bob Jamison
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

#include "domimpl.h"
#include "events.h"
#include "traversal.h"
#include "ls.h"


#include "xmlreader.h"

namespace org
{
namespace w3c
{
namespace dom
{
namespace ls
{


/*#########################################################################
## LSParserImpl
#########################################################################*/

/**
 *
 */
class LSParserImpl : virtual public LSParser
{
public:

    typedef enum
        {
        PARSE_AS_DATA     = 0,
        PARSE_AS_DOCUMENT = 1
        } ParsingModes;

   /**
     *
     */
    virtual bool getBusy();

    /**
     *
     */
    virtual DocumentPtr parse(const LSInput &input)
                              throw(dom::DOMException, LSException);


    /**
     *
     */
    virtual DocumentPtr parseURI(const DOMString &uri)
                                 throw(dom::DOMException, LSException);

   /**
     *
     */
    virtual NodePtr parseWithContext(const LSInput &input,
                                     const NodePtr contextArg,
                                     unsigned short action)
                                     throw(dom::DOMException, LSException);


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    LSParserImpl()
        {}

    /**
     *
     */
    LSParserImpl(const LSParserImpl &other) : LSParser(other)
        {}

    /**
     *
     */
    virtual ~LSParserImpl()
        {}



    //##################
    //# Internals
    //##################


protected:

    XmlReader reader;
    LSParserFilter *filter;

};




/*#########################################################################
## LSParserFilterImpl
#########################################################################*/

/**
 *
 */
class LSParserFilterImpl : virtual public LSParserFilter
{
public:

    /**
     *
     */
    virtual unsigned short startElement(const ElementPtr /*elementArg*/)
        { return 0; }

    /**
     *
     */
    virtual unsigned short acceptNode(const NodePtr /*nodeArg*/)
        { return 0; }

    /**
     *
     */
    virtual unsigned long getWhatToShow()
        { return 0; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~LSParserFilterImpl()
        {}



};

/*#########################################################################
## LSSerializerImpl
#########################################################################*/

/**
 *
 */
class LSSerializerImpl : virtual public LSSerializer
{
public:


    /**
     *
     */
    virtual bool write(const NodePtr nodeArg,
                       const LSOutput &destination)
                       throw (LSException);

    /**
     *
     */
    virtual bool writeToURI(const NodePtr nodeArg,
                            const DOMString &uri)
                            throw(LSException);

    /**
     *
     */
    virtual DOMString writeToString(const NodePtr nodeArg)
                                    throw(dom::DOMException, LSException);

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    LSSerializerImpl()
        {
        indent = 0;
        }

    /**
     *
     */
    virtual ~LSSerializerImpl()
        {}



protected:

    /**
     *
     */
    void writeNode(const NodePtr nodeArg);

private:

    void spaces();

    void po(char const *fmt, ...)
    #ifdef G_GNUC_PRINTF
    G_GNUC_PRINTF(2, 3)
    #endif
    ;

    void pos(const DOMString &str);

    void poxml(const DOMString &str);

    DOMString          outbuf;

    int                indent;

    DOMConfiguration   *domConfig;

    LSSerializerFilter *filter;



};




/*#########################################################################
## LSSerializerFilterImpl
#########################################################################*/

/**
 *
 */
class LSSerializerFilterImpl : virtual public LSSerializerFilter
{
public:

    /**
     *
     */
    virtual unsigned long  getWhatToShow()
        { return 0; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~LSSerializerFilterImpl()
        {}
};



/*#########################################################################
## DOMImplementationLSImpl
#########################################################################*/

/**
 *
 */
class DOMImplementationLSImpl : virtual public DOMImplementationLS
{
public:

    /**
     *
     */
    virtual LSParser &createLSParser(unsigned short /*mode*/,
                                     const DOMString &/*schemaType*/)
                                     throw (dom::DOMException)
        {
        LSParserImpl newParser;
        parser = newParser;
        return parser;
        }


    /**
     *
     */
    virtual LSSerializer &createLSSerializer()
        {
        LSSerializerImpl newSerializer;
        serializer = newSerializer;
        return serializer;
        }


    /**
     *
     */
    virtual LSInput createLSInput()
        {
        LSInput input;
        return input;
        }

    /**
     *
     */
    virtual LSOutput createLSOutput()
        {
        LSOutput output;
        return output;
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~DOMImplementationLSImpl() {}

protected:

    LSParserImpl     parser;
    LSSerializerImpl serializer;
};






}  //namespace ls
}  //namespace dom
}  //namespace w3c
}  //namespace org




#endif   /* __LSIMPL_H__ */

/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/

