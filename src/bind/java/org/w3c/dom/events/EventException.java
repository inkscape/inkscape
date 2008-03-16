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
 *  Event operations may throw an <code>EventException</code> as specified in 
 * their method descriptions. 
 * <p>See also the <a href='http://www.w3.org/TR/2003/NOTE-DOM-Level-3-Events-20031107'>Document Object Model (DOM) Level 3 Events Specification</a>.
 * @since DOM Level 2
 */
public class EventException extends RuntimeException {
    public EventException(short code, String message) {
       super(message);
       this.code = code;
    }
    public short   code;
    // EventExceptionCode
    /**
     *  If the <code>Event.type</code> was not specified by initializing the 
     * event before the method was called. Specification of the 
     * <code>Event.type</code> as <code>null</code> or an empty string will 
     * also trigger this exception. 
     */
    public static final short UNSPECIFIED_EVENT_TYPE_ERR = 0;
    /**
     *  If the <code>Event</code> object is already dispatched in the tree. 
     * @since DOM Level 3
     */
    public static final short DISPATCH_REQUEST_ERR      = 1;

}
