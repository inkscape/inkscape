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

#include "svglsimpl.h"

#include <stdarg.h>

namespace org {
namespace w3c {
namespace dom {
namespace svg {




/*#########################################################################
## SVGLSParserImpl
#########################################################################*/


/**
 *
 */
Document *SVGLSParserImpl::parse(const LSInput &inputArg)
                            throw(dom::DOMException, LSException)
{
    Document *doc = LSParserImpl::parse(inputArg);

    if (!doc)
        {
        return NULL;
        }

    svg::SvgParser svgParser;

    Document *svgdoc = svgParser.parse(doc);

    delete doc;

    return svgdoc;
}





/*#########################################################################
## SVGLSSerializerImpl
#########################################################################*/


/**
 *
 */
void SVGLSSerializerImpl::writeNode(const Node *nodeArg)
{
    LSSerializerImpl::writeNode(nodeArg);
}









}  //namespace svg
}  //namespace dom
}  //namespace w3c
}  //namespace org





/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/

