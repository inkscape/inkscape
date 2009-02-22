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

import org.w3c.dom.views.AbstractView;

/**
 * The <code>MouseEvent</code> interface provides specific contextual 
 * information associated with Mouse events.
 * <p> In the case of nested elements mouse events are always targeted at the 
 * most deeply nested element. Ancestors of the targeted element may use 
 * bubbling to obtain notification of mouse events which occur within theirs 
 * descendent elements. 
 * <p> To create an instance of the <code>MouseEvent</code> interface, use the 
 * <code>DocumentEvent.createEvent("MouseEvent")</code> method call. 
 * <p ><b>Note:</b>  When initializing <code>MouseEvent</code> objects using 
 * <code>initMouseEvent</code> or <code>initMouseEventNS</code>, 
 * implementations should use the client coordinates <code>clientX</code> 
 * and <code>clientY</code> for calculation of other coordinates (such as 
 * target coordinates exposed by DOM Level 0 implementations). 
 * <p>See also the <a href='http://www.w3.org/TR/2003/NOTE-DOM-Level-3-Events-20031107'>Document Object Model (DOM) Level 3 Events Specification</a>.
 * @since DOM Level 2
 */
public interface MouseEvent extends UIEvent {
    /**
     * The horizontal coordinate at which the event occurred relative to the 
     * origin of the screen coordinate system.
     */
    public int getScreenX();

    /**
     * The vertical coordinate at which the event occurred relative to the 
     * origin of the screen coordinate system.
     */
    public int getScreenY();

    /**
     * The horizontal coordinate at which the event occurred relative to the 
     * DOM implementation's client area.
     */
    public int getClientX();

    /**
     * The vertical coordinate at which the event occurred relative to the DOM 
     * implementation's client area.
     */
    public int getClientY();

    /**
     *  <code>true</code> if the control (Ctrl) key modifier is activated. 
     */
    public boolean getCtrlKey();

    /**
     *  <code>true</code> if the shift (Shift) key modifier is activated. 
     */
    public boolean getShiftKey();

    /**
     *  <code>true</code> if the alt (alternative) key modifier is activated. 
     * <p ><b>Note:</b>  The Option key modifier on Macintosh systems must be 
     * represented using this key modifier. 
     */
    public boolean getAltKey();

    /**
     *  <code>true</code> if the meta (Meta) key modifier is activated. 
     * <p ><b>Note:</b>  The Command key modifier on Macintosh system must be 
     * represented using this meta key. 
     */
    public boolean getMetaKey();

    /**
     *  During mouse events caused by the depression or release of a mouse 
     * button, <code>button</code> is used to indicate which mouse button 
     * changed state. <code>0</code> indicates the normal button of the 
     * mouse (in general on the left or the one button on Macintosh mice, 
     * used to activate a button or select text). <code>2</code> indicates 
     * the contextual property (in general on the right, used to display a 
     * context menu) button of the mouse if present. <code>1</code> 
     * indicates the extra (in general in the middle and often combined with 
     * the mouse wheel) button. Some mice may provide or simulate more 
     * buttons, and values higher than <code>2</code> can be used to 
     * represent such buttons. 
     */
    public short getButton();

    /**
     *  Used to identify a secondary <code>EventTarget</code> related to a UI 
     * event. Currently this attribute is used with the mouseover event to 
     * indicate the <code>EventTarget</code> which the pointing device 
     * exited and with the mouseout event to indicate the 
     * <code>EventTarget</code> which the pointing device entered.
     */
    public EventTarget getRelatedTarget();

    /**
     *  The <code>initMouseEvent</code> method is used to initialize the value 
     * of a <code>MouseEvent</code> object and has the same behavior as 
     * <code>UIEvent.initUIEvent()</code>. 
     * @param typeArg  Refer to the <code>UIEvent.initUIEvent()</code> method 
     *   for a description of this parameter. 
     * @param canBubbleArg  Refer to the <code>UIEvent.initUIEvent()</code> 
     *   method for a description of this parameter. 
     * @param cancelableArg  Refer to the <code>UIEvent.initUIEvent()</code> 
     *   method for a description of this parameter. 
     * @param viewArg  Refer to the <code>UIEvent.initUIEvent()</code> method 
     *   for a description of this parameter. 
     * @param detailArg  Refer to the <code>UIEvent.initUIEvent()</code> 
     *   method for a description of this parameter. 
     * @param screenXArg  Specifies <code>MouseEvent.screenX</code>. 
     * @param screenYArg  Specifies <code>MouseEvent.screenY</code>. 
     * @param clientXArg  Specifies <code>MouseEvent.clientX</code>. 
     * @param clientYArg  Specifies <code>MouseEvent.clientY</code>. 
     * @param ctrlKeyArg  Specifies <code>MouseEvent.ctrlKey</code>. 
     * @param altKeyArg  Specifies <code>MouseEvent.altKey</code>. 
     * @param shiftKeyArg  Specifies <code>MouseEvent.shiftKey</code>. 
     * @param metaKeyArg  Specifies <code>MouseEvent.metaKey</code>. 
     * @param buttonArg  Specifies <code>MouseEvent.button</code>. 
     * @param relatedTargetArg  Specifies 
     *   <code>MouseEvent.relatedTarget</code>. 
     */
    public void initMouseEvent(String typeArg, 
                               boolean canBubbleArg, 
                               boolean cancelableArg, 
                               AbstractView viewArg, 
                               int detailArg, 
                               int screenXArg, 
                               int screenYArg, 
                               int clientXArg, 
                               int clientYArg, 
                               boolean ctrlKeyArg, 
                               boolean altKeyArg, 
                               boolean shiftKeyArg, 
                               boolean metaKeyArg, 
                               short buttonArg, 
                               EventTarget relatedTargetArg);

    /**
     *  This methods queries the state of a modifier using a key identifier. 
     * See also . 
     * @param keyIdentifierArg  A modifier key identifier, as defined by the 
     *   <code>KeyboardEvent.keyIdentifier</code> attribute. Common modifier 
     *   keys are <code>"Alt"</code>, <code>"AltGraph"</code>, 
     *   <code>"CapsLock"</code>, <code>"Control"</code>, <code>"Meta"</code>
     *   , <code>"NumLock"</code>, <code>"Scroll"</code>, or 
     *   <code>"Shift"</code>. 
     * <p ><b>Note:</b>    If an application wishes to distinguish between 
     *   right and left modifiers, this information could be deduced using 
     *   keyboard events and <code>KeyboardEvent.keyLocation</code>. 
     * @return  <code>true</code> if it is modifier key and the modifier is 
     *   activated, <code>false</code> otherwise. 
     * @since DOM Level 3
     */
    public boolean getModifierState(String keyIdentifierArg);

    /**
     *  The <code>initMouseEventNS</code> method is used to initialize the 
     * value of a <code>MouseEvent</code> object and has the same behavior 
     * as <code>UIEvent.initUIEventNS()</code>. 
     * @param namespaceURI  Refer to the <code>UIEvent.initUIEventNS()</code> 
     *   method for a description of this parameter. 
     * @param typeArg  Refer to the <code>UIEvent.initUIEventNS()</code> 
     *   method for a description of this parameter. 
     * @param canBubbleArg  Refer to the <code>UIEvent.initUIEventNS()</code> 
     *   method for a description of this parameter. 
     * @param cancelableArg  Refer to the <code>UIEvent.initUIEventNS()</code>
     *    method for a description of this parameter. 
     * @param viewArg  Refer to the <code>UIEvent.initUIEventNS()</code> 
     *   method for a description of this parameter. 
     * @param detailArg  Refer to the <code>UIEvent.initUIEventNS()</code> 
     *   method for a description of this parameter. 
     * @param screenXArg  Refer to the 
     *   <code>MouseEvent.initMouseEvent()</code> method for a description 
     *   of this parameter. 
     * @param screenYArg  Refer to the 
     *   <code>MouseEvent.initMouseEvent()</code> method for a description 
     *   of this parameter. 
     * @param clientXArg  Refer to the 
     *   <code>MouseEvent.initMouseEvent()</code> method for a description 
     *   of this parameter. 
     * @param clientYArg  Refer to the 
     *   <code>MouseEvent.initMouseEvent()</code> method for a description 
     *   of this parameter. 
     * @param buttonArg  Refer to the <code>MouseEvent.initMouseEvent()</code>
     *    method for a description of this parameter. 
     * @param relatedTargetArg  Refer to the 
     *   <code>MouseEvent.initMouseEvent()</code> method for a description 
     *   of this parameter. 
     * @param modifiersList  A <a href='http://www.w3.org/TR/2000/REC-xml-20001006#NT-S'>white space</a> separated list of modifier key identifiers to be activated on this 
     *   object. As an example, <code>"Control Alt"</code> will activated 
     *   the control and alt modifiers. 
     * @since DOM Level 3
     */
    public void initMouseEventNS(String namespaceURI, 
                                 String typeArg, 
                                 boolean canBubbleArg, 
                                 boolean cancelableArg, 
                                 AbstractView viewArg, 
                                 int detailArg, 
                                 int screenXArg, 
                                 int screenYArg, 
                                 int clientXArg, 
                                 int clientYArg, 
                                 short buttonArg, 
                                 EventTarget relatedTargetArg, 
                                 String modifiersList);

}
