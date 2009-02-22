#ifndef __EVENTS_H__
#define __EVENTS_H__

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
 * ========================================================================
 * NOTES
 * 
 * This Events API follows somewhat this specification:
 * http://www.w3.org/TR/2003/NOTE-DOM-Level-3-Events-20031107/events.html
 * 
 * Some of the comments are excerpted from that document.
 * 
 *          
 */


#include "dom.h"
#include "views.h"




namespace org    {
namespace w3c    {
namespace dom    {
namespace events {




//Local definitions
typedef dom::DOMString DOMString;
typedef dom::DOMTimeStamp DOMTimeStamp;
typedef dom::NodePtr  NodePtr ;




//forward declarations
class Event;
class EventTarget;
class EventListener;
class DocumentEvent;
class CustomEvent;
class UIEvent;
class TextEvent;
class MouseEvent;
class KeyboardEvent;
class MutationEvent;
class MutationNameEvent;



/*#########################################################################
## EventException
#########################################################################*/

/**
 * Event operations may throw an EventException as specified in their
 *  method descriptions. 
 */
class EventException
{
public:

    /**
     * An integer indicating the type of error generated. 
     */
    typedef enum
        {
        UNSPECIFIED_EVENT_TYPE_ERR = 0,
        DISPATCH_REQUEST_ERR       = 1
        } EventExceptionCode;

    unsigned short code;


    //##################
    //# Non-API methods
    //##################

    EventException(short theCode)
        {
            code = theCode;
        }

    virtual ~EventException() throw()
       {}

};




/*#########################################################################
## Event
#########################################################################*/

/**
 * The Event interface is used to provide contextual information about an event 
 * to the listener processing the event. An object which implements the Event 
 * interface is passed as the parameter to an EventListener. More specific 
 * context information is passed to event listeners by deriving additional 
 * interfaces from Event which contain information directly relating to the type 
 * of event they represent. These derived interfaces are also implemented by the 
 * object passed to the event listener.
 * 
 * To create an instance of the Event interface, use the 
 * DocumentEvent.createEvent("Event") method call.
 */
class Event
{
public:

    /**
     * An integer indicating which phase of the event flow is being processed
     * 	 as defined in DOM event flow. 
     */
    typedef enum
        {
        CAPTURING_PHASE = 1,
        AT_TARGET       = 2,
        BUBBLING_PHASE  = 3
        } PhaseType;

    /**
     * The name should be an NCName  as defined in [XML Namespaces] and is
     * case-sensitive.
     * If the attribute Event.namespaceURI is different from null, this
     * attribute represents a local name. 
     */
    virtual DOMString getType() const
        { return eventType; }

    /**
     * Used to indicate the event target. This attribute contains the target
     * node when used with the DOM event flow. 
     */
    virtual EventTarget *getTarget()
        { return target; }

    /**
     * Used to indicate the EventTarget whose EventListeners are currently
     * being processed. This is particularly useful during the capture and
     * bubbling phases. This attribute could contain the target node or a
     * target ancestor when used with the DOM event flow. 
     */
    virtual EventTarget *getCurrentTarget()
        { return currentTarget; }

    /**
     * Used to indicate which phase of event flow is currently being accomplished. 
     */
    virtual unsigned short getEventPhase()
        { return eventPhase; }

    /**
     * Used to indicate whether or not an event is a bubbling event. If the
     * event can bubble the value is true, otherwise the value is false. 
     */
    virtual bool getBubbles()
        { return canBubble; }

    /**
     * Used to indicate whether or not an event can have its default action
     * prevented (see also Default actions and cancelable events). If the
     * default action can be prevented the value is true, otherwise the
     * value is false. 
     */
    virtual bool getCancelable()
        { return cancelable; }

    /**
     * Used to specify the time (in milliseconds relative to the epoch) at which the 
     * event was created. Due to the fact that some systems may not provide this 
     * information the value of timeStamp may be not available for all events. When 
     * not available, a value of 0 will be returned. Examples of epoch time are the 
     * time of the system start or 0:0:0 UTC 1st January 1970.
     */
    virtual DOMTimeStamp getTimeStamp()
        { return timeStamp; }

    /** 
     * This method is used to prevent event listeners of the same group to be 
     * triggered but its effect is deferred until all event listeners attached on the 
     * currentTarget have been triggered (see Event propagation and event groups). 
     * Once it has been called, further calls to that method have no additional effect.
     */
    virtual void stopPropagation()
        {
        }

    /**
     * If an event is cancelable, the preventDefault method is used to signify that 
     * the event is to be canceled, meaning any default action normally taken by the 
     * implementation as a result of the event will not occur (see also Default 
     * actions and cancelable events), and thus independently of event groups. 
     * Calling this method for a non-cancelable event has no effect.
     */
    virtual void preventDefault()
        {
        }

    /**
     * The initEvent method is used to initialize the value of an Event created 
     * through the DocumentEvent.createEvent method. This method may only be called 
     * before the Event has been dispatched via the EventTarget.dispatchEvent() 
     * method. If the method is called several times before invoking 
     * EventTarget.dispatchEvent, only the final invocation takes precedence. This 
     * method has no effect if called after the event has been dispatched. If called 
     * from a subclass of the Event interface only the values specified in this 
     * method are modified, all other attributes are left unchanged.
     * 
     * This method sets the Event.type attribute to eventTypeArg, and 
     * Event.namespaceURI to null. To initialize an event with a namespace URI, use 
     * the Event.initEventNS(namespaceURIArg, eventTypeArg, ...) method.
     */
    virtual void initEvent(const DOMString &eventTypeArg,
                           bool canBubbleArg,
                           bool cancelableArg)
        {
        namespaceURI = "";
        eventType    = eventTypeArg;
        canBubble    = canBubbleArg;
        cancelable   = cancelableArg;
        }


    /**
     * The namespace URI  associated with this event at creation time, or
     *  null if it is unspecified. 
     */
    virtual DOMString getNamespaceURI() const
        { return namespaceURI; }

    /**
     * This method will always return false, unless the event implements
     * the CustomEvent interface. 
     */
    virtual bool isCustom()
        { return custom; }

    /**
     * This method is used to prevent event listeners of the same group to be 
     * triggered and, unlike stopPropagation its effect is immediate (see Event 
     * propagation and event groups). Once it has been called, further calls to that 
     * method have no additional effect.
     * 
     * Note: This method does not prevent the default action from being invoked; use 
     * Event.preventDefault() for that effect.
     */
    virtual void stopImmediatePropagation()
        {
        }

    /**
     * This method will return true if the method Event.preventDefault()
     * has been called for this event, false otherwise. 
     */
    virtual bool isDefaultPrevented()
        { return defaultPrevented; }

    /**
     * The initEventNS method is used to initialize the value of an Event
     *  object and has the same behavior as Event.initEvent(). 
     */
    virtual void initEventNS(const DOMString &namespaceURIArg,
                             const DOMString &eventTypeArg,
                             bool canBubbleArg,
                             bool cancelableArg)
        {
        namespaceURI = namespaceURIArg;
        eventType    = eventTypeArg;
        canBubble    = canBubbleArg;
        cancelable   = cancelableArg;
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    Event()
        {
        init();
        }

    /**
     *
     */
    Event(const DOMString &eventTypeArg)
        {
        init();
        eventType     = eventTypeArg;
        }


    /**
     *
     */
    Event(const Event &other)
        {
        eventType        = other.eventType;
        target           = other.target;
        currentTarget    = other.currentTarget;
        eventPhase       = other.eventPhase;
        canBubble        = other.canBubble;
        cancelable       = other.cancelable;
        timeStamp        = other.timeStamp;
        namespaceURI     = other.namespaceURI;
        custom           = other.custom;
        defaultPrevented = other.defaultPrevented;
        }

    /**
     *
     */
    virtual ~Event() {}

protected:

    /**
     *
     */
    void init()
        {
        eventType        = "";
        target           = NULL;
        currentTarget    = NULL;
        eventPhase       = 0;
        canBubble        = false;
        cancelable       = false;
        //timeStamp      = other.timeStamp;
        namespaceURI     = "";
        custom           = false;
        defaultPrevented = false;
        }

    DOMString      eventType;
    EventTarget    *target;
    EventTarget    *currentTarget;
    unsigned short eventPhase;
    bool           canBubble;
    bool           cancelable;
    DOMTimeStamp   timeStamp;
    DOMString      namespaceURI;
    bool           custom;
    bool           defaultPrevented;

};




/*#########################################################################
## EventListener
#########################################################################*/

/**
 * The EventListener interface is the primary way for handling events. Users 
 * implement the EventListener interface and register their event listener on an 
 * EventTarget. The users should also remove their EventListener from its 
 * EventTarget after they have completed using the listener.
 * 
 * Copying a Node, with methods such as Node.cloneNode or Range.cloneContents, 
 * does not copy the event listeners attached to it. Event listeners must be 
 * attached to the newly created Node afterwards if so desired.
 * 
 * Moving a Node, with methods Document.adoptNode, Node.appendChild, or 
 * Range.extractContents, does not affect the event listeners attached to it.
 */
class EventListener
{
public:

    /**
     * This method is called whenever an event occurs of the event type
     * for which the EventListener interface was registered. 
     */
    virtual void handleEvent(const Event &/*evt*/)
        {}

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~EventListener() {}
};






/*#########################################################################
## EventTarget
#########################################################################*/


/**
 * The EventTarget interface is implemented by all the objects which could be 
 * event targets in an implementation which supports the Event flows. The 
 * interface allows registration, removal or query of event listeners, and 
 * dispatch of events to an event target.
 * 
 * When used with DOM event flow, this interface is implemented by all target 
 * nodes and target ancestors, i.e. all DOM Nodes of the tree support this 
 * interface when the implementation conforms to DOM Level 3 Events and, 
 * therefore, this interface can be obtained by using binding-specific casting 
 * methods on an instance of the Node interface.
 * 
 * Invoking addEventListener or addEventListenerNS multiple times on the same 
 * EventTarget with the same parameters (namespaceURI, type, listener, and 
 * useCapture) is considered to be a no-op and thus independently of the event 
 * group. They do not cause the EventListener to be called more than once and do 
 * not cause a change in the triggering order. In order to guarantee that an 
 * event listener will be added to the event target for the specified event group,
 * one needs to invoke removeEventListener or removeEventListenerNS first.
 */
class EventTarget
{
private:

	class EventListenerEntry
	{
	public:
	    EventListenerEntry(const DOMString &namespaceURIArg,
	                       const DOMString &eventTypeArg,
	                       const EventListener *listenerArg,
	                       bool useCaptureArg)
	        {
	        namespaceURI = namespaceURIArg;
	        eventType    = eventTypeArg;
	        listener     = (EventListener *)listenerArg;
	        useCapture   = useCaptureArg;
	        }
	
	    EventListenerEntry(const EventListenerEntry &other)
	        {
	        namespaceURI = other.namespaceURI;
	        eventType    = other.eventType;
	        listener     = other.listener;
	        useCapture   = other.useCapture;
	        }
	
	    virtual ~EventListenerEntry() {}
	
	    DOMString namespaceURI;
	    DOMString eventType;
	    EventListener *listener;
	    bool useCapture;
	};



public:

    /**
     * This method allows the registration of an event listener in the default group 
     * and, depending on the useCapture parameter, on the capture phase of the DOM 
     * event flow or its target and bubbling phases.
     */
    virtual void addEventListener(const DOMString &type,
                                  const EventListener *listener,
                                  bool useCapture)
        {
        EventListenerEntry entry("", type, listener, useCapture);
        listeners.push_back(entry);
        }

    /**
     * This method allows the removal of event listeners from the default group.
     * Calling removeEventListener with arguments which do not identify any currently 
     * registered EventListener on the EventTarget has no effect.
     */
    virtual void removeEventListener(const DOMString &type,
                                     const EventListener *listener,
                                     bool useCapture)
        {
        std::vector<EventListenerEntry>::iterator iter;
        for (iter = listeners.begin() ; iter != listeners.end() ; iter++)
            {
            EventListenerEntry entry = *iter;
            if (entry.eventType == type &&
                entry.listener  == listener &&
                useCapture && entry.useCapture)
                listeners.erase(iter);
            }
        }

    /**
     * This method allows the dispatch of events into the implementation's event 
     * model. The event target of the event is the EventTarget object on which 
     * dispatchEvent is called.
     */
    virtual bool dispatchEvent(const Event &evt) throw(EventException)
        {

        for (unsigned int i=0 ; i<listeners.size() ; i++)
            {
            EventListenerEntry listener = listeners[i];
            if (listener.namespaceURI == evt.getNamespaceURI()  &&
                listener.eventType    == evt.getType())
                {
                if (listener.listener)
                    listener.listener->handleEvent(evt);
                }
            }
        return true;
        }


    /**
     * This method allows the registration of an event listener in a specified group 
     * or the default group and, depending on the useCapture parameter, on the 
     * capture phase of the DOM event flow or its target and bubbling phases.
     */
    virtual void addEventListenerNS(const DOMString &namespaceURI,
                                    const DOMString &type,
                                    const EventListener *listener,
                                    bool useCapture)
        {
        EventListenerEntry entry(namespaceURI, type, listener, useCapture);
        listeners.push_back(entry);
        }

    /**
     * This method allows the removal of an event listener, independently of the 
     * associated event group.
     * Calling removeEventListenerNS with arguments which do not identify any 
     * currently registered EventListener on the EventTarget has no effect.
     */
    virtual void removeEventListenerNS(const DOMString &namespaceURI,
                                       const DOMString &type,
                                       const EventListener *listener,
                                       bool useCapture)
        {
        std::vector<EventListenerEntry>::iterator iter;
        for (iter = listeners.begin() ; iter != listeners.end() ; iter++)
            {
            EventListenerEntry entry = *iter;
            if (entry.namespaceURI == namespaceURI &&
                entry.eventType    == type &&
                entry.listener     == listener &&
                useCapture && entry.useCapture)
                listeners.erase(iter);
            }
        }

    /**
     * This method allows the DOM application to know if an event listener, attached 
     * to this EventTarget or one of its ancestors, will be triggered by the 
     * specified event type during the dispatch of the event to this event target or 
     * one of its descendants.
     */
    virtual bool willTriggerNS(const DOMString &namespaceURI,
                               const DOMString &type)
        {
        std::vector<EventListenerEntry>::iterator iter;
        for (iter = listeners.begin() ; iter != listeners.end() ; iter++)
            {
            EventListenerEntry entry = *iter;
            if (entry.namespaceURI == namespaceURI &&
                entry.eventType    == type)
                return true;
            }
        return false;
        }

    /**
     * This method allows the DOM application to know if this EventTarget contains an 
     * event listener registered for the specified event type. This is useful for 
     * determining at which nodes within a hierarchy altered handling of specific 
     * event types has been introduced, but should not be used to determine whether 
     * the specified event type triggers an event listener (see 
     * EventTarget.willTriggerNS()).
     */
    virtual bool hasEventListenerNS(const DOMString &namespaceURI,
                                    const DOMString &type)
        {
        std::vector<EventListenerEntry>::iterator iter;
        for (iter = listeners.begin() ; iter != listeners.end() ; iter++)
            {
            EventListenerEntry entry = *iter;
            if (entry.namespaceURI == namespaceURI &&
                entry.eventType    == type)
                return true;
            }
        return false;
        }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    EventTarget() {}

    /**
     *
     */
    EventTarget(const EventTarget &other)
        {
        listeners = other.listeners;
        }

    /**
     *
     */
    virtual ~EventTarget() {}

protected:

    std::vector<EventListenerEntry> listeners;

};




/*#########################################################################
## DocumentEvent
#########################################################################*/

/**
 * The DocumentEvent interface provides a mechanism by which the user can create 
 * an Event object of a type supported by the implementation. If the feature 
 * "Events" is supported by the Document object, the DocumentEvent interface must 
 * be implemented on the same object. If the feature "+Events" is supported by 
 * the Document object, an object that supports the DocumentEvent interface must 
 * be returned by invoking the method Node.getFeature("+Events", "3.0") on the 
 * Document object.
 */
class DocumentEvent : virtual public Event
{
public:

    /**
     * Create an event with the current document
     */
    virtual Event createEvent(const DOMString &/*eventType*/)
                               throw (dom::DOMException)
        {
        Event event;
        return event;
        }

    /**
     * Test if the implementation can generate events of a specified type. 
     */
    virtual bool canDispatch(const DOMString &/*namespaceURI*/,
                             const DOMString &/*type*/)
        {
        return dispatchable;
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    DocumentEvent() {}

    /**
     *
     */
    DocumentEvent(const DocumentEvent &other) : Event(other)
        {
        dispatchable = other.dispatchable;
        }

    /**
     *
     */
    virtual ~DocumentEvent() {}

protected:

    bool dispatchable;


};


/*#########################################################################
## CustomEvent
#########################################################################*/

/**
 * The CustomEvent interface gives access to the attributes Event.currentTarget 
 * and Event.eventPhase. It is intended to be used by the DOM Events 
 * implementation to access the underlying current target and event phase while 
 * dispatching a custom Event in the tree; it is also intended to be implemented, 
 * and not used, by DOM applications.
 * 
 * The methods contained in this interface are not intended to be used by a DOM 
 * application, especially during the dispatch on the Event object. Changing the 
 * current target or the current phase may result in unpredictable results of the 
 * event flow. The DOM Events implementation should ensure that both methods 
 * return the appropriate current target and phase before invoking each event 
 * listener on the current target to protect DOM applications from malicious 
 * event listeners.
 * 
 * Note: If this interface is supported by the event object, Event.isCustom() 
 * must return true.
*/
class CustomEvent : virtual public Event
{
public:

    /**
     * The setDispatchState method is used by the DOM Events implementation to set 
     * the values of Event.currentTarget and Event.eventPhase. It also reset the 
     * states of isPropagationStopped and isImmediatePropagationStopped.
     */
    virtual void setDispatchState(const EventTarget */*target*/,
                                  unsigned short /*phase*/)
        {
        }

    /**
     * This method will return true if the method stopPropagation() has been
     * called for this event, false in any other cases. 
     */
    virtual bool isPropagationStopped()
        {
        return propagationStopped;
        }

    /**
     * The isImmediatePropagationStopped method is used by the DOM Events 
     * implementation to know if the method stopImmediatePropagation() has been 
     * called for this event. It returns true if the method has been called, false 
     * otherwise.
     */
    virtual bool isImmediatePropagationStopped()
        {
        return immediatePropagationStopped;
        }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    CustomEvent() {}

    /**
     *
     */
    CustomEvent(const CustomEvent &other) : Event(other)
        {
        propagationStopped          = other.propagationStopped;
        immediatePropagationStopped = other.immediatePropagationStopped;
        }

    /**
     *
     */
    virtual ~CustomEvent() {}

protected:

    bool propagationStopped;
    bool immediatePropagationStopped;



};




/*#########################################################################
## UIEvent
#########################################################################*/

/**
 * The UIEvent interface provides specific contextual information associated with 
 * User Interface events.
 * 
 * To create an instance of the UIEvent interface, use the 
 * DocumentEvent.createEvent("UIEvent") method call.
 * 
 * NOTE:
 * For dom level 2 and 3, note that views.idl and events.idl disagree on the
 *  name of Views.  We are using level -2- Views
 */
class UIEvent : virtual public Event
{
public:

    /**
     * The view attribute identifies the AbstractView from which the
     * event was generated.
     */
    virtual views::AbstractView getView()
        { return view; }

    /**
     * Specifies some detail information about the Event, depending on
     * the type of event.
     */
    virtual long getDetail()
        { return detail; }

    /**
     * The initUIEvent method is used to initialize the value of a UIEvent object and 
     * has the same behavior as Event.initEvent().
     */
    virtual void initUIEvent(const DOMString &/*typeArg*/,
                             bool /*canBubbleArg*/,
                             bool /*cancelableArg*/,
                             const views::AbstractView */*viewArg*/,
                             long /*detailArg*/)
        {
        }

    /**
     * The initUIEventNS method is used to initialize the value of a UIEvent object 
     * and has the same behavior as Event.initEventNS().
     */
    virtual void initUIEventNS(const DOMString &/*namespaceURI*/,
                               const DOMString &/*typeArg*/,
                               bool /*canBubbleArg*/,
                               bool /*cancelableArg*/,
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
    UIEvent() {}

    /**
     *
     */
    UIEvent(const UIEvent &other) : Event(other)
        {
        view   = other.view;
        detail = other.detail;
        }

    /**
     *
     */
    virtual ~UIEvent() {}

protected:

    views::AbstractView view;
    long detail;
};




/*#########################################################################
## TextEvent
#########################################################################*/

/** 
 * The TextEvent interface provides specific contextual information associated
 * with Text Events.
 *
 * To create an instance of the TextEvent interface, use the
 * DocumentEvent.createEvent("TextEvent") method call.   
 */
class TextEvent : virtual public UIEvent
{
public:

    /**
     * data holds the value of the characters generated by the character device. This 
     * may be a single Unicode character or a non-empty sequence of Unicode 
     * characters [Unicode]. Characters should be normalized as defined by the 
     * Unicode normalization form NFC, defined in [UTR #15]. This attribute cannot be 
     * null or contain the empty string.
     */
    virtual DOMString getData()
        { return data; }

    /**
     * The initTextEvent method is used to initialize the value of a TextEvent object 
     * and has the same behavior as UIEvent.initUIEvent(). The value of 
     * UIEvent.detail remains undefined.
     */
    virtual void initTextEvent(const DOMString &/*typeArg*/,
                               bool /*canBubbleArg*/,
                               bool /*cancelableArg*/,
                               const views::AbstractView */*viewArg*/,
                               long /*detailArg*/)
        {
        }

    /**
     * The initTextEventNS method is used to initialize the value of a TextEvent 
     * object and has the same behavior as UIEvent.initUIEventNS(). The value of 
     * UIEvent.detail remains undefined.
     */
    virtual void initTextEventNS(const DOMString &/*namespaceURI*/,
                                 const DOMString &/*typeArg*/,
                                 bool /*canBubbleArg*/,
                                 bool /*cancelableArg*/,
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
    TextEvent() {}

    /**
     *
     */
    TextEvent(const TextEvent &other) : Event(other), UIEvent(other)
        {
        data = other.data;
        }

    /**
     *
     */
    virtual ~TextEvent() {}

protected:

    DOMString data;

};








/*#########################################################################
## MouseEvent
#########################################################################*/

/**
 * The MouseEvent interface provides specific contextual information associated 
 * with Mouse events.
 * 
 * In the case of nested elements mouse events are always targeted at the most 
 * deeply nested element. Ancestors of the targeted element may use bubbling to 
 * obtain notification of mouse events which occur within theirs descendent 
 * elements.
 * 
 * To create an instance of the MouseEvent interface, use the 
 * DocumentEvent.createEvent("MouseEvent") method call.
 */
class MouseEvent : virtual public UIEvent
{
public:

    /**
     * The horizontal coordinate at which the event occurred relative to the
     * origin of the screen coordinate system.
     */
    virtual long getScreenX()
        { return screenX; }

    /**
     * The vertical coordinate at which the event occurred relative to the
     *  origin of the screen coordinate system.
     */
    virtual long getScreenY()
        { return screenY; }

    /**
     * The horizontal coordinate at which the event occurred relative to the
     *  DOM implementation's client area.
     */
    virtual long getClientX()
        { return clientX; }

    /**
     * The vertical coordinate at which the event occurred relative to the
     * DOM implementation's client area.
     */
    virtual long getClientY()
        { return clientY; }

    /**
     * true if the control (Ctrl) key modifier is activated. 
     */
    virtual bool getCtrlKey()
        { return ctrlKey; }

    /**
     * true if the shift (Shift) key modifier is activated. 
     */
    virtual bool getShiftKey()
        { return shiftKey; }

    /**
     * true if the alt (alternative) key modifier is activated. 
     */
    virtual bool getAltKey()
        { return altKey; }

    /**
     * true if the meta (Meta) key modifier is activated. 
     */
    virtual bool getMetaKey()
        { return metaKey; }

    /**
     * During mouse events caused by the depression or release of a mouse button, 
     * button is used to indicate which mouse button changed state. 0 indicates the 
     * normal button of the mouse (in general on the left or the one button on 
     * Macintosh mice, used to activate a button or select text). 2 indicates the 
     * contextual property (in general on the right, used to display a context menu) 
     * button of the mouse if present. 1 indicates the extra (in general in the 
     * middle and often combined with the mouse wheel) button. Some mice may provide 
     * or simulate more buttons, and values higher than 2 can be used to represent 
     * such buttons.
     */
    virtual unsigned short getButton()
        { return button; }

    /**
     * Used to identify a secondary EventTarget related to a UI event. Currently this 
     * attribute is used with the mouseover event to indicate the EventTarget which 
     * the pointing device exited and with the mouseout event to indicate the 
     * EventTarget which the pointing device entered.
     */
    virtual EventTarget *getRelatedTarget()
        { return relatedTarget; }


    /**
     * This methods queries the state of a modifier using a key identifier.
     * The argument is a modifier key identifier, as defined by the 
     * KeyboardEvent.keyIdentifier attribute. Common modifier keys are "Alt", 
     * "AltGraph", "CapsLock", "Control", "Meta", "NumLock", "Scroll", or "Shift".
     */
    virtual bool getModifierState(const DOMString &/*id*/)
        { return false; }

    /**
     * The initMouseEvent method is used to initialize the value of a MouseEvent
     * object and has the same behavior as UIEvent.initUIEvent(). 
     */
    virtual void initMouseEvent(const DOMString &/*typeArg*/,
                                bool /*canBubbleArg*/,
                                bool /*cancelableArg*/,
                                const views::AbstractView */*viewArg*/,
                                long /*detailArg*/,
                                long /*screenXArg*/,
                                long /*screenYArg*/,
                                long /*clientXArg*/,
                                long /*clientYArg*/,
                                bool /*ctrlKeyArg*/,
                                bool /*altKeyArg*/,
                                bool /*shiftKeyArg*/,
                                bool /*metaKeyArg*/,
                                unsigned short /*buttonArg*/,
                                const EventTarget */*relatedTargetArg*/)
        {
        }


    /**
     * The initMouseEventNS method is used to initialize the value of a
     * MouseEvent object and has the same behavior as UIEvent.initUIEventNS(). 
     */
    virtual void initMouseEventNS(const DOMString &/*namespaceURI*/,
                                  const DOMString &/*typeArg*/,
                                  bool /*canBubbleArg*/,
                                  bool /*cancelableArg*/,
                                  const views::AbstractView */*viewArg*/,
                                  long /*detailArg*/,
                                  long /*screenXArg*/,
                                  long /*screenYArg*/,
                                  long /*clientXArg*/,
                                  long /*clientYArg*/,
                                  unsigned short /*buttonArg*/,
                                  const EventTarget */*relatedTargetArg*/,
                                  const DOMString &/*modifiersList*/)
        {
        }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    MouseEvent() {}

    /**
     *
     */
    MouseEvent(const MouseEvent &other) : Event(other), UIEvent(other)
        {
        screenX       = other.screenX;
        screenY       = other.screenY;
        clientX       = other.clientX;
        clientY       = other.clientY;
        ctrlKey       = other.ctrlKey;
        shiftKey      = other.shiftKey;
        altKey        = other.altKey;
        metaKey       = other.metaKey;
        button        = other.button;
        relatedTarget = other.relatedTarget;
        }

    /**
     *
     */
    virtual ~MouseEvent() {}

protected:

    long screenX;
    long screenY;
    long clientX;
    long clientY;
    bool ctrlKey;
    bool shiftKey;
    bool altKey;
    bool metaKey;
    unsigned short button;
    EventTarget *relatedTarget;
};




/*#########################################################################
## KeyboardEvent
#########################################################################*/

/**
 * The KeyboardEvent interface provides specific contextual information 
 * associated with keyboard devices. Each keyboard event references a key using 
 * an identifier. Keyboard events are commonly directed at the element that has 
 * the focus.
 * 
 * The KeyboardEvent interface provides convenient attributes for some common 
 * modifiers keys: KeyboardEvent.ctrlKey, KeyboardEvent.shiftKey, 
 * KeyboardEvent.altKey, KeyboardEvent.metaKey. These attributes are equivalent 
 * to use the method KeyboardEvent.getModifierState(keyIdentifierArg) with 
 * "Control", "Shift", "Alt", or "Meta" respectively.
 * 
 * To create an instance of the KeyboardEvent interface, use the 
 * DocumentEvent.createEvent("KeyboardEvent") method call.
 */
class KeyboardEvent : virtual public UIEvent
{
public:

    /**
     * This set of constants is used to indicate the location of a key on
     * the device. In case a DOM implementation wishes to provide a new
     * location information, a value different from the following constant
     * values must be used. 
     */	     
    typedef enum
        {
        DOM_KEY_LOCATION_STANDARD      = 0x00,
        DOM_KEY_LOCATION_LEFT          = 0x01,
        DOM_KEY_LOCATION_RIGHT         = 0x02,
        DOM_KEY_LOCATION_NUMPAD        = 0x03
        } KeyLocationCode;

    /**
     * keyIdentifier holds the identifier of the key.
     * Key identifiers can be found here:
     * http://www.w3.org/TR/2003/NOTE-DOM-Level-3-Events-20031107/keyset.html#KeySet-Set
     * Implementations that are unable to identify a key must use the key
     * identifier "Unidentified".      
     */
    virtual DOMString getKeyIdentifier()
        { return keyIdentifier; }

    /**
     * The keyLocation attribute contains an indication of the location of
     * they key on the device, as described in:
     * http://www.w3.org/TR/2003/NOTE-DOM-Level-3-Events-20031107/events.html#ID-KeyboardEvent-KeyLocationCode
     */
    virtual unsigned long getKeyLocation()
        { return keyLocation; }

    /**
     * true if the control (Ctrl) key modifier is activated. 
     */
    virtual bool getCtrlKey()
        { return ctrlKey; }

    /**
     * true if the shift (Shift) key modifier is activated. 
     */
    virtual bool getShiftKey()
        { return shiftKey; }

    /**
     * true if the alternative (Alt) key modifier is activated. 
     */
    virtual bool getAltKey()
        { return altKey; }

    /**
     * true if the meta (Meta) key modifier is activated. 
     */
    virtual bool getMetaKey()
        { return metaKey; }

    /**
     * This methods queries the state of a modifier using a key identifier.
     * The argument is a modifier key identifier. Common modifier keys are "Alt", 
     * "AltGraph", "CapsLock", "Control", "Meta", "NumLock", "Scroll", or "Shift".
     */
    virtual bool getModifierState(const DOMString &/*id*/)
        { return false; }

    /**
     * The initKeyboardEvent method is used to initialize the value of a 
     * KeyboardEvent object and has the same behavior as UIEvent.initUIEvent(). The 
     * value of UIEvent.detail remains undefined.
     */
    virtual void initKeyboardEvent(const DOMString &/*typeArg*/,
                                   bool /*canBubbleArg*/,
                                   bool /*cancelableArg*/,
                                   const views::AbstractView */*viewArg*/,
                                   const DOMString &/*keyIdentifier*/,
                                   unsigned long /*keyLocation*/,
                                   const DOMString /*modifiersList*/)
        {
        }



    /**
     * The initKeyboardEventNS method is used to initialize the value of a 
     * KeyboardEvent object and has the same behavior as UIEvent.initUIEventNS(). The 
     * value of UIEvent.detail remains undefined.
     */
    virtual void initKeyboardEventNS(const DOMString &/*namespaceURI*/,
                                     const DOMString &/*typeArg*/,
                                     bool /*canBubbleArg*/,
                                     bool /*cancelableArg*/,
                                     const views::AbstractView */*viewArg*/,
                                     const DOMString &/*keyIdentifier*/,
                                     unsigned long /*keyLocation*/,
                                     const DOMString /*modifiersList*/)
        {
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    KeyboardEvent() {}

    /**
     *
     */
    KeyboardEvent(const KeyboardEvent &other) : Event(other), UIEvent(other)
        {
        keyIdentifier = other.keyIdentifier;
        keyLocation   = other.keyLocation;
        ctrlKey       = other.ctrlKey;
        shiftKey      = other.shiftKey;
        altKey        = other.altKey;
        metaKey       = other.metaKey;
        }

    /**
     *
     */
    virtual ~KeyboardEvent() {}

protected:

    DOMString keyIdentifier;
    unsigned long keyLocation;
    bool ctrlKey;
    bool shiftKey;
    bool altKey;
    bool metaKey;
};









/*#########################################################################
## MutationEvent
#########################################################################*/

/**
 * The MutationEvent interface provides specific contextual information 
 * associated with Mutation events.
 * 
 * To create an instance of the MutationEvent interface, use the 
 * DocumentEvent.createEvent("MutationEvent") method call.
 */
class MutationEvent : virtual public Event
{
public:

    /**
     * An integer indicating in which way the Attr was changed. 
     */
    typedef enum
        {
        MODIFICATION = 1,
        ADDITION     = 2,
        REMOVAL      = 3
        } AttrChangeType;

    /**
     * relatedNode is used to identify a secondary node related to a mutation event. 
     * For example, if a mutation event is dispatched to a node indicating that its 
     * parent has changed, the relatedNode is the changed parent. If an event is 
     * instead dispatched to a subtree indicating a node was changed within it, the 
     * relatedNode is the changed node. In the case of the 
     * {"http://www.w3.org/2001/xml-events", "DOMAttrModified"} event it indicates 
     * the Attr node which was modified, added, or removed.
     */
    virtual NodePtr getRelatedNode()
        { return relatedNodePtr ; }

    /**
     * prevValue indicates the previous value of the Attr node in 
     * {"http://www.w3.org/2001/xml-events", "DOMAttrModified"} events, and of the 
     * CharacterData node in {"http://www.w3.org/2001/xml-events", 
     * "DOMCharacterDataModified"} events.
     */
    virtual DOMString getPrevValue()
        { return prevValue; }

    /**
     * newValue indicates the new value of the Attr node in 
     * {"http://www.w3.org/2001/xml-events", "DOMAttrModified"} events, and of the 
     * CharacterData node in {"http://www.w3.org/2001/xml-events", 
     * "DOMCharacterDataModified"} events.
     */
    virtual DOMString getNewValue()
        { return newValue; }

    /**
     * attrName indicates the name of the changed Attr node in a 
     * {"http://www.w3.org/2001/xml-events", "DOMAttrModified"} event.
     */
    virtual DOMString getAttrName()
        { return attrName; }

    /**
     * attrChange indicates the type of change which triggered the 
     * {"http://www.w3.org/2001/xml-events", "DOMAttrModified"} event. The values can 
     * be MODIFICATION, ADDITION, or REMOVAL.
     */
    virtual unsigned short getAttrChange()
        {
        return attrChange;
        }

    /**
     * The initMutationEvent method is used to initialize the value of a 
     * MutationEvent object and has the same behavior as Event.initEvent().
     */
    virtual void initMutationEvent(const DOMString &/*typeArg*/,
                                   bool /*canBubbleArg*/,
                                   bool /*cancelableArg*/,
                                   const NodePtr   /*relatedNodeArg*/,
                                   const DOMString &/*prevValueArg*/,
                                   const DOMString &/*newValueArg*/,
                                   const DOMString &/*attrNameArg*/,
                                   unsigned short /*attrChangeArg*/)
        {
        }

    /**
     * The initMutationEventNS method is used to initialize the value of a 
     * MutationEvent object and has the same behavior as Event.initEventNS().
     */
    virtual void initMutationEventNS(const DOMString &/*namespaceURI*/,
                                     const DOMString &/*typeArg*/,
                                     bool /*canBubbleArg*/,
                                     bool /*cancelableArg*/,
                                     const NodePtr   /*relatedNodeArg*/,
                                     const DOMString &/*prevValueArg*/,
                                     const DOMString &/*newValueArg*/,
                                     const DOMString &/*attrNameArg*/,
                                     unsigned short /*attrChangeArg*/)
        {
        }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    MutationEvent()
        {
        relatedNodePtr  = NULL;
        }

    /**
     *
     */
    MutationEvent(const MutationEvent &other) : Event(other)
        {
        relatedNodePtr  = other.relatedNodePtr ;
        prevValue   = other.prevValue;
        newValue    = other.newValue;
        attrName    = other.attrName;
        attrChange  = other.attrChange;
        }

    /**
     *
     */
    virtual ~MutationEvent() {}

protected:

    NodePtr   relatedNodePtr ;
    DOMString prevValue;
    DOMString newValue;
    DOMString attrName;
    unsigned short attrChange;

};




/*#########################################################################
## MutationNameEvent
#########################################################################*/

/**
 * The MutationNameEvent interface provides specific contextual information 
 * associated with Mutation name event types.
 * 
 * To create an instance of the MutationNameEvent interface, use the 
 * Document.createEvent("MutationNameEvent") method call.
 */
class MutationNameEvent : virtual public MutationEvent
{
public:

    /**
     * The previous value of the relatedNode's namespaceURI. 
     */
    virtual DOMString getPrevNamespaceURI()
        { return prevNamespaceURI; }

    /**
     * The previous value of the relatedNode's nodeName. 
     */
    virtual DOMString getPrevNodeName()
        { return prevNodeName; }

    /**
     * The initMutationNameEvent method is used to initialize the value of a 
     * MutationNameEvent object and has the same behavior as 
     * MutationEvent.initMutationEvent().
     */
    virtual void initMutationNameEvent(const DOMString &/*typeArg*/,
                                       bool /*canBubbleArg*/,
                                       bool /*cancelableArg*/,
                                       const NodePtr   /*relatedNodeArg*/,
                                       const DOMString &/*prevNamespaceURIArg*/,
                                       const DOMString &/*prevNodeNameArg*/)
        {
        }


    /**
     * The initMutationNameEventNS method is used to initialize the value of a 
     * MutationNameEvent object and has the same behavior as 
     * MutationEvent.initMutationEventNS().
     */
    virtual void initMutationNameEventNS(const DOMString &/*namespaceURI*/,
                                         const DOMString &/*typeArg*/,
                                         bool /*canBubbleArg*/,
                                         bool /*cancelableArg*/,
                                         const NodePtr   /*relatedNodeArg*/,
                                         const DOMString &/*prevNamespaceURIArg*/,
                                         const DOMString &/*prevNodeNameArg*/)
        {
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    MutationNameEvent() {}


    /**
     *
     */
    MutationNameEvent(const MutationNameEvent &other) 
                          : Event(other), MutationEvent(other)
        {
        prevNamespaceURI  = other.prevNamespaceURI;
        prevNodeName      = other.prevNodeName;
        }


    /**
     *
     */
    virtual ~MutationNameEvent() {}

protected:

    DOMString prevNamespaceURI;
    DOMString prevNodeName;


};






}  //namespace events
}  //namespace dom
}  //namespace w3c
}  //namespace org

#endif   /* __EVENTS_H__ */

/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/

