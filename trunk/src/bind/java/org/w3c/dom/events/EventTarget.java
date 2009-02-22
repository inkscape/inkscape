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

/**
 *  The <code>EventTarget</code> interface is implemented by all the objects 
 * which could be event targets in an implementation which supports the . 
 * The interface allows registration, removal or query of event listeners, 
 * and dispatch of events to an event target. 
 * <p> When used with , this interface is implemented by all target nodes and 
 * target ancestors, i.e. all DOM <code>Nodes</code> of the tree support 
 * this interface when the implementation conforms to DOM Level 3 Events 
 * and, therefore, this interface can be obtained by using binding-specific 
 * casting methods on an instance of the <code>Node</code> interface. 
 * <p> Invoking <code>addEventListener</code> or 
 * <code>addEventListenerNS</code> multiple times on the same 
 * <code>EventTarget</code> with the same parameters (
 * <code>namespaceURI</code>, <code>type</code>, <code>listener</code>, and 
 * <code>useCapture</code>) is considered to be a no-op and thus 
 * independently of the event group. They do not cause the 
 * <code>EventListener</code> to be called more than once and do not cause a 
 * change in the triggering order. In order to guarantee that an event 
 * listener will be added to the event target for the specified event group, 
 * one needs to invoke <code>removeEventListener</code> or 
 * <code>removeEventListenerNS</code> first. 
 * <p>See also the <a href='http://www.w3.org/TR/2003/NOTE-DOM-Level-3-Events-20031107'>Document Object Model (DOM) Level 3 Events Specification</a>.
 * @since DOM Level 2
 */
public interface EventTarget {
    /**
     *  This method allows the registration of an event listener in the 
     * default group and, depending on the <code>useCapture</code> 
     * parameter, on the capture phase of the DOM event flow or its target 
     * and bubbling phases. 
     * @param type  Specifies the <code>Event.type</code> associated with the 
     *   event for which the user is registering. 
     * @param listener  The <code>listener</code> parameter takes an object 
     *   implemented by the user which implements the 
     *   <code>EventListener</code> interface and contains the method to be 
     *   called when the event occurs. 
     * @param useCapture  If true, <code>useCapture</code> indicates that the 
     *   user wishes to add the event listener for the capture phase only, 
     *   i.e. this event listener will not be triggered during the target 
     *   and bubbling phases. If <code>false</code>, the event listener will 
     *   only be triggered during the target and bubbling phases. 
     */
    public void addEventListener(String type, 
                                 EventListener listener, 
                                 boolean useCapture);

    /**
     *  This method allows the removal of event listeners from the default 
     * group. 
     * <br> Calling <code>removeEventListener</code> with arguments which do 
     * not identify any currently registered <code>EventListener</code> on 
     * the <code>EventTarget</code> has no effect. 
     * @param type  Specifies the <code>Event.type</code> for which the user 
     *   registered the event listener. 
     * @param listener  The <code>EventListener</code> to be removed. 
     * @param useCapture  Specifies whether the <code>EventListener</code> 
     *   being removed was registered for the capture phase or not. If a 
     *   listener was registered twice, once for the capture phase and once 
     *   for the target and bubbling phases, each must be removed 
     *   separately. Removal of an event listener registered for the capture 
     *   phase does not affect the same event listener registered for the 
     *   target and bubbling phases, and vice versa. 
     */
    public void removeEventListener(String type, 
                                    EventListener listener, 
                                    boolean useCapture);

    /**
     *  This method allows the dispatch of events into the implementation's 
     * event model. The event target of the event is the 
     * <code>EventTarget</code> object on which <code>dispatchEvent</code> 
     * is called. 
     * @param evt  The event to be dispatched. 
     * @return  Indicates whether any of the listeners which handled the 
     *   event called <code>Event.preventDefault()</code>. If 
     *   <code>Event.preventDefault()</code> was called the returned value 
     *   is <code>false</code>, else it is <code>true</code>. 
     * @exception EventException
     *    UNSPECIFIED_EVENT_TYPE_ERR: Raised if the <code>Event.type</code> 
     *   was not specified by initializing the event before 
     *   <code>dispatchEvent</code> was called. Specification of the 
     *   <code>Event.type</code> as <code>null</code> or an empty string 
     *   will also trigger this exception. 
     *   <br> DISPATCH_REQUEST_ERR: Raised if the <code>Event</code> object is 
     *   already being dispatched in the tree. 
     *   <br> NOT_SUPPORTED_ERR: Raised if the <code>Event</code> object has 
     *   not been created using <code>DocumentEvent.createEvent()</code> or 
     *   does not support the interface <code>CustomEvent</code>. 
     * @version DOM Level 3
     */
    public boolean dispatchEvent(Event evt)
                                 throws EventException;

    /**
     *  This method allows the registration of an event listener in a 
     * specified group or the default group and, depending on the 
     * <code>useCapture</code> parameter, on the capture phase of the DOM 
     * event flow or its target and bubbling phases. 
     * @param namespaceURI  Specifies the <code>Event.namespaceURI</code> 
     *   associated with the event for which the user is registering. 
     * @param type  Specifies the <code>Event.type</code> associated with the 
     *   event for which the user is registering. 
     * @param listener  The <code>listener</code> parameter takes an object 
     *   implemented by the user which implements the 
     *   <code>EventListener</code> interface and contains the method to be 
     *   called when the event occurs. 
     * @param useCapture  If true, <code>useCapture</code> indicates that the 
     *   user wishes to add the event listener for the capture phase only, 
     *   i.e. this event listener will not be triggered during the target 
     *   and bubbling phases. If <code>false</code>, the event listener will 
     *   only be triggered during the target and bubbling phases. 
     * @param evtGroup  The object that represents the event group to 
     *   associate with the <code>EventListener</code> (see also ). Use 
     *   <code>null</code> to attach the event listener to the default 
     *   group. 
     * @since DOM Level 3
     */
    public void addEventListenerNS(String namespaceURI, 
                                   String type, 
                                   EventListener listener, 
                                   boolean useCapture, 
                                   Object evtGroup);

    /**
     *  This method allows the removal of an event listener, independently of 
     * the associated event group. 
     * <br> Calling <code>removeEventListenerNS</code> with arguments which do 
     * not identify any currently registered <code>EventListener</code> on 
     * the <code>EventTarget</code> has no effect. 
     * @param namespaceURI  Specifies the <code>Event.namespaceURI</code> 
     *   associated with the event for which the user registered the event 
     *   listener. 
     * @param type  Specifies the <code>Event.type</code> associated with the 
     *   event for which the user registered the event listener. 
     * @param listener  The <code>EventListener</code> parameter indicates 
     *   the <code>EventListener</code> to be removed. 
     * @param useCapture  Specifies whether the <code>EventListener</code> 
     *   being removed was registered for the capture phase or not. If a 
     *   listener was registered twice, once for the capture phase and once 
     *   for the target and bubbling phases, each must be removed 
     *   separately. Removal of an event listener registered for the capture 
     *   phase does not affect the same event listener registered for the 
     *   target and bubbling phases, and vice versa. 
     * @since DOM Level 3
     */
    public void removeEventListenerNS(String namespaceURI, 
                                      String type, 
                                      EventListener listener, 
                                      boolean useCapture);

    /**
     *  This method allows the DOM application to know if an event listener, 
     * attached to this <code>EventTarget</code> or one of its ancestors, 
     * will be triggered by the specified event type during the dispatch of 
     * the event to this event target or one of its descendants. 
     * @param namespaceURI  Specifies the <code>Event.namespaceURI</code> 
     *   associated with the event. 
     * @param type  Specifies the <code>Event.type</code> associated with the 
     *   event. 
     * @return  <code>true</code> if an event listener will be triggered on 
     *   the <code>EventTarget</code> with the specified event type, 
     *   <code>false</code> otherwise. 
     * @since DOM Level 3
     */
    public boolean willTriggerNS(String namespaceURI, 
                                 String type);

    /**
     *  This method allows the DOM application to know if this 
     * <code>EventTarget</code> contains an event listener registered for 
     * the specified event type. This is useful for determining at which 
     * nodes within a hierarchy altered handling of specific event types has 
     * been introduced, but should not be used to determine whether the 
     * specified event type triggers an event listener (see 
     * <code>EventTarget.willTriggerNS()</code>). 
     * @param namespaceURI  Specifies the <code>Event.namespaceURI</code> 
     *   associated with the event. 
     * @param type  Specifies the <code>Event.type</code> associated with the 
     *   event. 
     * @return  <code>true</code> if an event listener is registered on this 
     *   <code>EventTarget</code> for the specified event type, 
     *   <code>false</code> otherwise. 
     * @since DOM Level 3
     */
    public boolean hasEventListenerNS(String namespaceURI, 
                                      String type);

}
