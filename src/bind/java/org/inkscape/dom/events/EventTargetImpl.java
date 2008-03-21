/**
 * This is a simple mechanism to bind Inkscape to Java, and thence
 * to all of the nice things that can be layered upon that.
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (c) 2007-2008 Inkscape.org
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
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
 *  Note that the Events files are implementations of the Java
 *  interface package found here:
 *      http://www.w3.org/TR/2003/NOTE-DOM-Level-3-Events-20031107/java-binding.html
 */

package org.inkscape.dom.events;

import org.w3c.dom.events.EventException;
import org.w3c.dom.events.Event;
import org.w3c.dom.events.EventListener;



public class EventTargetImpl
       implements org.w3c.dom.events.EventTarget
{

public native void addEventListener(String type,
                             EventListener listener,
                             boolean useCapture);

public native void removeEventListener(String type,
                                EventListener listener,
                                boolean useCapture);

public native boolean dispatchEvent(Event evt)
                             throws EventException;

public native void addEventListenerNS(String namespaceURI,
                               String type,
                               EventListener listener,
                               boolean useCapture,
                               Object evtGroup);

public native void removeEventListenerNS(String namespaceURI,
                                  String type,
                                  EventListener listener,
                                  boolean useCapture);

public native boolean willTriggerNS(String namespaceURI,
                             String type);

public native boolean hasEventListenerNS(String namespaceURI,
                                  String type);

}
