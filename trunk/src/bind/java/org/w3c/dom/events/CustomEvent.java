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
 *  The <code>CustomEvent</code> interface gives access to the attributes 
 * <code>Event.currentTarget</code> and <code>Event.eventPhase</code>. It is 
 * intended to be used by the DOM Events implementation to access the 
 * underlying current target and event phase while dispatching a custom 
 * <code>Event</code> in the tree; it is also intended to be implemented, 
 * and <em>not used</em>, by DOM applications. 
 * <p> The methods contained in this interface are not intended to be used by 
 * a DOM application, especially during the dispatch on the 
 * <code>Event</code> object. Changing the current target or the current 
 * phase may result in unpredictable results of the event flow. The DOM 
 * Events implementation should ensure that both methods return the 
 * appropriate current target and phase before invoking each event listener 
 * on the current target to protect DOM applications from malicious event 
 * listeners. 
 * <p ><b>Note:</b>  If this interface is supported by the event object, 
 * <code>Event.isCustom()</code> must return <code>true</code>. 
 * <p>See also the <a href='http://www.w3.org/TR/2003/NOTE-DOM-Level-3-Events-20031107'>Document Object Model (DOM) Level 3 Events Specification</a>.
 * @since DOM Level 3
 */
public interface CustomEvent extends Event {
    /**
     *  The <code>setDispatchState</code> method is used by the DOM Events 
     * implementation to set the values of <code>Event.currentTarget</code> 
     * and <code>Event.eventPhase</code>. It also reset the states of 
     * <code>isPropagationStopped</code> and 
     * <code>isImmediatePropagationStopped</code>. 
     * @param target  Specifies the new value for the 
     *   <code>Event.currentTarget</code> attribute. 
     * @param phase  Specifies the new value for the 
     *   <code>Event.eventPhase</code> attribute. 
     */
    public void setDispatchState(EventTarget target, 
                                 short phase);

    /**
     *  This method will return <code>true</code> if the method 
     * <code>stopPropagation()</code> has been called for this event, 
     * <code>false</code> in any other cases. 
     * @return  <code>true</code> if the event propagation has been stopped 
     *   in the current group. 
     */
    public boolean isPropagationStopped();

    /**
     *  The <code>isImmediatePropagationStopped</code> method is used by the 
     * DOM Events implementation to know if the method 
     * <code>stopImmediatePropagation()</code> has been called for this 
     * event. It returns <code>true</code> if the method has been called, 
     * <code>false</code> otherwise. 
     * @return  <code>true</code> if the event propagation has been stopped 
     *   immediately in the current group. 
     */
    public boolean isImmediatePropagationStopped();

}
