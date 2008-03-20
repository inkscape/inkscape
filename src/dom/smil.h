#ifndef __SMIL_H__
#define __SMIL_H__
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


#include "dom.h"
#include "views.h"
#include "events.h"

#include <vector>

namespace org
{
namespace w3c
{
namespace dom
{
namespace smil
{




//Local definitions
typedef dom::DOMString DOMString;
typedef dom::Element Element;
typedef dom::NodeList NodeList;
typedef dom::Document Document;

//forward declarations
class ElementExclusiveTimeContainer;
class ElementLayout;
class ElementParallelTimeContainer;
class ElementSequentialTimeContainer;
class ElementSyncBehavior;
class ElementTargetAttributes;
class ElementTest;
class ElementTime;
class ElementTimeContainer;
class ElementTimeControl;
class ElementTimeManipulation;
class SMILAnimateColorElement;
class SMILAnimateElement;
class SMILAnimateMotionElement;
class SMILAnimation;
class SMILDocument;
class SMILElement;
class SMILLayoutElement;
class SMILMediaElement;
class SMILRefElement;
class SMILRegionElement;
class SMILRegionInterface;
class SMILRootLayoutElement;
class SMILSetElement;
class SMILSwitchElement;
class SMILTopLayoutElement;
class Time;
class TimeEvent;
class TimeList;



/*#########################################################################
###########################################################################
##  D A T A    T Y P E S
###########################################################################
#########################################################################*/



/*#########################################################################
## ElementLayout
#########################################################################*/

/**
 *
 */
class ElementLayout
{
public:


    /**
     *
     */
    virtual DOMString getTitle()
        { return title; }

    /**
     *
     */
    virtual void setTitle(const DOMString &val) throw(dom::DOMException)
        { title = val; }

    /**
     *
     */
    virtual DOMString getBackgroundColor()
        { return backgroundColor; }

    /**
     *
     */
    virtual void setBackgroundColor(const DOMString &val) throw(dom::DOMException)
        { backgroundColor = val; }

    /**
     *
     */
    virtual long getHeight()
        { return height; }

    /**
     *
     */
    virtual void setHeight(long val) throw(dom::DOMException)
        { height = val; }

    /**
     *
     */
    virtual long getWidth()
        { return width; }

    /**
     *
     */
    virtual void setWidth(long val) throw(dom::DOMException)
        { width = val; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    ElementLayout() {}

    /**
     *
     */
    ElementLayout(const ElementLayout &other)
        {
        title           = other.title;
        backgroundColor = other.backgroundColor;
        height          = other.height;
        width           = other.width;
        }

    /**
     *
     */
    virtual ~ElementLayout() {}

protected:

    DOMString title;
    DOMString backgroundColor;
    long height;
    long width;

};


/*#########################################################################
## SMILRegionInterface
#########################################################################*/

/**
 *
 */
class SMILRegionInterface
{
public:

    /**
     *
     */
    virtual SMILRegionElement *getRegion()
        { return regionElement; }
    /**
     *
     */
    virtual void setRegion(const SMILRegionElement *val)
        { regionElement = (SMILRegionElement *)val; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SMILRegionInterface()
        { regionElement = NULL; }

    /**
     *
     */
    SMILRegionInterface(const SMILRegionInterface &other)
       { regionElement = other.regionElement; }

    /**
     *
     */
    virtual ~SMILRegionInterface() {}

protected:

    SMILRegionElement *regionElement;

};


/*#########################################################################
## Time
#########################################################################*/

/**
 *
 */
class Time
{
public:


    /**
     *
     */
    virtual bool getResolved()
        { return resolved; }

    /**
     *
     */
    virtual double getResolvedOffset()
        { return resolvedOffset; }

    typedef enum
        {
        SMIL_TIME_INDEFINITE           = 0,
        SMIL_TIME_OFFSET               = 1,
        SMIL_TIME_SYNC_BASED           = 2,
        SMIL_TIME_EVENT_BASED          = 3,
        SMIL_TIME_WALLCLOCK            = 4,
        SMIL_TIME_MEDIA_MARKER         = 5
        } TimeTypes;



    /**
     *
     */
    virtual unsigned short getTimeType()
        { return timeType; }


    /**
     *
     */
    virtual double getOffset()
        { return offset; }

    /**
     *
     */
    virtual void setOffset(double val) throw (dom::DOMException)
        { offset = val; }

    /**
     *
     */
    virtual Element *getBaseElement()
        { return baseElement; }

    /**
     *
     */
    virtual void setBaseElement(const Element *val) throw (dom::DOMException)
        { baseElement = (Element *)val; }

    /**
     *
     */
    virtual bool getBaseBegin()
        { return baseBegin; }

    /**
     *
     */
    virtual void setBaseBegin(bool val) throw (dom::DOMException)
        { baseBegin = val; }

    /**
     *
     */
    virtual DOMString getEvent()
        { return eventStr; }

    /**
     *
     */
    virtual void setEvent(const DOMString &val) throw (dom::DOMException)
        { eventStr = val; }

    /**
     *
     */
    virtual DOMString getMarker()
        { return marker; }

    /**
     *
     */
    virtual void setMarker(const DOMString &val) throw (dom::DOMException)
        { marker = val; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    Time()
        {
        resolved       = false;
        resolvedOffset = 0.0;
        timeType       = SMIL_TIME_INDEFINITE;
        offset         = 0.0;
        baseElement    = NULL;
        baseBegin      = false;
        eventStr       = "";
        marker         = "";
        }

    /**
     *
     */
    Time(const Time &other)
       {
        resolved       = other.resolved;
        resolvedOffset = other.resolvedOffset;
        timeType       = other.timeType;
        offset         = other.offset;
        baseElement    = other.baseElement;
        baseBegin      = other.baseBegin;
        eventStr       = other.eventStr;
        marker         = other.marker;
       }

    /**
     *
     */
    virtual ~Time() {}

protected:

    bool           resolved;
    double         resolvedOffset;
    unsigned short timeType;
    double         offset;
    Element *      baseElement;
    bool           baseBegin;
    DOMString      eventStr;
    DOMString      marker;

};


/*#########################################################################
## TimeList
#########################################################################*/

/**
 *
 */
class TimeList
{
public:


    /**
     *
     */
    virtual Time item(unsigned long index)
        {
        if (index >=items.size())
            {
            Time tim;
            return tim;
            }
        return items[index];
        }

    /**
     *
     */
    virtual unsigned long getLength()
        {
        return items.size();
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    TimeList() {}

    /**
     *
     */
    TimeList(const TimeList &other)
        {
        items = other.items;
        }

    /**
     *
     */
    virtual ~TimeList() {}

protected:

    std::vector<Time>items;


};


/*#########################################################################
## ElementTime
#########################################################################*/

/**
 *
 */
class ElementTime
{
public:


    /**
     *
     */
    virtual TimeList getBegin()
        { return beginTime; }

    /**
     *
     */
    virtual void setBegin(const TimeList &val) throw(dom::DOMException)
        { beginTime = val; }
    /**
     *
     */
    virtual TimeList getEnd()
        { return endTime; }

    /**
     *
     */
    virtual void setEnd(const TimeList &val) throw(dom::DOMException)
        { endTime = val; }

    /**
     *
     */
    virtual double getDur()
        { return dur; }

    /**
     *
     */
    virtual void setDur(double val) throw(dom::DOMException)
        { dur = val; }

    typedef enum
        {
        RESTART_ALWAYS                 = 0,
        RESTART_NEVER                  = 1,
        RESTART_WHEN_NOT_ACTIVE        = 2
        } RestartTypes;

    /**
     *
     */
    virtual unsigned short getRestart()
        { return restart; }

    /**
     *
     */
    virtual void setRestart(unsigned short val) throw (dom::DOMException)
        { restart = val; }


    // fillTypes
    typedef enum
        {
        FILL_REMOVE                    = 0,
        FILL_FREEZE                    = 1
        } FillTypes;


    /**
     *
     */
    virtual unsigned short getFill()
        { return fill; }

    /**
     *
     */
    virtual void setFill(unsigned short val) throw (dom::DOMException)
        { fill = val; }

    /**
     *
     */
    virtual double getRepeatCount()
        { return repeatCount; }

    /**
     *
     */
    virtual void setRepeatCount(double val) throw (dom::DOMException)
        { repeatCount = val; }

    /**
     *
     */
    virtual double getRepeatDur()
        { return repeatDur; }

    /**
     *
     */
    virtual void setRepeatDur(double val) throw (dom::DOMException)
        { repeatDur = val; }

    /**
     *
     */
    virtual bool beginElement()
        {
        return true;
        }

    /**
     *
     */
    virtual bool endElement()
        {
        return true;
        }

    /**
     *
     */
    virtual void pauseElement()
        {
        }

    /**
     *
     */
    virtual void resumeElement()
        {
        }

    /**
     *
     */
    virtual void seekElement(double &/*seekTo*/)
        {
        }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    ElementTime()
        {
        dur         = 0.0;
        restart     = RESTART_ALWAYS;
        fill        = FILL_REMOVE;
        repeatCount = 0.0;
        repeatDur   = 0.0;
        }

    /**
     *
     */
    ElementTime(const ElementTime &other)
       {
       beginTime   = other.beginTime;
       endTime     = other.endTime;
       dur         = other.dur;
       restart     = other.restart;
       fill        = other.fill;
       repeatCount = other.repeatCount;
       repeatDur   = other.repeatDur;
       }

    /**
     *
     */
    virtual ~ElementTime() {}


protected:

    TimeList       beginTime;
    TimeList       endTime;
    double         dur;
    unsigned short restart;
    unsigned short fill;
    double         repeatCount;
    double         repeatDur;


};


/*#########################################################################
## ElementTimeManipulation
#########################################################################*/

/**
 *
 */
class ElementTimeManipulation
{
public:

    /**
     *
     */
    virtual double getSpeed()
        { return speed; }
    /**
     *
     */
    virtual void setSpeed(double val) throw (dom::DOMException)
        { speed = val; }
    /**
     *
     */
    virtual double getAccelerate()
        { return accelerate; }

    /**
     *
     */
    virtual void setAccelerate(double val) throw (dom::DOMException)
        { accelerate = val; }

    /**
     *
     */
    virtual double getDecelerate()
        { return decelerate; }

    /**
     *
     */
    virtual void setDecelerate(double val) throw (dom::DOMException)
        { decelerate = val; }

    /**
     *
     */
    virtual bool getAutoReverse()
        { return autoReverse; }

    /**
     *
     */
    virtual void setAutoReverse(bool val) throw (dom::DOMException)
        { autoReverse = val; }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    ElementTimeManipulation()
        {
        speed       = 0.0;
        accelerate  = 0.0;
        decelerate  = 0.0;
        autoReverse = false;
        }

    /**
     *
     */
    ElementTimeManipulation(const ElementTimeManipulation &other)
        {
        speed       = other.speed;
        accelerate  = other.accelerate;
        decelerate  = other.decelerate;
        autoReverse = other.autoReverse;
        }

    /**
     *
     */
    virtual ~ElementTimeManipulation() {}


protected:

    double speed;
    double accelerate;
    double decelerate;
    bool autoReverse;

};


/*#########################################################################
## ElementTimeContainer
#########################################################################*/

/**
 *
 */
class ElementTimeContainer : public ElementTime
{
public:


    /**
     *
     */
    virtual NodeList getTimeChildren()
        {
        NodeList list;
        return list;
        }

    /**
     *
     */
    virtual NodeList getActiveChildrenAt(double /*instant*/)
        {
        NodeList list;
        return list;
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    ElementTimeContainer() {}

    /**
     *
     */
    ElementTimeContainer(const ElementTimeContainer &other) : ElementTime(other)
        {
        }

    /**
     *
     */
    virtual ~ElementTimeContainer() {}

protected:


};


/*#########################################################################
## ElementSyncBehavior
#########################################################################*/

/**
 *
 */
class ElementSyncBehavior
{
public:

    /**
     *
     */
    virtual DOMString getSyncBehavior()
        { return syncBehavior; }

    /**
     *
     */
    virtual double getSyncTolerance()
        { return syncTolerance; }

    /**
     *
     */
    virtual DOMString getDefaultSyncBehavior()
        { return defaultSyncBehavior; }

    /**
     *
     */
    virtual double getDefaultSyncTolerance()
        { return defaultSyncTolerance; }

    /**
     *
     */
    virtual bool getSyncMaster()
        { return syncMaster; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    ElementSyncBehavior()
        {
        syncBehavior         = "";
        syncTolerance        = 0.0;
        defaultSyncBehavior  = "";
        defaultSyncTolerance = 0.0;
        syncMaster           = false;
        }

    /**
     *
     */
    ElementSyncBehavior(const ElementSyncBehavior &other)
        {
        syncBehavior         = other.syncBehavior;
        syncTolerance        = other.syncTolerance;
        defaultSyncBehavior  = other.defaultSyncBehavior;
        defaultSyncTolerance = other.defaultSyncTolerance;
        syncMaster           = other.syncMaster;
        }

    /**
     *
     */
    virtual ~ElementSyncBehavior() {}

protected:

    DOMString syncBehavior;
    double    syncTolerance;
    DOMString defaultSyncBehavior;
    double    defaultSyncTolerance;
    bool      syncMaster;

};


/*#########################################################################
## ElementParallelTimeContainer
#########################################################################*/

/**
 *
 */
class ElementParallelTimeContainer : public ElementTimeContainer
{
public:

    /**
     *
     */
    virtual DOMString getEndSync()
        { return endSync; }

    /**
     *
     */
    virtual void setEndSync(const DOMString &val) throw (dom::DOMException)
        { endSync = val; }

    /**
     *
     */
    virtual double getImplicitDuration()
        { return implicitDuration; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    ElementParallelTimeContainer()
        {
        endSync          = "";
        implicitDuration = 0.0;
        }

    /**
     *
     */
    ElementParallelTimeContainer(const ElementParallelTimeContainer &other)
                                : ElementTimeContainer(other)
        {
        endSync          = other.endSync;
        implicitDuration = other.implicitDuration;
        }

    /**
     *
     */
    virtual ~ElementParallelTimeContainer() {}

protected:

    DOMString endSync;
    double implicitDuration;

};


/*#########################################################################
## ElementSequentialTimeContainer
#########################################################################*/

/**
 *
 */
class ElementSequentialTimeContainer : public ElementTimeContainer
{
public:


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    ElementSequentialTimeContainer() {}

    /**
     *
     */
    ElementSequentialTimeContainer(const ElementSequentialTimeContainer &other)
                             : ElementTimeContainer(other)
        {
        }

    /**
     *
     */
    virtual ~ElementSequentialTimeContainer() {}


};


/*#########################################################################
## ElementExclusiveTimeContainer
#########################################################################*/

/**
 *
 */
class ElementExclusiveTimeContainer : public ElementTimeContainer
{
public:

    /**
     *
     */
    virtual DOMString getEndSync()
        { return endSync; }

    /**
     *
     */
    virtual void setEndSync(const DOMString &val) throw (dom::DOMException)
        { endSync = val; }

    /**
     *
     */
    virtual NodeList getPausedElements()
        { return pausedElements; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    ElementExclusiveTimeContainer() {}

    /**
     *
     */
    ElementExclusiveTimeContainer(const ElementExclusiveTimeContainer &other)
                    : ElementTimeContainer(other)
        {
        endSync        = other.endSync;
        pausedElements = other.pausedElements;
        }

    /**
     *
     */
    virtual ~ElementExclusiveTimeContainer() {}

protected:

    DOMString endSync;
    NodeList pausedElements;


};


/*#########################################################################
## ElementTimeControl
#########################################################################*/

/**
 *
 */
class ElementTimeControl
{
public:

    /**
     *
     */
    virtual bool beginElement() throw(dom::DOMException)
        {
        return true;
        }

    /**
     *
     */
    virtual bool beginElementAt(double /*offset*/) throw(dom::DOMException)
        {
        return true;
        }

    /**
     *
     */
    virtual bool endElement() throw(dom::DOMException)
        {
        return true;
        }

    /**
     *
     */
    virtual bool endElementAt(double /*offset*/) throw(dom::DOMException)
        {
        return true;
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    ElementTimeControl() {}

    /**
     *
     */
    ElementTimeControl(const ElementTimeControl &/*other*/)
        {
        }

    /**
     *
     */
    virtual ~ElementTimeControl() {}


};


/*#########################################################################
## ElementTargetAttributes
#########################################################################*/

/**
 *
 */
class ElementTargetAttributes
{
public:


    /**
     *
     */
    virtual DOMString getAttributeName()
        { return attributeName; }

    /**
     *
     */
    virtual void setAttributeName(const DOMString &val)
        { attributeName = val; }

    typedef enum
        {
        ATTRIBUTE_TYPE_AUTO            = 0,
        ATTRIBUTE_TYPE_CSS             = 1,
        ATTRIBUTE_TYPE_XML             = 2
        } AttributeTypes;

    /**
     *
     */
    virtual unsigned short getAttributeType()
        { return attributeType; }

    /**
     *
     */
    virtual void setAttributeType(unsigned short val)
        { attributeType = val; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    ElementTargetAttributes()
        {
        attributeName = "";
        attributeType = ATTRIBUTE_TYPE_AUTO;
        }

    /**
     *
     */
    ElementTargetAttributes(const ElementTargetAttributes &other)
       {
       attributeName = other.attributeName;
       attributeType = other.attributeType;
       }

    /**
     *
     */
    virtual ~ElementTargetAttributes() {}

protected:

    DOMString      attributeName;
    unsigned short attributeType;

};


/*#########################################################################
## ElementTest
#########################################################################*/

/**
 *
 */
class ElementTest
{
public:


    /**
     *
     */
    virtual long getSystemBitrate()
        { return systemBitrate; }

    /**
     *
     */
    virtual void setSystemBitrate(long val) throw (dom::DOMException)
        { systemBitrate = val; }

    /**
     *
     */
    virtual bool getSystemCaptions()
        { return systemCaptions; }

    /**
     *
     */
    virtual void setSystemCaptions(bool val) throw (dom::DOMException)
        { systemCaptions = val; }

    /**
     *
     */
    virtual DOMString getSystemLanguage()
        { return systemLanguage; }

    /**
     *
     */
    virtual void setSystemLanguage(const DOMString &val) throw (dom::DOMException)
        { systemLanguage = val; }

    /**
     *
     */
    virtual bool getSystemRequired()
        { return systemRequired; }

    /**
     *
     */
    virtual bool getSystemScreenSize()
        { return systemScreenSize; }

    /**
     *
     */
    virtual bool getSystemScreenDepth()
        { return systemScreenDepth; }

    /**
     *
     */
    virtual DOMString getSystemOverdubOrSubtitle()
        { return systemOverdubOrSubtitle; }

    /**
     *
     */
    virtual void setSystemOverdubOrSubtitle(const DOMString &val) throw (dom::DOMException)
        { systemOverdubOrSubtitle = val; }

    /**
     *
     */
    virtual bool getSystemAudioDesc()
        { return systemBitrate; }

    /**
     *
     */
    virtual void setSystemAudioDesc(bool val)  throw (dom::DOMException)
        { systemAudioDesc = val; }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    ElementTest()
        {
        systemBitrate           = 0;
        systemCaptions          = false;
        systemLanguage          = "";
        systemRequired          = false;
        systemScreenSize        = false;
        systemScreenDepth       = false;
        systemOverdubOrSubtitle = "";
        systemAudioDesc         = false;
        }

    /**
     *
     */
    ElementTest(const ElementTest &other)
       {
        systemBitrate           = other.systemBitrate;
        systemCaptions          = other.systemCaptions;
        systemLanguage          = other.systemLanguage;
        systemRequired          = other.systemRequired;
        systemScreenSize        = other.systemScreenSize;
        systemScreenDepth       = other.systemScreenDepth;
        systemOverdubOrSubtitle = other.systemOverdubOrSubtitle;
        systemAudioDesc         = other.systemAudioDesc;
       }

    /**
     *
     */
    virtual ~ElementTest() {}


protected:


    long      systemBitrate;
    bool      systemCaptions;
    DOMString systemLanguage;
    bool      systemRequired;
    bool      systemScreenSize;
    bool      systemScreenDepth;
    DOMString systemOverdubOrSubtitle;
    bool      systemAudioDesc;
};


/*#########################################################################
## TimeEvent
#########################################################################*/

/**
 *
 */
class TimeEvent : public events::Event
{
public:

    /**
     *
     */
    virtual views::AbstractView getView()
        { return view; }

    /**
     *
     */
    virtual long getDetail()
        { return detail; }

    /**
     *
     */
    virtual void initTimeEvent(const DOMString &/*typeArg*/,
                               const views::AbstractView */*viewArg*/,
                               long /*detailArg*/)
        {
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    TimeEvent()
        {
        detail = 0;
        }

    /**
     *
     */
    TimeEvent(const TimeEvent &other) : events::Event(other)
        {
        view   = other.view;
        detail = other.detail;
        }

    /**
     *
     */
    virtual ~TimeEvent() {}


protected:

    views::AbstractView view;
    long detail;


};




/*#########################################################################
###########################################################################
##  I N T E R F A C E    T Y P E S
###########################################################################
#########################################################################*/




/*#########################################################################
## SMILDocument
#########################################################################*/

/**
 *
 */
class SMILDocument : virtual public Document,
                     public ElementSequentialTimeContainer
{
public:


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SMILDocument() {}


};


/*#########################################################################
## SMILElement
#########################################################################*/

/**
 *
 */
class SMILElement : virtual public Element
{
public:

    /**
     *
     */
    virtual DOMString getId() =0;

    /**
     *
     */
    virtual void setId(const DOMString &val) throw (dom::DOMException) =0;


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SMILElement() {}


};


/*#########################################################################
## SMILLayoutElement
#########################################################################*/

/**
 *
 */
class SMILLayoutElement : virtual public SMILElement
{
public:

    /**
     *
     */
    virtual DOMString getType() =0;

    /**
     *
     */
    virtual bool getResolved() =0;

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SMILLayoutElement() {}


};


/*#########################################################################
## SMILTopLayoutElement
#########################################################################*/

/**
 *
 */
class SMILTopLayoutElement : virtual public SMILElement,
                             virtual public ElementLayout
{
public:


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SMILTopLayoutElement() {}


};


/*#########################################################################
## SMILRootLayoutElement
#########################################################################*/

/**
 *
 */
class SMILRootLayoutElement : virtual public SMILElement,
                              virtual public ElementLayout
{
public:


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SMILRootLayoutElement() {}


};


/*#########################################################################
## SMILRegionElement
#########################################################################*/

/**
 *
 */
class SMILRegionElement : virtual public SMILElement,
                          virtual public ElementLayout
{
public:


    /**
     *
     */
    virtual DOMString getFit() =0;

    /**
     *
     */
    virtual void setFit(const DOMString &val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getTop() =0;

    /**
     *
     */
    virtual void setTop(const DOMString &val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual long getZIndex() =0;

    /**
     *
     */
    virtual void setZIndex(long val) throw (dom::DOMException) =0;


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SMILRegionElement() {}


};



/*#########################################################################
## SMILMediaElement
#########################################################################*/

/**
 *
 */
class SMILMediaElement : virtual public ElementTime,
                         virtual public SMILElement
{
public:


    /**
     *
     */
    virtual DOMString getAbstractAttr() =0;

    /**
     *
     */
    virtual void setAbstractAttr(const DOMString &val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getAlt() =0;

    /**
     *
     */
    virtual void setAlt(const DOMString &val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getAuthor() =0;

    /**
     *
     */
    virtual void setAuthor(const DOMString &val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getClipBegin() =0;

    /**
     *
     */
    virtual void setClipBegin(const DOMString &val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getClipEnd() =0;

    /**
     *
     */
    virtual void setClipEnd(const DOMString &val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getCopyright() =0;

    /**
     *
     */
    virtual void setCopyright(const DOMString &val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getLongdesc() =0;

    /**
     *
     */
    virtual void setLongdesc(const DOMString &val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getPort() =0;

    /**
     *
     */
    virtual void setPort(const DOMString &val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getReadIndex() =0;

    /**
     *
     */
    virtual void setReadIndex(const DOMString &val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getRtpformat() =0;

    /**
     *
     */
    virtual void setRtpformat(const DOMString &val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getSrc() =0;

    /**
     *
     */
    virtual void setSrc(const DOMString &val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getStripRepeat() =0;

    /**
     *
     */
    virtual void setStripRepeat(const DOMString &val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getTitle() =0;

    /**
     *
     */
    virtual void setTitle(const DOMString &val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getTransport() =0;

    /**
     *
     */
    virtual void setTransport(const DOMString &val) throw (dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getType() =0;

    /**
     *
     */
    virtual void setType(const DOMString &val) throw (dom::DOMException) =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SMILMediaElement() {}


};


/*#########################################################################
## SMILRefElement
#########################################################################*/

/**
 *
 */
class SMILRefElement : virtual public SMILMediaElement
{
public:


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SMILRefElement() {}


};


/*#########################################################################
## SMILAnimation
#########################################################################*/

/**
 *
 */
class SMILAnimation : virtual public SMILElement,
                      virtual public ElementTargetAttributes,
                      virtual public ElementTime,
                      virtual public ElementTimeControl
{
public:

    typedef enum
        {
        ADDITIVE_REPLACE               = 0,
        ADDITIVE_SUM                   = 1
        } AdditiveTypes;


    /**
     *
     */
    virtual unsigned short getAdditive() =0;

    /**
     *
     */
    virtual void setAdditive(unsigned short val) throw (dom::DOMException)=0;

    typedef enum
        {
        ACCUMULATE_NONE                = 0,
        ACCUMULATE_SUM                 = 1
        } AccumulateTypes;


    /**
     *
     */
    virtual unsigned short getAccumulate() =0;

    /**
     *
     */
    virtual void setAccumulate(unsigned short val) throw (dom::DOMException)=0;

    typedef enum
        {
        CALCMODE_DISCRETE              = 0,
        CALCMODE_LINEAR                = 1,
        CALCMODE_PACED                 = 2,
        CALCMODE_SPLINE                = 3
        } CalcModeTypes;


    /**
     *
     */
    virtual unsigned short getCalcMode() =0;

    /**
     *
     */
    virtual void setCalcMode(unsigned short val) throw (dom::DOMException)=0;

    /**
     *
     */
    virtual DOMString getKeySplines() =0;

    /**
     *
     */
    virtual void setKeySplines(const DOMString &val) throw (dom::DOMException)=0;

    /**
     *
     */
    virtual TimeList getKeyTimes() =0;

    /**
     *
     */
    virtual void setKeyTimes(const TimeList &val) throw (dom::DOMException)=0;

    /**
     *
     */
    virtual DOMString getValues() =0;

    /**
     *
     */
    virtual void setValues(const DOMString &val) throw (dom::DOMException)=0;

    /**
     *
     */
    virtual DOMString getFrom() =0;

    /**
     *
     */
    virtual void setFrom(const DOMString &val) throw (dom::DOMException)=0;

    /**
     *
     */
    virtual DOMString getTo() =0;

    /**
     *
     */
    virtual void setTo(const DOMString &val) throw (dom::DOMException)=0;

    /**
     *
     */
    virtual DOMString getBy() =0;

    /**
     *
     */
    virtual void setBy(const DOMString &val) throw (dom::DOMException)=0;

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SMILAnimation() {}


};


/*#########################################################################
## SMILAnimateElement
#########################################################################*/

/**
 *
 */
class SMILAnimateElement : virtual public SMILAnimation
{
public:


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SMILAnimateElement() {}


};


/*#########################################################################
## SMILSetElement
#########################################################################*/

/**
 *
 */
class SMILSetElement : virtual public ElementTimeControl,
                       virtual public ElementTime,
                       virtual public ElementTargetAttributes,
                       virtual public SMILElement
{
public:

    /**
     *
     */
    virtual DOMString getTo() =0;

    /**
     *
     */
    virtual void setTo(const DOMString &val) =0;

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SMILSetElement() {}


};


/*#########################################################################
## SMILAnimateMotionElement
#########################################################################*/

/**
 *
 */
class SMILAnimateMotionElement : virtual public SMILAnimateElement
{
public:

    /**
     *
     */
    virtual DOMString getPath() =0;

    /**
     *
     */
    virtual void setPath(const DOMString &val) throw(dom::DOMException) =0;

    /**
     *
     */
    virtual DOMString getOrigin() =0;

    /**
     *
     */
    virtual void setOrigin(const DOMString &val) throw(dom::DOMException) =0;


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SMILAnimateMotionElement() {}


};


/*#########################################################################
## SMILAnimateColorElement
#########################################################################*/

/**
 *
 */
class SMILAnimateColorElement : virtual public SMILAnimation
{
public:


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SMILAnimateColorElement() {}


};





/*#########################################################################
## SMILSwitchElement
#########################################################################*/

/**
 *
 */
class SMILSwitchElement : virtual public SMILElement
{
public:

    /**
     *
     */
    virtual Element *getSelectedElement() =0;

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SMILSwitchElement() {}


};





}  //namespace smil
}  //namespace dom
}  //namespace w3c
}  //namespace org

#endif   /* __SMIL_H__ */

/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/

