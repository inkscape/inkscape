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

import org.w3c.dom.views.AbstractView;

import org.w3c.dom.events.EventTarget;



public class MouseEventImpl
       extends UIEventImpl
       implements org.w3c.dom.events.MouseEvent
{

public native int getScreenX();

public native int getScreenY();

public native int getClientX();

public native int getClientY();

public native boolean getCtrlKey();

public native boolean getShiftKey();

public native boolean getAltKey();

public native boolean getMetaKey();

public native short getButton();

public native EventTarget getRelatedTarget();

public native void initMouseEvent(String typeArg,
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

public native boolean getModifierState(String keyIdentifierArg);

public native void initMouseEventNS(String namespaceURI,
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


