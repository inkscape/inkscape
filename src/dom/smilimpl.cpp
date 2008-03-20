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
 * Copyright (C) 2005-2008 Bob Jamison
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


#include "smilimpl.h"


namespace org
{
namespace w3c
{
namespace dom
{
namespace smil
{


/*#########################################################################
## SMILDocumentImpl
#########################################################################*/


//##################
//# Non-API methods
//##################

/**
 *
 */
SMILDocumentImpl::SMILDocumentImpl() : DocumentImpl(NULL, "", "", NULL)
{
}

/**
 *
 */
SMILDocumentImpl::~SMILDocumentImpl()
{
}




/*#########################################################################
## SMILElementImpl
#########################################################################*/

/**
 *
 */
DOMString SMILElementImpl::getId()
{
    return DOMString("");
}

/**
 *
 */
void SMILElementImpl::setId(const DOMString &val) throw (dom::DOMException)
{
}


//##################
//# Non-API methods
//##################

/**
 *
 */
SMILElementImpl::~SMILElementImpl()
{
}




/*#########################################################################
## SMILLayoutElementImpl
#########################################################################*/

/**
 *
 */
DOMString SMILLayoutElementImpl::getType()
{
    return DOMString("");
}

/**
 *
 */
bool SMILLayoutElementImpl::getResolved()
{
    return false;
}

//##################
//# Non-API methods
//##################

/**
 *
 */
SMILLayoutElementImpl::~SMILLayoutElementImpl()
{
}




/*#########################################################################
## SMILTopLayoutElementImpl
#########################################################################*/


//##################
//# Non-API methods
//##################

/**
 *
 */
SMILTopLayoutElementImpl::~SMILTopLayoutElementImpl()
{
}




/*#########################################################################
## SMILRootLayoutElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################

/**
 *
 */
SMILRootLayoutElementImpl::~SMILRootLayoutElementImpl()
{
}




/*#########################################################################
## SMILRegionElementImpl
#########################################################################*/

/**
 *
 */
DOMString SMILRegionElementImpl::getFit()
{
    return DOMString("");
}

/**
 *
 */
void SMILRegionElementImpl::setFit(const DOMString &val) throw (dom::DOMException)
{
}

/**
 *
 */
DOMString SMILRegionElementImpl::getTop()
{
    return DOMString("");
}

/**
 *
 */
void SMILRegionElementImpl::setTop(const DOMString &val) throw (dom::DOMException)
{
}

/**
 *
 */
long SMILRegionElementImpl::getZIndex()
{
    return 0L;
}

/**
 *
 */
void SMILRegionElementImpl::setZIndex(long val) throw (dom::DOMException)
{
}


//##################
//# Non-API methods
//##################

/**
 *
 */
SMILRegionElementImpl::~SMILRegionElementImpl()
{
}




/*#########################################################################
## SMILMediaElementImpl
#########################################################################*/

/**
 *
 */
DOMString SMILMediaElementImpl::getAbstractAttr()
{
    return DOMString("");
}

/**
 *
 */
void SMILMediaElementImpl::setAbstractAttr(const DOMString &val)
                                   throw (dom::DOMException)
{
}

/**
 *
 */
DOMString SMILMediaElementImpl::getAlt()
{
    return DOMString("");
}

/**
 *
 */
void SMILMediaElementImpl::setAlt(const DOMString &val)
                                   throw (dom::DOMException)
{
}

/**
 *
 */
DOMString SMILMediaElementImpl::getAuthor()
{
    return DOMString("");
}

/**
 *
 */
void SMILMediaElementImpl::setAuthor(const DOMString &val)
                                  throw (dom::DOMException)
{
}

/**
 *
 */
DOMString SMILMediaElementImpl::getClipBegin()
{
    return DOMString("");
}

/**
 *
 */
void SMILMediaElementImpl::setClipBegin(const DOMString &val)
                                 throw (dom::DOMException)
{
}

/**
 *
 */
DOMString SMILMediaElementImpl::getClipEnd()
{
    return DOMString("");
}

/**
 *
 */
void SMILMediaElementImpl::setClipEnd(const DOMString &val)
                               throw (dom::DOMException)
{
}

/**
 *
 */
DOMString SMILMediaElementImpl::getCopyright()
{
    return DOMString("");
}

/**
 *
 */
void SMILMediaElementImpl::setCopyright(const DOMString &val)
                                  throw (dom::DOMException)
{
}

/**
 *
 */
DOMString SMILMediaElementImpl::getLongdesc()
{
    return DOMString("");
}

/**
 *
 */
void SMILMediaElementImpl::setLongdesc(const DOMString &val)
                               throw (dom::DOMException)
{
}

/**
 *
 */
DOMString SMILMediaElementImpl::getPort()
{
    return DOMString("");
}

/**
 *
 */
void SMILMediaElementImpl::setPort(const DOMString &val)
                                 throw (dom::DOMException)
{
}

/**
 *
 */
DOMString SMILMediaElementImpl::getReadIndex()
{
    return DOMString("");
}

/**
 *
 */
void SMILMediaElementImpl::setReadIndex(const DOMString &val)
                                throw (dom::DOMException)
{
}

/**
 *
 */
DOMString SMILMediaElementImpl::getRtpformat()
{
    return DOMString("");
}

/**
 *
 */
void SMILMediaElementImpl::setRtpformat(const DOMString &val)
                              throw (dom::DOMException)
{
}

/**
 *
 */
DOMString SMILMediaElementImpl::getSrc()
{
    return DOMString("");
}

/**
 *
 */
void SMILMediaElementImpl::setSrc(const DOMString &val)
                                 throw (dom::DOMException)
{
}

/**
 *
 */
DOMString SMILMediaElementImpl::getStripRepeat()
{
    return DOMString("");
}

/**
 *
 */
void SMILMediaElementImpl::setStripRepeat(const DOMString &val)
                                 throw (dom::DOMException)
{
}

/**
 *
 */
DOMString SMILMediaElementImpl::getTitle()
{
    return DOMString("");
}

/**
 *
 */
void SMILMediaElementImpl::setTitle(const DOMString &val)
                                throw (dom::DOMException)
{
}

/**
 *
 */
DOMString SMILMediaElementImpl::getTransport()
{
    return DOMString("");
}

/**
 *
 */
void SMILMediaElementImpl::setTransport(const DOMString &val)
                                 throw (dom::DOMException)
{
}

/**
 *
 */
DOMString SMILMediaElementImpl::getType()
{
    return DOMString("");
}

/**
 *
 */
void SMILMediaElementImpl::setType(const DOMString &val)
                                      throw (dom::DOMException)
{
}



//##################
//# Non-API methods
//##################

/**
 *
 */
SMILMediaElementImpl::~SMILMediaElementImpl()
{
}




/*#########################################################################
## SMILRefElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################

/**
 *
 */
SMILRefElementImpl::~SMILRefElementImpl()
{
}




/*#########################################################################
## SMILAnimationImpl
#########################################################################*/


/**
 *
 */
unsigned short SMILAnimationImpl::getAdditive()
{
    return 0;
}

/**
 *
 */
void SMILAnimationImpl::setAdditive(unsigned short val)
                                 throw (dom::DOMException)
{
}

/**
 *
 */
unsigned short SMILAnimationImpl::getAccumulate()
{
    return 0;
}

/**
 *
 */
void SMILAnimationImpl::setAccumulate(unsigned short val)
                                throw (dom::DOMException)
{
}

/**
 *
 */
unsigned short SMILAnimationImpl::getCalcMode()
{
    return 0;
}

/**
 *
 */
void SMILAnimationImpl::setCalcMode(unsigned short val)
                                  throw (dom::DOMException)
{
}

/**
 *
 */
DOMString SMILAnimationImpl::getKeySplines()
{
    return DOMString("");
}

/**
 *
 */
void SMILAnimationImpl::setKeySplines(const DOMString &val)
                                throw (dom::DOMException)
{
}

/**
 *
 */
TimeList SMILAnimationImpl::getKeyTimes()
{
    return keyTimes;
}

/**
 *
 */
void SMILAnimationImpl::setKeyTimes(const TimeList &val)
                               throw (dom::DOMException)
{
    keyTimes = val;
}

/**
 *
 */
DOMString SMILAnimationImpl::getValues()
{
    return DOMString("");
}

/**
 *
 */
void SMILAnimationImpl::setValues(const DOMString &val)
                               throw (dom::DOMException)
{
}

/**
 *
 */
DOMString SMILAnimationImpl::getFrom()
{
    return DOMString("");
}

/**
 *
 */
void SMILAnimationImpl::setFrom(const DOMString &val)
                               throw (dom::DOMException)
{
}

/**
 *
 */
DOMString SMILAnimationImpl::getTo()
{
    return DOMString("");
}

/**
 *
 */
void SMILAnimationImpl::setTo(const DOMString &val)
                              throw (dom::DOMException)
{
}

/**
 *
 */
DOMString SMILAnimationImpl::getBy()
{
    return DOMString("");
}

/**
 *
 */
void SMILAnimationImpl::setBy(const DOMString &val)
                                  throw (dom::DOMException)
{
}

//##################
//# Non-API methods
//##################

/**
 *
 */
SMILAnimationImpl::~SMILAnimationImpl()
{
}




/*#########################################################################
## SMILAnimateElementImpl
#########################################################################*/


//##################
//# Non-API methods
//##################

/**
 *
 */
SMILAnimateElementImpl::~SMILAnimateElementImpl()
{
}




/*#########################################################################
## SMILSetElementImpl
#########################################################################*/

/**
 *
 */
DOMString SMILSetElementImpl::getTo()
{
    return DOMString("");
}

/**
 *
 */
void SMILSetElementImpl::setTo(const DOMString &val)
{
}

//##################
//# Non-API methods
//##################

/**
 *
 */
SMILSetElementImpl::~SMILSetElementImpl()
{
}




/*#########################################################################
## SMILAnimateMotionElementImpl
#########################################################################*/

/**
 *
 */
DOMString SMILAnimateMotionElementImpl::getPath()
{
    return DOMString("");
}

/**
 *
 */
void SMILAnimateMotionElementImpl::setPath(const DOMString &val)
                                      throw(dom::DOMException)
{
}

/**
 *
 */
DOMString SMILAnimateMotionElementImpl::getOrigin()
{
    return DOMString("");
}

/**
 *
 */
void SMILAnimateMotionElementImpl::setOrigin(const DOMString &val)
                                         throw(dom::DOMException)
{
}


//##################
//# Non-API methods
//##################

/**
 *
 */
SMILAnimateMotionElementImpl::~SMILAnimateMotionElementImpl()
{
}




/*#########################################################################
## SMILAnimateColorElementImpl
#########################################################################*/


//##################
//# Non-API methods
//##################

/**
 *
 */
SMILAnimateColorElementImpl::~SMILAnimateColorElementImpl()
{
}







/*#########################################################################
## SMILSwitchElementImpl
#########################################################################*/


/**
 *
 */
Element *SMILSwitchElementImpl::getSelectedElement()
{
    return NULL;
}

//##################
//# Non-API methods
//##################

/**
 *
 */
SMILSwitchElementImpl::~SMILSwitchElementImpl()
{
}










}  //namespace smil
}  //namespace dom
}  //namespace w3c
}  //namespace org




/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/

