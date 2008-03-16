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
 *  The <code>Event</code> interface is used to provide contextual information 
 * about an event to the listener processing the event. An object which 
 * implements the <code>Event</code> interface is passed as the parameter to 
 * an <code>EventListener</code>. More specific context information is 
 * passed to event listeners by deriving additional interfaces from 
 * <code>Event</code> which contain information directly relating to the 
 * type of event they represent. These derived interfaces are also 
 * implemented by the object passed to the event listener. 
 * <p> To create an instance of the <code>Event</code> interface, use the 
 * <code>DocumentEvent.createEvent("Event")</code> method call. 
 * <p>See also the <a href='http://www.w3.org/TR/2003/NOTE-DOM-Level-3-Events-20031107'>Document Object Model (DOM) Level 3 Events Specification</a>.
 * @since DOM Level 2
 */
public interface Event {
    // PhaseType
    /**
     *  The current event phase is the capture phase. 
     */
    public static final short CAPTURING_PHASE           = 1;
    /**
     *  The current event is in the target phase, i.e. it is being evaluated 
     * at the event target. 
     */
    public static final short AT_TARGET                 = 2;
    /**
     *  The current event phase is the bubbling phase. 
     */
    public static final short BUBBLING_PHASE            = 3;

    /**
     *  The name should be an <a href='http://www.w3.org/TR/1999/REC-xml-names-19990114/#NT-NCName'>NCName</a> as defined in [<a href='http://www.w3.org/TR/1999/REC-xml-names-19990114/'>XML Namespaces</a>]
     *  and is case-sensitive. 
     * <br> If the attribute <code>Event.namespaceURI</code> is different from 
     * <code>null</code>, this attribute represents a local name. 
     */
    public String getType();

    /**
     *  Used to indicate the event target. This attribute contains the target 
     * node when used with the . 
     */
    public EventTarget getTarget();

    /**
     *  Used to indicate the <code>EventTarget</code> whose 
     * <code>EventListeners</code> are currently being processed. This is 
     * particularly useful during the capture and bubbling phases. This 
     * attribute could contain the target node or a target ancestor when 
     * used with the . 
     */
    public EventTarget getCurrentTarget();

    /**
     *  Used to indicate which phase of event flow is currently being 
     * accomplished. 
     */
    public short getEventPhase();

    /**
     *  Used to indicate whether or not an event is a bubbling event. If the 
     * event can bubble the value is <code>true</code>, otherwise the value 
     * is <code>false</code>. 
     */
    public boolean getBubbles();

    /**
     *  Used to indicate whether or not an event can have its default action 
     * prevented (see also ). If the default action can be prevented the 
     * value is <code>true</code>, otherwise the value is <code>false</code>
     * . 
     */
    public boolean getCancelable();

    /**
     *  Used to specify the time (in milliseconds relative to the epoch) at 
     * which the event was created. Due to the fact that some systems may 
     * not provide this information the value of <code>timeStamp</code> may 
     * be not available for all events. When not available, a value of 
     * <code>0</code> will be returned. Examples of epoch time are the time 
     * of the system start or 0:0:0 UTC 1st January 1970. 
     */
    public long getTimeStamp();

    /**
     *  This method is used to prevent event listeners of the same group to be 
     * triggered but its effect is deferred until all event listeners 
     * attached on the <code>currentTarget</code> have been triggered (see 
     * ). Once it has been called, further calls to that method have no 
     * additional effect. 
     * <p ><b>Note:</b>  This method does not prevent the default action from 
     * being invoked; use <code>preventDefault</code> for that effect. 
     */
    public void stopPropagation();

    /**
     *  If an event is cancelable, the <code>preventDefault</code> method is 
     * used to signify that the event is to be canceled, meaning any default 
     * action normally taken by the implementation as a result of the event 
     * will not occur (see also ), and thus independently of event groups. 
     * Calling this method for a non-cancelable event has no effect. 
     * <p ><b>Note:</b>  This method does not stop the event propagation; use 
     * <code>stopPropagation</code> or <code>stopImmediatePropagation</code> 
     * for that effect. 
     */
    public void preventDefault();

    /**
     *  The <code>initEvent</code> method is used to initialize the value of 
     * an <code>Event</code> created through the 
     * <code>DocumentEvent.createEvent</code> method. This method may only 
     * be called before the <code>Event</code> has been dispatched via the 
     * <code>EventTarget.dispatchEvent()</code> method. If the method is 
     * called several times before invoking 
     * <code>EventTarget.dispatchEvent</code>, only the final invocation 
     * takes precedence. This method has no effect if called after the event 
     * has been dispatched. If called from a subclass of the 
     * <code>Event</code> interface only the values specified in this method 
     * are modified, all other attributes are left unchanged. 
     * <br> This method sets the <code>Event.type</code> attribute to 
     * <code>eventTypeArg</code>, and <code>Event.namespaceURI</code> to 
     * <code>null</code>. To initialize an event with a namespace URI, use 
     * the <code>Event.initEventNS(namespaceURIArg, eventTypeArg, ...)</code>
     *  method. 
     * @param eventTypeArg  Specifies <code>Event.type</code>. 
     * @param canBubbleArg  Specifies <code>Event.bubbles</code>. This 
     *   parameter overrides the intrinsic bubbling behavior of the event. 
     * @param cancelableArg  Specifies <code>Event.cancelable</code>. This 
     *   parameter overrides the intrinsic cancelable behavior of the event. 
     */
    public void initEvent(String eventTypeArg, 
                          boolean canBubbleArg, 
                          boolean cancelableArg);

    /**
     *  The namespace URI associated with this event at creation time, or 
     * <code>null</code> if it is unspecified. 
     * <br> For events initialized with a DOM Level 2 Events method, such as 
     * <code>Event.initEvent()</code>, this is always <code>null</code>. 
     * @since DOM Level 3
     */
    public String getNamespaceURI();

    /**
     *  This method will always return <code>false</code>, unless the event 
     * implements the <code>CustomEvent</code> interface. 
     * @return  <code>false</code>, unless the event object implements the 
     *   <code>CustomEvent</code> interface. 
     * @since DOM Level 3
     */
    public boolean isCustom();

    /**
     *  This method is used to prevent event listeners of the same group to be 
     * triggered and, unlike <code>stopPropagation</code> its effect is 
     * immediate (see ). Once it has been called, further calls to that 
     * method have no additional effect. 
     * <p ><b>Note:</b>  This method does not prevent the default action from 
     * being invoked; use <code>Event.preventDefault()</code> for that 
     * effect. 
     * @since DOM Level 3
     */
    public void stopImmediatePropagation();

    /**
     *  This method will return <code>true</code> if the method 
     * <code>Event.preventDefault()</code> has been called for this event, 
     * <code>false</code> otherwise. 
     * @return  <code>true</code> if <code>Event.preventDefault()</code> has 
     *   been called for this event. 
     * @since DOM Level 3
     */
    public boolean isDefaultPrevented();

    /**
     *  The <code>initEventNS</code> method is used to initialize the value of 
     * an <code>Event</code> object and has the same behavior as 
     * <code>Event.initEvent()</code>. 
     * @param namespaceURIArg  Specifies <code>Event.namespaceuRI</code>, the 
     *   namespace URI associated with this event, or <code>null</code> if 
     *   no namespace. 
     * @param eventTypeArg  Specifies <code>Event.type</code>, the local name 
     *   of the event type.
     * @param canBubbleArg  Refer to the <code>Event.initEvent()</code> 
     *   method for a description of this parameter.
     * @param cancelableArg  Refer to the <code>Event.initEvent()</code> 
     *   method for a description of this parameter. 
     * @since DOM Level 3
     */
    public void initEventNS(String namespaceURIArg, 
                            String eventTypeArg, 
                            boolean canBubbleArg, 
                            boolean cancelableArg);

}
