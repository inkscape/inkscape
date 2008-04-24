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
 *  
 * ===========================================================================
 * NOTES
 * 
 * http://www.w3.org/TR/smil-boston-dom     
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
class Time;
class TimeEvent;
class TimeList;

class SMILAnimateColorElement;
typedef Ptr<SMILAnimateColorElement> SMILAnimateColorElementPtr;
class SMILAnimateElement;
typedef Ptr<SMILAnimateElement> SMILAnimateElementPtr;
class SMILAnimateMotionElement;
typedef Ptr<SMILAnimateMotionElement> SMILAnimateMotionElementPtr;
class SMILAnimation;
typedef Ptr<SMILAnimation> SMILAnimationPtr;
class SMILDocument;
typedef Ptr<SMILDocument> SMILDocumentPtr;
class SMILElement;
typedef Ptr<SMILElement> SMILElementPtr;
class SMILLayoutElement;
typedef Ptr<SMILLayoutElement> SMILLayoutElementPtr;
class SMILMediaElement;
typedef Ptr<SMILMediaElement> SMILMediaElementPtr;
class SMILRefElement;
typedef Ptr<SMILRefElement> SMILRefElementPtr;
class SMILRegionElement;
typedef Ptr<SMILRegionElement> SMILRegionElementPtr;
class SMILRegionInterface;
typedef Ptr<SMILRegionInterface> SMILRegionInterfacePtr;
class SMILRootLayoutElement;
typedef Ptr<SMILRootLayoutElement> SMILRootLayoutElementPtr;
class SMILSetElement;
typedef Ptr<SMILSetElement> SMILSetElementPtr;
class SMILSwitchElement;
typedef Ptr<SMILSwitchElement> SMILSwitchElementPtr;
class SMILTopLayoutElement;
typedef Ptr<SMILTopLayoutElement> SMILTopLayoutElementPtr;



/*#########################################################################
###########################################################################
##  D A T A    T Y P E S
###########################################################################
#########################################################################*/



/*#########################################################################
## ElementLayout
#########################################################################*/

/**
 * This interface is used by SMIL elements root-layout, top-layout and region. 
 */
class ElementLayout
{
public:


    /**
     * Return the title of an item implementing this interface
     */
    virtual DOMString getTitle()
        { return title; }

    /**
     * Set the title of an item implementing this interface
     */
    virtual void setTitle(const DOMString &val) throw(dom::DOMException)
        { title = val; }

    /**
     * Return the background color of an item implementing this interface
     */
    virtual DOMString getBackgroundColor()
        { return backgroundColor; }

    /**
     * Set the background color of an item implementing this interface
     */
    virtual void setBackgroundColor(const DOMString &val) throw(dom::DOMException)
        { backgroundColor = val; }

    /**
     * Return the height of an item implementing this interface
     */
    virtual long getHeight()
        { return height; }

    /**
     * Set the height of an item implementing this interface
     */
    virtual void setHeight(long val) throw(dom::DOMException)
        { height = val; }

    /**
     * Return the width of an item implementing this interface
     */
    virtual long getWidth()
        { return width; }

    /**
     * Set the width of an item implementing this interface
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
 * Declares rendering surface for an element. See the region attribute definition. 
 */
class SMILRegionInterface
{
public:

    /**
     * Gets an associated region element
     */
    virtual SMILRegionElementPtr getRegion()
        { return regionElement; }
        
    /**
     * Sets an associated region element
     */
    virtual void setRegion(const SMILRegionElementPtr val)
        { regionElement = val; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SMILRegionInterface()
        {  }

    /**
     *
     */
    SMILRegionInterface(const SMILRegionInterface &other)
       { regionElement = other.regionElement; }

    /**
     *
     */
    SMILRegionInterface& operator=(const SMILRegionInterface &other)
       {
	   regionElement = other.regionElement;
	   return *this;
	   }

    /**
     *
     */
    virtual ~SMILRegionInterface() {}

protected:

    SMILRegionElementPtr regionElement;

};


/*#########################################################################
## Time
#########################################################################*/

/**
 * The Time interface is a datatype that represents times within the timegraph.
 * A Time has a type, key values to describe the time, and a boolean to
 * indicate whether the values are currently unresolved. 
 */
class Time
{
public:


    /**
     * A boolean indicating whether the current Time has been fully resolved to the 
     * document schedule. Note that for this to be true, the current Time must be 
     * defined (not indefinite), the syncbase and all Time's that the syncbase 
     * depends on must be defined (not indefinite), and the begin Time of all 
     * ascendent time containers of this element and all Time elements that this 
     * depends upon must be defined (not indefinite). If this Time is based upon an 
     * event, this Time will only be resolved once the specified event has happened, 
     * subject to the constraints of the time container. Note that this may change 
     * from true to false when the parent time container ends its simple duration 
     * (including when it repeats or restarts).
     */
    virtual bool getResolved()
        { return resolved; }

    /**
     * The clock value in seconds relative to the parent time container begin. This 
     * indicates the resolved time relationship to the parent time container. This is 
     * only valid if resolved is true.
     */
    virtual double getResolvedOffset()
        { return resolvedOffset; }

    /**
     * An integer indicating the type of this time value. 
     */	     
    typedef enum
        {
        SMIL_TIME_INDEFINITE           = 0,
        SMIL_TIME_OFFSET               = 1,
        SMIL_TIME_SYNC_BASED           = 2,
        SMIL_TIME_EVENT_BASED          = 3,
        SMIL_TIME_WALLCLOCK            = 4,
        SMIL_TIME_MEDIA_MARKER         = 5
        } TimeType;



    /**
     * A code representing the type of the underlying object, as defined above. 
     */
    virtual unsigned short getTimeType()
        { return timeType; }


    /**
     * The clock value in seconds relative to the syncbase or eventbase.
     * Default value is 0. 
     */
    virtual double getOffset()
        { return offset; }

    /**
     * Set the value above
     */
    virtual void setOffset(double val) throw (dom::DOMException)
        { offset = val; }

    /**
     * Get the base element for a sync-based or event-based time. 
     */
    virtual ElementPtr getBaseElement()
        { return baseElement; }

    /**
     * Set the base element for a sync-based or event-based time. 
     */
    virtual void setBaseElement(const ElementPtr val) throw (dom::DOMException)
        { baseElement = val; }

    /**
     * If true, indicates that a sync-based time is relative to the begin of the 
     * baseElement. If false, indicates that a sync-based time is relative to the 
     * active end of the baseElement.
     */
    virtual bool getBaseBegin()
        { return baseBegin; }

    /**
     *  Set the value above.
     */
    virtual void setBaseBegin(bool val) throw (dom::DOMException)
        { baseBegin = val; }

    /**
     * Get the name of the event for an event-based time. Default value is null. 
     */
    virtual DOMString getEvent()
        { return eventStr; }

    /**
     * Set the name of the event for an event-based time. Default value is null. 
     */
    virtual void setEvent(const DOMString &val) throw (dom::DOMException)
        { eventStr = val; }

    /**
     * Get the name of the marker from the media element, for media marker
     * times. Default value is null. 
     */
    virtual DOMString getMarker()
        { return marker; }

    /**
     * Set the name of the marker from the media element, for media marker
     * times. Default value is null. 
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
        //baseElement    = NULL; //not necessary
        baseBegin      = false;
        eventStr       = "";
        marker         = "";
        }

    /**
     *
     */
    Time(const Time &other)
        {
        assign(other);
        }
       
    /**
     *
     */
    Time &operator=(const Time &other)
        {
        assign(other);
        return *this;
        }
       
    void assign(const Time &other)
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
    ElementPtr     baseElement;
    bool           baseBegin;
    DOMString      eventStr;
    DOMString      marker;

};


/*#########################################################################
## TimeList
#########################################################################*/

/**
 * The TimeList interface provides the abstraction of an ordered collection of 
 * times, without defining or constraining how this collection is implemented.
 * 
 * The items in the TimeList are accessible via an integral index, starting from 0.
 */
class TimeList
{
public:

    /**
     * Returns the indexth item in the collection. If index is greater than or equal 
     * to the number of times in the list, this returns null.
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
     * The number of times in the list. The range of valid child time
     * indices is 0 to length-1 inclusive. 
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
    TimeList &operator=(const TimeList &other)
        {
        items = other.items;
        return *this;
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
 * This interface defines the set of timing attributes that are common to
 * all timed elements. 
 */
class ElementTime
{
public:


    /**
     * Get the desired value (as a list of times) of the begin instant of this node. 
     */
    virtual TimeList &getBegin()
        { return beginTime; }

    /**
     * Set the desired value (as a list of times) of the begin instant of this node. 
     */
    virtual void setBegin(const TimeList &val) throw(dom::DOMException)
        { beginTime = val; }

    /**
     * Set the list of active ends for this node. 
     */
    virtual TimeList &getEnd()
        { return endTime; }

    /**
     * Set the list of active ends for this node. 
     */
    virtual void setEnd(const TimeList &val) throw(dom::DOMException)
        { endTime = val; }

    /**
     * Get the desired simple duration  value of this node in seconds.
     * Negative value means "indefinite". 
     */
    virtual double getDur()
        { return dur; }

    /**
     * Set the desired simple duration  value of this node in seconds.
     * Negative value means "indefinite". 
     */
    virtual void setDur(double val) throw(dom::DOMException)
        { dur = val; }

    /**
     * An integer indicating the value of the restart attribute. 
     */	     
    typedef enum
        {
        RESTART_ALWAYS                 = 0,
        RESTART_NEVER                  = 1,
        RESTART_WHEN_NOT_ACTIVE        = 2
        } RestartType;

    /**
     *  Get code representing the value of the restart  attribute,
     *  as defined above. Default value is RESTART_ALWAYS. 
     */
    virtual unsigned short getRestart()
        { return restart; }

    /**
     *  Set code representing the value of the restart  attribute,
     *  as defined above. Default value is RESTART_ALWAYS. 
     */
    virtual void setRestart(unsigned short val) throw (dom::DOMException)
        { restart = val; }


    /**
     * An integer indicating the value of the fill attribute. 
     */	     
    typedef enum
        {
        FILL_REMOVE                    = 0,
        FILL_FREEZE                    = 1
        } FillType;


    /**
     * Get code representing the value of the fill attribute, as defined above.
     * Default value is FILL_REMOVE. 
     */
    virtual unsigned short getFill()
        { return fill; }

    /**
     * Set code representing the value of the fill attribute, as defined above.
     */
    virtual void setFill(unsigned short val) throw (dom::DOMException)
        { fill = val; }

    /**
     * The repeatCount attribute causes the element to play repeatedly (loop) for the 
     * specified number of times. A negative value repeat the element indefinitely. 
     * Default value is 0 (unspecified).
     */
    virtual double getRepeatCount()
        { return repeatCount; }

    /**
     * Set the value above.
     */
    virtual void setRepeatCount(double val) throw (dom::DOMException)
        { repeatCount = val; }

    /**
     * The repeatDur causes the element to play repeatedly (loop) for the specified 
     * duration in milliseconds. Negative means "indefinite".
     */
    virtual double getRepeatDur()
        { return repeatDur; }

    /**
     * Set the value above.
     */
    virtual void setRepeatDur(double val) throw (dom::DOMException)
        { repeatDur = val; }

    /**
     * Causes this element to begin the local timeline (subject to sync constraints). 
     */
    virtual bool beginElement()
        {
        return true;
        }

    /**
     * Causes this element to end the local timeline (subject to sync constraints). 
     */
    virtual bool endElement()
        {
        return true;
        }

    /**
     * Causes this element to pause the local timeline (subject to sync constraints). 
     */
    virtual void pauseElement()
        {
        }

    /**
     * Causes this element to resume a paused local timeline. 
     */
    virtual void resumeElement()
        {
        }

    /**
     * Seeks this element to the specified point on the local timeline (subject to 
     * sync constraints). If this is a timeline, this must seek the entire timeline 
     * (i.e. propagate to all timeChildren).
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
       assign(other);
       }

    /**
     *
     */
    ElementTime &operator=(const ElementTime &other)
       {
       assign(other);
       return *this;
       }

    /**
     *
     */
    void assign(const ElementTime &other)
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
 * This interface support use-cases commonly associated with animation. 
 */
class ElementTimeManipulation
{
public:

    /**
     * Defines the playback speed of element time. The value is specified as a 
     * multiple of normal (parent time container) play speed. Legal values are signed 
     * floating point values. Zero values are not allowed. The default is 1.0 (no 
     * modification of speed).
     */
    virtual double getSpeed()
        { return speed; }
        
    /**
     * Sets the value above.
     */
    virtual void setSpeed(double val) throw (dom::DOMException)
        { speed = val; }

    /**
     * The percentage value of the simple acceleration of time for the element. 
     * Allowed values are from 0 to 100. Default value is 0 (no acceleration). The 
     * sum of the values for accelerate and decelerate must not exceed 100. If it 
     * does, the deceleration value will be reduced to make the sum legal.
     */
    virtual double getAccelerate()
        { return accelerate; }

    /**
     * Sets the value above.
     */
    virtual void setAccelerate(double val) throw (dom::DOMException)
        { accelerate = val; }

    /**
     * The percentage value of the simple decelerate of time for the element. Allowed 
     * values are from 0 to 100. Default value is 0 (no deceleration). The sum of the 
     * values for accelerate and decelerate must not exceed 100. If it does, the 
     * deceleration value will be reduced to make the sum legal.
     */
    virtual double getDecelerate()
        { return decelerate; }

    /**
     * Sets the value above.
     */
    virtual void setDecelerate(double val) throw (dom::DOMException)
        { decelerate = val; }

    /**
     * The autoReverse attribute controls the "play forwards then backwards" 
     * functionality. Default value is false.
     */
    virtual bool getAutoReverse()
        { return autoReverse; }

    /**
     * Sets the value above.
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
        assign(other);
        }

    /**
     *
     */
    ElementTimeManipulation &operator=(const ElementTimeManipulation &other)
        {
        assign(other);
        return *this;
        }

    /**
     *
     */
    void assign(const ElementTimeManipulation &other)
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
    bool   autoReverse;

};


/*#########################################################################
## ElementTimeContainer
#########################################################################*/

/**
 * This is a placeholder - subject to change. This represents generic timelines. 
 */
class ElementTimeContainer : public ElementTime
{
public:


    /**
     * A NodeList that contains all timed childrens of this node. If there
     * are no timed children, the Nodelist is empty. 
     */
    virtual NodeList getTimeChildren()
        {
        NodeList list;
        return list;
        }

    /**
     * Returns a list of child elements active at the specified invocation. 
     */
    virtual NodeList getActiveChildrenAt(double /*instant_in_millis*/)
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
 * The synchronization behavior extension. 
 */
class ElementSyncBehavior
{
public:

    /**
     * The runtime synchronization behavior for an element. 
     */
    virtual DOMString getSyncBehavior()
        { return syncBehavior; }

    /**
     * The sync tolerance for the associated element. It has an effect only
     * if the element has syncBehavior="locked". 
     */
    virtual double getSyncTolerance()
        { return syncTolerance; }

    /**
     * Defines the default value for the runtime synchronization behavior
     * for an element, and all descendents. 
     */
    virtual DOMString getDefaultSyncBehavior()
        { return defaultSyncBehavior; }

    /**
     * Defines the default value for the sync tolerance for an element,
     * 	 and all descendents. 
     */
    virtual double getDefaultSyncTolerance()
        { return defaultSyncTolerance; }

    /**
     * If set to true, forces the time container playback to sync to this element. 
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
        assign(other);
        }

    /**
     *
     */
    ElementSyncBehavior &operator=(const ElementSyncBehavior &other)
        {
        assign(other);
        return *this;
        }

    /**
     *
     */
    void assign(const ElementSyncBehavior &other)
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
 * A parallel container defines a simple parallel time grouping in which multiple 
 * elements can play back at the same time.
 */
class ElementParallelTimeContainer : public ElementTimeContainer
{
public:

    /**
     * Controls the end of the container. 
     */
    virtual DOMString getEndSync()
        { return endSync; }

    /**
     * Controls the end of the container. 
     */
    virtual void setEndSync(const DOMString &val) throw (dom::DOMException)
        { endSync = val; }

    /**
     * This method returns the implicit duration in seconds. 
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
        assign(other);
        }

    /**
     *
     */
    ElementParallelTimeContainer &operator=(const ElementParallelTimeContainer &other)
        {
        assign(other);
        return *this;
        }

    /**
     *
     */
    void assign(const ElementParallelTimeContainer &other)
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
 * A seq container defines a sequence of elements in which elements play
 * one after the other. 
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
 * This interface defines a time container with semantics based upon par, but 
 * with the additional constraint that only one child element may play at a time.
 */
class ElementExclusiveTimeContainer : public ElementTimeContainer
{
public:

    /**
     * Controls the end of the container. 
     */
    virtual DOMString getEndSync()
        { return endSync; }

    /**
     * Controls the end of the container. 
     */
    virtual void setEndSync(const DOMString &val) throw (dom::DOMException)
        { endSync = val; }

    /**
     * This should support another method to get the ordered collection of paused 
     * elements (the paused stack) at a given point in time.
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
        assign(other);
        }

    /**
     *
     */
    ElementExclusiveTimeContainer &operator=(const ElementExclusiveTimeContainer &other)
        {
        assign(other);
        return *this;
        }

    /**
     *
     */
    void assign(const ElementExclusiveTimeContainer &other)
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
 * NOTE:  need more info
 */
class ElementTimeControl
{
public:

    /**
     * Causes this element to begin the local timeline (subject to sync constraints). 
     */
    virtual bool beginElement() throw(dom::DOMException)
        {
        return true;
        }

    /**
     * Causes this element to begin the local timeline (subject to sync constraints), 
     * at the passed offset from the current time when the method is called. If the 
     * offset is >= 0, the semantics are equivalent to an event-base begin with the 
     * specified offset. If the offset is < 0, the semantics are equivalent to 
     * beginElement(), but the element active duration is evaluated as though the 
     * element had begun at the passed (negative) offset from the current time when 
     * the method is called.
     */
    virtual bool beginElementAt(double /*offset*/) throw(dom::DOMException)
        {
        return true;
        }

    /**
     * Causes this element to end the local timeline (subject to sync constraints). 
     */
    virtual bool endElement() throw(dom::DOMException)
        {
        return true;
        }

    /**
     * Causes this element to end the local timeline (subject to sync constraints) at 
     * the specified offset from the current time when the method is called.
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
 * This interface define the set of animation target extensions. 
 */
class ElementTargetAttributes
{
public:


    /**
     * Get the name of the target attribute. 
     */
    virtual DOMString getAttributeName()
        { return attributeName; }

    /**
     * Set the name of the target attribute. 
     */
    virtual void setAttributeName(const DOMString &val)
        { attributeName = val; }

    /**
     * A code representing the value of the attributeType attribute
     */	     
    typedef enum
        {
        ATTRIBUTE_TYPE_AUTO            = 0,
        ATTRIBUTE_TYPE_CSS             = 1,
        ATTRIBUTE_TYPE_XML             = 2
        } AttributeType;

    /**
     * Get the attribute type, as defined above.
     */
    virtual unsigned short getAttributeType()
        { return attributeType; }

    /**
     * Set the attribute type, as defined above.
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
       assign(other);
       }

    /**
     *
     */
    ElementTargetAttributes &operator=(const ElementTargetAttributes &other)
       {
       assign(other);
       return *this;
       }

    /**
     *
     */
    void assign(const ElementTargetAttributes &other)
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
 * Defines the test attributes interface. See the Test attributes definition. 
 */
class ElementTest
{
public:


    /**
     * Get the systemBitrate value. 
     */
    virtual long getSystemBitrate()
        { return systemBitrate; }

    /**
     * Set the systemBitrate value. 
     */
    virtual void setSystemBitrate(long val) throw (dom::DOMException)
        { systemBitrate = val; }

    /**
     *  Get the systemCaptions value. 
     */
    virtual bool getSystemCaptions()
        { return systemCaptions; }

    /**
     * Set the systemCaptions value. 
     */
    virtual void setSystemCaptions(bool val) throw (dom::DOMException)
        { systemCaptions = val; }

    /**
     * Get the systemLanguage value. 
     */
    virtual DOMString getSystemLanguage()
        { return systemLanguage; }

    /**
     * Set the systemLanguage value. 
     */
    virtual void setSystemLanguage(const DOMString &val) throw (dom::DOMException)
        { systemLanguage = val; }

    /**
     * Get the systemRequired value. 
     */
    virtual bool getSystemRequired()
        { return systemRequired; }

    /**
     * Get the systemScreenSize value. 
     */
    virtual bool getSystemScreenSize()
        { return systemScreenSize; }

    /**
     * Get the systemScreenDepth value. 
     */
    virtual bool getSystemScreenDepth()
        { return systemScreenDepth; }

    /**
     * Get the systemOverdubOrSubtitle value. 
     */
    virtual DOMString getSystemOverdubOrSubtitle()
        { return systemOverdubOrSubtitle; }

    /**
     * Set the systemOverdubOrSubtitle value. 
     */
    virtual void setSystemOverdubOrSubtitle(const DOMString &val) throw (dom::DOMException)
        { systemOverdubOrSubtitle = val; }

    /**
     * Get the systemAudioDesc value. 
     */
    virtual bool getSystemAudioDesc()
        { return systemAudioDesc; }

    /**
     * Set the systemOverdubOrSubtitle value. 
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
        }

    /**
     *
     */
    ElementTest(const ElementTest &other)
        {
        assign(other);
        }

    /**
     *
     */
    ElementTest &operator=(const ElementTest &other)
        {
        assign(other);
        return *this;
        }

    /**
     *
     */
    void assign(const ElementTest &other)
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
 * The TimeEvent interface provides specific contextual information associated
 * with Time events. 
 */
class TimeEvent : public events::Event
{
public:

    /**
     * The view attribute identifies the AbstractView from which the event
     * was generated. 
     */
    virtual views::AbstractView &getView()
        { return view; }

    /**
     * Specifies some detail information about the Event, depending on
     * 	 the type of event. 
     */
    virtual long getDetail()
        { return detail; }

    /**
     * The initTimeEvent method is used to initialize the value of a TimeEvent 
     * created through the DocumentEvent interface. This method may only be called 
     * before the TimeEvent has been dispatched via the dispatchEvent method, though 
     * it may be called multiple times during that phase if necessary. If called 
     * multiple times, the final invocation takes precedence.
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
        assign(other);
        }

    /**
     *
     */
    TimeEvent &operator=(const TimeEvent &other)
        {
        assign(other);
        return *this;
        }

    /**
     *
     */
    void assign(const TimeEvent &other)
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
 * A SMIL document is the root of the SMIL Hierarchy and holds the entire 
 * content. Beside providing access to the hierarchy, it also provides some 
 * convenience methods for accessing certain sets of information from the document.
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
 * The SMILElement interface is the base for all SMIL element types. It follows 
 * the model of the HTMLElement in the HTML DOM, extending the base Element class 
 * to denote SMIL-specific elements.
 */
class SMILElement : virtual public Element
{
public:

    /**
     * Get the unique ID
     */
    virtual DOMString getId() =0;

    /**
     * Set the unique ID
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
 * Declares layout type for the document. See:
 * http://www.w3.org/TR/2000/WD-smil-boston-20000225/layout.html#LayoutModuleNS-TheLayoutElement 
 */
class SMILLayoutElement : virtual public SMILElement
{
public:

    /**
     * The mime type of the layout langage used in this layout element.The default 
     * value of the type attribute is "text/smil-basic-layout".
     */
    virtual DOMString getType() =0;

    /**
     * true if the player can understand the mime type, false otherwise. 
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
 * Declares layout properties for the top-layout element. See:
 * http://www.w3.org/TR/2000/WD-smil-boston-20000225/layout.html#LayoutModuleNS-TheTopLayoutElement 
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
 * Declares layout properties for the root-layout element. See:
 * http://www.w3.org/TR/2000/WD-smil-boston-20000225/layout.html#LayoutModuleNS-TheRootLayoutElement 
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
 * Controls the position, size and scaling of media object elements. See:
 * http://www.w3.org/TR/2000/WD-smil-boston-20000225/layout.html#LayoutModuleNS-TheRegionElement
 */
class SMILRegionElement : virtual public SMILElement,
                          virtual public ElementLayout
{
public:


    /**
     * Get the fit value
     */
    virtual DOMString getFit() =0;

    /**
     * Set the fit value
     */
    virtual void setFit(const DOMString &val) throw (dom::DOMException) =0;

    /**
     * Get the top value
     */
    virtual DOMString getTop() =0;

    /**
     * Set the top value
     */
    virtual void setTop(const DOMString &val) throw (dom::DOMException) =0;

    /**
     * Get the ZIndex value
     */
    virtual long getZIndex() =0;

    /**
     * Set the ZIndex value
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
 * Declares media content. 
 */
class SMILMediaElement : virtual public ElementTime,
                         virtual public SMILElement
{
public:


    /**
     * Get the "abstractAttr" value
     */
    virtual DOMString getAbstractAttr() =0;

    /**
     * Set the "abstractAttr" value
     */
    virtual void setAbstractAttr(const DOMString &val) throw (dom::DOMException) =0;

    /**
     * Get the "alt" value
     */
    virtual DOMString getAlt() =0;

    /**
     * Set the "alt" value
     */
    virtual void setAlt(const DOMString &val) throw (dom::DOMException) =0;

    /**
     * Get the "author" value
     */
    virtual DOMString getAuthor() =0;

    /**
     * Set the "author" value
     */
    virtual void setAuthor(const DOMString &val) throw (dom::DOMException) =0;

    /**
     * Get the "clipBegin" value
     */
    virtual DOMString getClipBegin() =0;

    /**
     * Set the "clipBegin" value
     */
    virtual void setClipBegin(const DOMString &val) throw (dom::DOMException) =0;

    /**
     * Get the "clipEnd" value
     */
    virtual DOMString getClipEnd() =0;

    /**
     * Set the "clipEnd" value
     */
    virtual void setClipEnd(const DOMString &val) throw (dom::DOMException) =0;

    /**
     * Get the "copyright" value
     */
    virtual DOMString getCopyright() =0;

    /**
     * Set the "copyright" value
     */
    virtual void setCopyright(const DOMString &val) throw (dom::DOMException) =0;

    /**
     * Get the "longdesc" value
     */
    virtual DOMString getLongdesc() =0;

    /**
     * Set the "longdesc" value
     */
    virtual void setLongdesc(const DOMString &val) throw (dom::DOMException) =0;

    /**
     * Get the "port" value
     */
    virtual DOMString getPort() =0;

    /**
     * Set the "port" value
     */
    virtual void setPort(const DOMString &val) throw (dom::DOMException) =0;

    /**
     * Get the "readIndex" value
     */
    virtual DOMString getReadIndex() =0;

    /**
     * Set the "readIndex" value
     */
    virtual void setReadIndex(const DOMString &val) throw (dom::DOMException) =0;

    /**
     * Get the "rtpFormat" value
     */
    virtual DOMString getRtpformat() =0;

    /**
     * Set the "readIndex" value
     */
    virtual void setRtpformat(const DOMString &val) throw (dom::DOMException) =0;

    /**
     * Get the "src" value
     */
    virtual DOMString getSrc() =0;

    /**
     * Set the "src" value
     */
    virtual void setSrc(const DOMString &val) throw (dom::DOMException) =0;

    /**
     * Get the "stripRepeat" value
     */
    virtual DOMString getStripRepeat() =0;

    /**
     * Set the "stripRepeat" value
     */
    virtual void setStripRepeat(const DOMString &val) throw (dom::DOMException) =0;

    /**
     * Get the "title" value
     */
    virtual DOMString getTitle() =0;

    /**
     * Set the "title" value
     */
    virtual void setTitle(const DOMString &val) throw (dom::DOMException) =0;

    /**
     * Get the "transport" value
     */
    virtual DOMString getTransport() =0;

    /**
     * Set the "transport" value
     */
    virtual void setTransport(const DOMString &val) throw (dom::DOMException) =0;

    /**
     * Get the "type" value
     */
    virtual DOMString getType() =0;

    /**
     * Set the "type" value
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
 * Audio, video,  etc
 * NOTE: need more info 
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
 * This interface defines the set of animation extensions for SMIL. 
 */
class SMILAnimation : virtual public SMILElement,
                      virtual public ElementTargetAttributes,
                      virtual public ElementTime,
                      virtual public ElementTimeControl
{
public:

    /**
     * Codes for the "additive" attribute
     */	     
    typedef enum
        {
        ADDITIVE_REPLACE               = 0,
        ADDITIVE_SUM                   = 1
        } AdditiveType;


    /**
     * A code representing the value of the additive  attribute, as defined above.
     * Default value is ADDITIVE_REPLACE. 
     */
    virtual unsigned short getAdditive() =0;

    /**
     * Set the value above.
     */
    virtual void setAdditive(unsigned short val) throw (dom::DOMException)=0;

    /**
     * Codes for the 'accumulate' attribute
     */	     
    typedef enum
        {
        ACCUMULATE_NONE                = 0,
        ACCUMULATE_SUM                 = 1
        } AccumulateType;


    /**
     * A code representing the value of the accumulate  attribute, as defined above.
     * Default value is ACCUMULATE_NONE. 
     */
    virtual unsigned short getAccumulate() =0;

    /**
     * Set the value above
     */
    virtual void setAccumulate(unsigned short val) throw (dom::DOMException)=0;

    /**
     * Codes for the "calcmode" attribute
     */	     
    typedef enum
        {
        CALCMODE_DISCRETE              = 0,
        CALCMODE_LINEAR                = 1,
        CALCMODE_PACED                 = 2,
        CALCMODE_SPLINE                = 3
        } CalcModeType;


    /**
     * A code representing the value of the calcMode  attribute, as defined above. 
     */
    virtual unsigned short getCalcMode() =0;

    /**
     *  Set the value above.
     */
    virtual void setCalcMode(unsigned short val) throw (dom::DOMException)=0;

    /**
     * A DOMString representing the value of the keySplines  attribute. 
     */
    virtual DOMString getKeySplines() =0;

    /**
     *  Set the value above.
     */
    virtual void setKeySplines(const DOMString &val) throw (dom::DOMException)=0;

    /**
     * Get a list of the time values of the keyTimes attribute. 
     */
    virtual TimeList getKeyTimes() =0;

    /**
     * Set the list of the time value of the keyTimes  attribute. 
     */
    virtual void setKeyTimes(const TimeList &val) throw (dom::DOMException)=0;

    /**
     * Get a DOMString representing the value of the values attribute. 
     */
    virtual DOMString getValues() =0;

    /**
     * Set the DOMString representing the value of the values attribute. 
     */
    virtual void setValues(const DOMString &val) throw (dom::DOMException)=0;

    /**
     * Get a DOMString representing the value of the from attribute. 
     */
    virtual DOMString getFrom() =0;

    /**
     * Set the DOMString representing the value of the from attribute. 
     */
    virtual void setFrom(const DOMString &val) throw (dom::DOMException)=0;

    /**
     * Get a DOMString representing the value of the to attribute. 
     */
    virtual DOMString getTo() =0;

    /**
     * Set the DOMString representing the value of the to attribute. 
     */
    virtual void setTo(const DOMString &val) throw (dom::DOMException)=0;

    /**
     * Get a DOMString representing the value of the by attribute. 
     */
    virtual DOMString getBy() =0;

    /**
     * Set the DOMString representing the value of the by attribute. 
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
 * This interface represents the SMIL animate element. 
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
 * This interface represents the set element. 
 */
class SMILSetElement : virtual public ElementTimeControl,
                       virtual public ElementTime,
                       virtual public ElementTargetAttributes,
                       virtual public SMILElement
{
public:

    /**
     * Specifies the value for the attribute during the duration of this element. 
     */
    virtual DOMString getTo() =0;

    /**
     * Set the value above.
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
 * This interface present the animationMotion element in SMIL. 
 */
class SMILAnimateMotionElement : virtual public SMILAnimateElement
{
public:

    /**
     * Specifies the curve that describes the attribute value as a function of time. 
     */
    virtual DOMString getPath() =0;

    /**
     * Specifies the curve that describes the attribute value as a function of time. 
     */
    virtual void setPath(const DOMString &val) throw(dom::DOMException) =0;

    /**
     * Specifies the origin of motion for the animation. 
     */
    virtual DOMString getOrigin() =0;

    /**
     * Specifies the origin of motion for the animation. 
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
 * This interface represents the SMIL animateColor element. 
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
 * Defines a block of content control. See:
 * http://www.w3.org/TR/2000/WD-smil-boston-20000225/content.html#ContentControlNS-SwitchElement
 */
class SMILSwitchElement : virtual public SMILElement
{
public:

    /**
     * Returns the slected element at runtime. null if the selected element
     * is not yet available. 
     */
    virtual ElementPtr getSelectedElement() =0;

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

