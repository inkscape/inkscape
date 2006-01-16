/*
 * Copyright (c) 2004 World Wide Web Consortium,
 *
 * (Massachusetts Institute of Technology, European Research Consortium for
 * Informatics and Mathematics, Keio University). All Rights Reserved. This
 * work is distributed under the W3C(r) Software License [1] in the hope that
 * it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * [1] http://www.w3.org/Consortium/Legal/2002/copyright-software-20021231
 */

// File: http://www.w3.org/TR/2004/REC-DOM-Level-3-LS-20040407/ls.idl

#ifndef __SVGLSIMPL_H__
#define __SVGLSIMPL_H__

#include "lsimpl.h"
#include "svgparser.h"


namespace org {
namespace w3c {
namespace dom {
namespace ls  {




/*#########################################################################
## SVGLSParser
#########################################################################*/

/**
 *
 */
class SVGLSParserImpl : virtual public LSParserImpl
{
public:

    /**
     *
     */
    virtual Document *parse(const LSInput &input)
                            throw(dom::DOMException, LSException);
                            


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGLSParserImpl()
        {}

    /**
     *
     */
    virtual ~SVGLSParserImpl()
        {}
    


    //##################
    //# Internals
    //##################


protected:


        
};


/*#########################################################################
## SVGLSSerializerImpl
#########################################################################*/

/**
 *
 */
class SVGLSSerializerImpl : virtual public LSSerializerImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGLSSerializerImpl()
        {
        }

    /**
     *
     */
    virtual ~SVGLSSerializerImpl()
        {}



protected:

    /**
     *  Overload me to change behaviour
     */
    virtual void writeNode(const Node *nodeArg);

    


};





/*#########################################################################
## SVGDOMImplementationLSImpl
#########################################################################*/

/**
 *
 */
class SVGDOMImplementationLSImpl : virtual public DOMImplementationLS
{
public:

    /**
     *
     */
    virtual LSParser &createLSParser(unsigned short mode,
                                     const DOMString &schemaType)
                                     throw (dom::DOMException)
        {
        SVGLSParserImpl newParser;
        parser = newParser;
        return parser;
        }


    /**
     *
     */
    virtual LSSerializer &createLSSerializer()
        {
        SVGLSSerializerImpl newSerializer;
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
    SVGDOMImplementationLSImpl() {}

    /**
     *
     */
    virtual ~SVGDOMImplementationLSImpl() {}

protected:

    SVGLSParserImpl     parser;
    SVGLSSerializerImpl serializer;
};







}  //namespace ls
}  //namespace dom
}  //namespace w3c
}  //namespace org




#endif   /* __SVGLSIMPL_H__ */

/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/

