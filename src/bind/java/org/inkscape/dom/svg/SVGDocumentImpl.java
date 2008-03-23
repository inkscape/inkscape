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
 *  Note that these SVG files are implementations of the Java
 *  interface package found here:
 *      http://www.w3.org/TR/SVG/java.html
 */


package org.inkscape.dom.svg;

import org.w3c.dom.DOMException;
import org.w3c.dom.Document;
import org.w3c.dom.events.Event;
import org.w3c.dom.events.DocumentEvent;

import org.w3c.dom.svg.SVGSVGElement;


public class SVGDocumentImpl
       extends
               org.inkscape.dom.DocumentImpl
               //DocumentEvent
       implements org.w3c.dom.svg.SVGDocument
{
public SVGDocumentImpl()
{
    imbue(_DocumentEvent = new org.inkscape.dom.events.DocumentEventImpl());
}

//from DocumentEvent
org.inkscape.dom.events.DocumentEventImpl _DocumentEvent;
public Event createEvent(String eventType) throws DOMException
    { return _DocumentEvent.createEvent(eventType); }
public boolean canDispatch(String namespaceURI, String type)
    { return _DocumentEvent.canDispatch(namespaceURI, type); }
//end DocumentEvent

public native String getTitle( );
public native String getReferrer( );
public native String getDomain( );
public native String getURL( );
public native SVGSVGElement getRootElement( );

}
