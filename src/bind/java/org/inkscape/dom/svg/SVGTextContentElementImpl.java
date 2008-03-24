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

import org.w3c.dom.svg.*;

import org.w3c.dom.events.EventTarget;
import org.w3c.dom.css.CSSStyleDeclaration;
import org.w3c.dom.css.CSSValue;

import org.w3c.dom.events.EventException;
import org.w3c.dom.events.Event;
import org.w3c.dom.events.EventListener;




public class SVGTextContentElementImpl
       extends
               SVGElementImpl
               //SVGTests,
               //SVGLangSpace,
               //SVGExternalResourcesRequired,
               //SVGStylable,
               //EventTarget
       implements org.w3c.dom.svg.SVGTextContentElement
{

public SVGTextContentElementImpl()
{
    imbue(_SVGTests = new SVGTestsImpl());
    imbue(_SVGLangSpace = new SVGLangSpaceImpl());
    imbue(_SVGExternalResourcesRequired = new SVGExternalResourcesRequiredImpl());
    imbue(_SVGStylable = new SVGStylableImpl());
    imbue(_EventTarget = new org.inkscape.dom.events.EventTargetImpl());
}


//from SVGTests
private SVGTestsImpl _SVGTests;
public SVGStringList getRequiredFeatures()
   { return _SVGTests.getRequiredFeatures(); }
public SVGStringList getRequiredExtensions()
   { return _SVGTests.getRequiredExtensions(); }
public SVGStringList getSystemLanguage()
   { return _SVGTests.getSystemLanguage(); }
public boolean hasExtension (String extension)
   { return _SVGTests.hasExtension(extension); }
//end SVGTests

//from SVGLangSpace
private SVGLangSpaceImpl _SVGLangSpace;
public String getXMLlang()
    { return _SVGLangSpace.getXMLlang(); }
public void setXMLlang(String xmllang)
                       throws DOMException
    { _SVGLangSpace.setXMLlang(xmllang); }
public String getXMLspace()
    { return _SVGLangSpace.getXMLspace(); }
public void setXMLspace(String xmlspace)
                       throws DOMException
    { _SVGLangSpace.setXMLspace(xmlspace); }
//end SVGLangSpace

//from SVGExternalResourcesRequired
private SVGExternalResourcesRequiredImpl _SVGExternalResourcesRequired;
public SVGAnimatedBoolean getExternalResourcesRequired()
    { return _SVGExternalResourcesRequired.getExternalResourcesRequired(); }
//end SVGExternalResourcesRequired

//from SVGStylable
private SVGStylableImpl _SVGStylable;
public SVGAnimatedString getClassName()
    { return _SVGStylable.getClassName(); }
public CSSStyleDeclaration getStyle()
    { return _SVGStylable.getStyle(); }
public CSSValue getPresentationAttribute(String name)
    { return _SVGStylable.getPresentationAttribute(name); }
//end SVGStylable

//from EventTarget
private org.inkscape.dom.events.EventTargetImpl _EventTarget;
public void addEventListener(String type,
                             EventListener listener,
                             boolean useCapture)
    { _EventTarget.addEventListener(type, listener, useCapture); }
public void removeEventListener(String type,
                                EventListener listener,
                                boolean useCapture)
    { _EventTarget.removeEventListener(type, listener, useCapture); }
public boolean dispatchEvent(Event evt)
                             throws EventException
    { return _EventTarget.dispatchEvent(evt); }
public void addEventListenerNS(String namespaceURI,
                               String type,
                               EventListener listener,
                               boolean useCapture,
                               Object evtGroup)
    { _EventTarget.addEventListenerNS(namespaceURI, type, listener, useCapture, evtGroup); }
public void removeEventListenerNS(String namespaceURI,
                                  String type,
                                  EventListener listener,
                                  boolean useCapture)
    { _EventTarget.removeEventListenerNS(namespaceURI, type, listener, useCapture); }
public boolean willTriggerNS(String namespaceURI,
                             String type)
    { return _EventTarget.willTriggerNS(namespaceURI, type); }
public boolean hasEventListenerNS(String namespaceURI,
                                  String type)
    { return _EventTarget.hasEventListenerNS(namespaceURI, type); }
//end EventTarget



public native SVGAnimatedLength      getTextLength( );
public native SVGAnimatedEnumeration getLengthAdjust( );

public native int      getNumberOfChars (  );
public native float    getComputedTextLength (  );
public native float    getSubStringLength ( int charnum, int nchars )
                  throws DOMException;
public native SVGPoint getStartPositionOfChar ( int charnum )
                  throws DOMException;
public native SVGPoint getEndPositionOfChar ( int charnum )
                  throws DOMException;
public native SVGRect  getExtentOfChar ( int charnum )
                  throws DOMException;
public native float    getRotationOfChar ( int charnum )
                  throws DOMException;
public native int      getCharNumAtPosition ( SVGPoint point );
public native void     selectSubString ( int charnum, int nchars )
                  throws DOMException;
}
