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
 *
 */
class EventException
{
public:

    EventException(short theCode)
        {
            code = theCode;
        }

    virtual ~EventException() throw()
       {}

    unsigned short code;
};

    /**
     * EventExceptionCode
     */
    enum
        {
        UNSPECIFIED_EVENT_TYPE_ERR = 0,
        DISPATCH_REQUEST_ERR       = 1
        };



/*#########################################################################
## Event
#########################################################################*/

/**
 *
 */
class Event
{
public:

    /**
     * PhaseType
     */
    typedef enum
        {
        CAPTURING_PHASE = 1,
        AT_TARGET       = 2,
        BUBBLING_PHASE  = 3
        } PhaseType;

    /**
     *
     */
    virtual DOMString getType() const
        { return eventType; }

    /**
     *
     */
    virtual EventTarget *getTarget()
        { return target; }

    /**
     *
     */
    virtual EventTarget *getCurrentTarget()
        { return currentTarget; }

    /**
     *
     */
    virtual unsigned short getEventPhase()
        { return eventPhase; }

    /**
     *
     */
    virtual bool getBubbles()
        { return canBubble; }

    /**
     *
     */
    virtual bool getCancelable()
        { return cancelable; }

    /**
     *
     */
    virtual DOMTimeStamp getTimeStamp()
        { return timeStamp; }

    /**
     *
     */
    virtual void stopPropagation()
        {
        }

    /**
     *
     */
    virtual void preventDefault()
        {
        }

    /**
     *
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
     *
     */
    virtual DOMString getNamespaceURI() const
        { return namespaceURI; }

    /**
     *
     */
    virtual bool isCustom()
        { return custom; }

    /**
     *
     */
    virtual void stopImmediatePropagation()
        {
        }

    /**
     *
     */
    virtual bool isDefaultPrevented()
        { return defaultPrevented; }

    /**
     *
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
 *
 */
class EventListener
{
public:

    /**
     *
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


/**
 *
 */
class EventTarget
{
public:

    /**
     *
     */
    virtual void addEventListener(const DOMString &type,
                                  const EventListener *listener,
                                  bool useCapture)
        {
        EventListenerEntry entry("", type, listener, useCapture);
        listeners.push_back(entry);
        }

    /**
     *
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
     *
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
     *
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
     *
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
     *
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
     *
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

/*
 *
 */
class DocumentEvent : virtual public Event
{
public:

    /**
     *
     */
    virtual Event createEvent(const DOMString &/*eventType*/)
                               throw (dom::DOMException)
        {
        Event event;
        return event;
        }

    /**
     *
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

/*
 *
 */
class CustomEvent : virtual public Event
{
public:

    /**
     *
     */
    virtual void setDispatchState(const EventTarget */*target*/,
                                  unsigned short /*phase*/)
        {
        }

    /**
     *
     */
    virtual bool isPropagationStopped()
        {
        return propagationStopped;
        }

    /**
     *
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
 *
 */
class UIEvent : virtual public Event
{
public:

    /**
     *  Note that the return type is level -2- views
     */
    virtual views::AbstractView getView()
        { return view; }

    /**
     *
     */
    virtual long getDetail()
        { return detail; }

    /**
     *  Note that views.idl and events.idl disagree on the name of Views
     */
    virtual void initUIEvent(const DOMString &/*typeArg*/,
                             bool /*canBubbleArg*/,
                             bool /*cancelableArg*/,
                             const views::AbstractView */*viewArg*/,
                             long /*detailArg*/)
        {
        }

    /**
     *  Note that views.idl and events.idl disagree on the name of Views
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
 *
 */
class TextEvent : virtual public UIEvent
{
public:

    /**
     *
     */
    virtual DOMString getData()
        { return data; }

    /**
     *  Note that views.idl and events.idl disagree on the name of Views
     */
    virtual void initTextEvent(const DOMString &/*typeArg*/,
                               bool /*canBubbleArg*/,
                               bool /*cancelableArg*/,
                               const views::AbstractView */*viewArg*/,
                               long /*detailArg*/)
        {
        }

    /**
     *
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
 *
 */
class MouseEvent : virtual public UIEvent
{
public:

    /**
     *
     */
    virtual long getScreenX()
        { return screenX; }

    /**
     *
     */
    virtual long getScreenY()
        { return screenY; }

    /**
     *
     */
    virtual long getClientX()
        { return clientX; }

    /**
     *
     */
    virtual long getClientY()
        { return clientY; }

    /**
     *
     */
    virtual bool getCtrlKey()
        { return ctrlKey; }

    /**
     *
     */
    virtual bool getShiftKey()
        { return shiftKey; }

    /**
     *
     */
    virtual bool getAltKey()
        { return altKey; }

    /**
     *
     */
    virtual bool getMetaKey()
        { return metaKey; }

    /**
     *
     */
    virtual unsigned short getButton()
        { return button; }

    /**
     *
     */
    virtual EventTarget *getRelatedTarget()
        { return relatedTarget; }


    /**
     *
     */
    virtual bool getModifierState()
        { return modifierState; }

    /**
     *
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
     *
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
        modifierState = other.modifierState;
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
    bool modifierState;


};




/*#########################################################################
## KeyboardEvent
#########################################################################*/

/**
 *
 */
class KeyboardEvent : virtual public UIEvent
{
public:

    typedef enum
        {
        DOM_KEY_LOCATION_STANDARD      = 0x00,
        DOM_KEY_LOCATION_LEFT          = 0x01,
        DOM_KEY_LOCATION_RIGHT         = 0x02,
        DOM_KEY_LOCATION_NUMPAD        = 0x03
        } KeyLocationCode;

    /**
     *
     */
    virtual DOMString getKeyIdentifier()
        { return keyIdentifier; }

    /**
     *
     */
    virtual unsigned long getKeyLocation()
        { return keyLocation; }

    /**
     *
     */
    virtual bool getCtrlKey()
        { return ctrlKey; }

    /**
     *
     */
    virtual bool getShiftKey()
        { return shiftKey; }

    /**
     *
     */
    virtual bool getAltKey()
        { return altKey; }

    /**
     *
     */
    virtual bool getMetaKey()
        { return metaKey; }

    /**
     *
     */
    virtual bool getModifierState()
        { return modifierState; }

    /**
     *
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
     *
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
        modifierState = other.modifierState;
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
    bool modifierState;
};









/*#########################################################################
## MutationEvent
#########################################################################*/

/**
 *
 */
class MutationEvent : virtual public Event
{
public:

    /**
     * attrChangeType
     */
    typedef enum
        {
        MODIFICATION = 1,
        ADDITION     = 2,
        REMOVAL      = 3
        } AttrChangeType;

    /**
     *
     */
    virtual NodePtr getRelatedNodePtr ()
        { return relatedNodePtr ; }

    /**
     *
     */
    virtual DOMString getPrevValue()
        { return prevValue; }

    /**
     *
     */
    virtual DOMString getNewValue()
        { return newValue; }

    /**
     *
     */
    virtual DOMString getAttrName()
        { return attrName; }

    /**
     *
     */
    virtual unsigned short getAttrChange()
        {
        return attrChange;
        }

    /**
     *
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
     *
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
 *
 */
class MutationNameEvent : virtual public MutationEvent
{
public:

    /**
     *
     */
    virtual DOMString getPrevNamespaceURI()
        { return prevNamespaceURI; }

    /**
     *
     */
    virtual DOMString getPrevNodeName()
        { return prevNodeName; }

    /**
     *
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
     *
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

