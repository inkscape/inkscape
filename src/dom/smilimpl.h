#ifndef __SMILIMPL_H__
#define __SMILIMPL_H__
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


#include "smil.h"
#include "domimpl.h"
#include "views.h"
#include "events.h"


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

/**
 *
 */
class SMILDocumentImpl : virtual public SMILDocument,
                         virtual public DocumentImpl
{
public:


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SMILDocumentImpl();

    /**
     *
     */
    virtual ~SMILDocumentImpl();


};


/*#########################################################################
## SMILElementImpl
#########################################################################*/

/**
 *
 */
class SMILElementImpl : virtual public SMILElement,
                        public ElementImpl
{
public:

    /**
     *
     */
    virtual DOMString getId();

    /**
     *
     */
    virtual void setId(const DOMString &val) throw (dom::DOMException);


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SMILElementImpl();


};


/*#########################################################################
## SMILLayoutElementImpl
#########################################################################*/

/**
 *
 */
class SMILLayoutElementImpl : virtual public SMILLayoutElement,
                              public SMILElementImpl
{
public:

    /**
     *
     */
    virtual DOMString getType();

    /**
     *
     */
    virtual bool getResolved();

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SMILLayoutElementImpl();


};


/*#########################################################################
## SMILTopLayoutElementImpl
#########################################################################*/

/**
 *
 */
class SMILTopLayoutElementImpl : virtual public SMILTopLayoutElement,
                                 public SMILElementImpl
{
public:


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SMILTopLayoutElementImpl();


};


/*#########################################################################
## SMILRootLayoutElementImpl
#########################################################################*/

/**
 *
 */
class SMILRootLayoutElementImpl : virtual public SMILRootLayoutElement,
                                  public SMILElementImpl
{
public:


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SMILRootLayoutElementImpl();


};


/*#########################################################################
## SMILRegionElementImpl
#########################################################################*/

/**
 *
 */
class SMILRegionElementImpl : virtual public SMILRegionElement,
                              public SMILElementImpl
{
public:


    /**
     *
     */
    virtual DOMString getFit();

    /**
     *
     */
    virtual void setFit(const DOMString &val) throw (dom::DOMException);

    /**
     *
     */
    virtual DOMString getTop();

    /**
     *
     */
    virtual void setTop(const DOMString &val) throw (dom::DOMException);

    /**
     *
     */
    virtual long getZIndex();

    /**
     *
     */
    virtual void setZIndex(long val) throw (dom::DOMException);


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SMILRegionElementImpl();


};





/*#########################################################################
## SMILMediaElementImpl
#########################################################################*/

/**
 *
 */
class SMILMediaElementImpl : virtual public SMILMediaElement,
                             public SMILElementImpl
{
public:


    /**
     *
     */
    virtual DOMString getAbstractAttr();

    /**
     *
     */
    virtual void setAbstractAttr(const DOMString &val) throw (dom::DOMException);

    /**
     *
     */
    virtual DOMString getAlt();

    /**
     *
     */
    virtual void setAlt(const DOMString &val) throw (dom::DOMException);

    /**
     *
     */
    virtual DOMString getAuthor();

    /**
     *
     */
    virtual void setAuthor(const DOMString &val) throw (dom::DOMException);

    /**
     *
     */
    virtual DOMString getClipBegin();

    /**
     *
     */
    virtual void setClipBegin(const DOMString &val) throw (dom::DOMException);

    /**
     *
     */
    virtual DOMString getClipEnd();

    /**
     *
     */
    virtual void setClipEnd(const DOMString &val) throw (dom::DOMException);

    /**
     *
     */
    virtual DOMString getCopyright();

    /**
     *
     */
    virtual void setCopyright(const DOMString &val) throw (dom::DOMException);

    /**
     *
     */
    virtual DOMString getLongdesc();

    /**
     *
     */
    virtual void setLongdesc(const DOMString &val) throw (dom::DOMException);

    /**
     *
     */
    virtual DOMString getPort();

    /**
     *
     */
    virtual void setPort(const DOMString &val) throw (dom::DOMException);

    /**
     *
     */
    virtual DOMString getReadIndex();

    /**
     *
     */
    virtual void setReadIndex(const DOMString &val) throw (dom::DOMException);

    /**
     *
     */
    virtual DOMString getRtpformat();

    /**
     *
     */
    virtual void setRtpformat(const DOMString &val) throw (dom::DOMException);

    /**
     *
     */
    virtual DOMString getSrc();

    /**
     *
     */
    virtual void setSrc(const DOMString &val) throw (dom::DOMException);

    /**
     *
     */
    virtual DOMString getStripRepeat();

    /**
     *
     */
    virtual void setStripRepeat(const DOMString &val) throw (dom::DOMException);

    /**
     *
     */
    virtual DOMString getTitle();

    /**
     *
     */
    virtual void setTitle(const DOMString &val) throw (dom::DOMException);

    /**
     *
     */
    virtual DOMString getTransport();

    /**
     *
     */
    virtual void setTransport(const DOMString &val) throw (dom::DOMException);

    /**
     *
     */
    virtual DOMString getType();

    /**
     *
     */
    virtual void setType(const DOMString &val) throw (dom::DOMException);



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SMILMediaElementImpl();


};


/*#########################################################################
## SMILRefElementImpl
#########################################################################*/

/**
 *
 */
class SMILRefElementImpl : virtual public SMILRefElement,
                           public SMILMediaElementImpl
{
public:


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SMILRefElementImpl();


};


/*#########################################################################
## SMILAnimationImpl
#########################################################################*/

/**
 *
 */
class SMILAnimationImpl : virtual public SMILAnimation,
                          public SMILElementImpl
{
public:

    /**
     *
     */
    virtual unsigned short getAdditive();

    /**
     *
     */
    virtual void setAdditive(unsigned short val) throw (dom::DOMException);

    /**
     *
     */
    virtual unsigned short getAccumulate();

    /**
     *
     */
    virtual void setAccumulate(unsigned short val) throw (dom::DOMException);

    /**
     *
     */
    virtual unsigned short getCalcMode();

    /**
     *
     */
    virtual void setCalcMode(unsigned short val) throw (dom::DOMException);

    /**
     *
     */
    virtual DOMString getKeySplines();

    /**
     *
     */
    virtual void setKeySplines(const DOMString &val) throw (dom::DOMException);

    /**
     *
     */
    virtual TimeList getKeyTimes();

    /**
     *
     */
    virtual void setKeyTimes(const TimeList &val) throw (dom::DOMException);

    /**
     *
     */
    virtual DOMString getValues();

    /**
     *
     */
    virtual void setValues(const DOMString &val) throw (dom::DOMException);

    /**
     *
     */
    virtual DOMString getFrom();

    /**
     *
     */
    virtual void setFrom(const DOMString &val) throw (dom::DOMException);

    /**
     *
     */
    virtual DOMString getTo();

    /**
     *
     */
    virtual void setTo(const DOMString &val) throw (dom::DOMException);

    /**
     *
     */
    virtual DOMString getBy();

    /**
     *
     */
    virtual void setBy(const DOMString &val) throw (dom::DOMException);

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SMILAnimationImpl();

protected:

    TimeList keyTimes;


};


/*#########################################################################
## SMILAnimateElementImpl
#########################################################################*/

/**
 *
 */
class SMILAnimateElementImpl : virtual public SMILAnimateElement,
                               public SMILAnimationImpl
{
public:


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SMILAnimateElementImpl();


};


/*#########################################################################
## SMILSetElementImpl
#########################################################################*/

/**
 *
 */
class SMILSetElementImpl : virtual public SMILSetElement,
                           public SMILElementImpl
{
public:

    /**
     *
     */
    virtual DOMString getTo();

    /**
     *
     */
    virtual void setTo(const DOMString &val);

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SMILSetElementImpl();


};


/*#########################################################################
## SMILAnimateMotionElementImpl
#########################################################################*/

/**
 *
 */
class SMILAnimateMotionElementImpl : virtual public SMILAnimateMotionElement,
                                     public SMILAnimateElementImpl
{
public:

    /**
     *
     */
    virtual DOMString getPath();

    /**
     *
     */
    virtual void setPath(const DOMString &val) throw(dom::DOMException);

    /**
     *
     */
    virtual DOMString getOrigin();

    /**
     *
     */
    virtual void setOrigin(const DOMString &val) throw(dom::DOMException);


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SMILAnimateMotionElementImpl();


};


/*#########################################################################
## SMILAnimateColorElementImpl
#########################################################################*/

/**
 *
 */
class SMILAnimateColorElementImpl : virtual public SMILAnimateColorElement,
                                    public SMILAnimationImpl
{
public:


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SMILAnimateColorElementImpl();


};





/*#########################################################################
## SMILSwitchElementImpl
#########################################################################*/

/**
 *
 */
class SMILSwitchElementImpl : virtual public SMILSwitchElement,
                              public SMILElementImpl
{
public:

    /**
     *
     */
    virtual ElementPtr getSelectedElement();

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SMILSwitchElementImpl();


};





}  //namespace smil
}  //namespace dom
}  //namespace w3c
}  //namespace org

#endif    /* __SMILIMPL_H__ */

/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/

