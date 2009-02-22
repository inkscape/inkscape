/*
 * Copyright (c) 2003 World Wide Web Consortium,
 *
 * (Massachusetts Institute of Technology, European Research Consortium for
 * Informatics and Mathematics, Keio University). All Rights Reserved. This
 * work is distributed under the W3C(r) Software License [1] in the hope that
 * it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * [1] http://www.w3.org/Consortium/Legal/2002/copyright-software-20021231
 */

package org.w3c.dom.events;

import org.w3c.dom.DOMException;

/**
 *  The <code>DocumentEvent</code> interface provides a mechanism by which the 
 * user can create an <code>Event</code> object of a type supported by the 
 * implementation. If the feature "Events" is supported by the 
 * <code>Document</code> object, the <code>DocumentEvent</code> interface 
 * must be implemented on the same object. If the feature "+Events" is 
 * supported by the <code>Document</code> object, an object that supports 
 * the <code>DocumentEvent</code> interface must be returned by invoking the 
 * method <code>Node.getFeature("+Events", "3.0")</code> on the 
 * <code>Document</code> object. 
 * <p>See also the <a href='http://www.w3.org/TR/2003/NOTE-DOM-Level-3-Events-20031107'>Document Object Model (DOM) Level 3 Events Specification</a>.
 * @since DOM Level 2
 */
public interface DocumentEvent {
    /**
     * 
     * @param eventType  The <code>eventType</code> parameter specifies the 
     *   name of the DOM Events interface to be supported by the created 
     *   event object, e.g. <code>"Event"</code>, <code>"MouseEvent"</code>, 
     *   <code>"MutationEvent"</code> and so on. If the <code>Event</code> 
     *   is to be dispatched via the <code>EventTarget.dispatchEvent()</code>
     *    method the appropriate event init method must be called after 
     *   creation in order to initialize the <code>Event</code>'s values.  
     *   As an example, a user wishing to synthesize some kind of 
     *   <code>UIEvent</code> would invoke 
     *   <code>DocumentEvent.createEvent("UIEvent")</code>. The 
     *   <code>UIEvent.initUIEventNS()</code> method could then be called on 
     *   the newly created <code>UIEvent</code> object to set the specific 
     *   type of user interface event to be dispatched, 
     *   <code>{"http://www.w3.org/2001/xml-events", "DOMActivate"}</code> 
     *   for example, and set its context information, e.g. 
     *   <code>UIEvent.detail</code> in this example.  The 
     *   <code>createEvent</code> method is used in creating 
     *   <code>Event</code>s when it is either inconvenient or unnecessary 
     *   for the user to create an <code>Event</code> themselves. In cases 
     *   where the implementation provided <code>Event</code> is 
     *   insufficient, users may supply their own <code>Event</code> 
     *   implementations for use with the 
     *   <code>EventTarget.dispatchEvent()</code> method. However, the DOM 
     *   implementation needs access to the attributes 
     *   <code>Event.currentTarget</code> and <code>Event.eventPhase</code> 
     *   to appropriately propagate the event in the DOM tree. Therefore 
     *   users' <code>Event</code> implementations might need to support the 
     *   <code>CustomEvent</code> interface for that effect. 
     * <p ><b>Note:</b>    For backward compatibility reason, "UIEvents", 
     *   "MouseEvents", "MutationEvents", and "HTMLEvents" feature names are 
     *   valid values for the parameter <code>eventType</code> and represent 
     *   respectively the interfaces "UIEvent", "MouseEvent", 
     *   "MutationEvent", and "Event". 
     * @return  The newly created event object. 
     * @exception DOMException
     *    NOT_SUPPORTED_ERR: Raised if the implementation does not support the 
     *   <code>Event</code> interface requested. 
     */
    public Event createEvent(String eventType)
                             throws DOMException;

    /**
     *  Test if the implementation can generate events of a specified type. 
     * @param namespaceURI  Specifies the <code>Event.namespaceURI</code> of 
     *   the event. 
     * @param type  Specifies the <code>Event.type</code> of the event. 
     * @return  <code>true</code> if the implementation can generate and 
     *   dispatch this event type, <code>false</code> otherwise. 
     * @since DOM Level 3
     */
    public boolean canDispatch(String namespaceURI, 
                               String type);

}
