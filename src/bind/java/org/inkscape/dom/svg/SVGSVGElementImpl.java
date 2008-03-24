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

import org.w3c.dom.NodeList;
import org.w3c.dom.Element;

import org.w3c.dom.DOMException;

import org.w3c.dom.svg.*;

import org.w3c.dom.css.CSSStyleDeclaration;
import org.w3c.dom.css.CSSValue;
import org.w3c.dom.css.RGBColor;

import org.w3c.dom.views.DocumentView;

import org.w3c.dom.events.Event;
import org.w3c.dom.events.EventTarget;
import org.w3c.dom.events.EventException;
import org.w3c.dom.events.EventListener;
import org.w3c.dom.events.DocumentEvent;

import org.w3c.dom.stylesheets.DocumentStyle;
import org.w3c.dom.stylesheets.StyleSheetList;



public class SVGSVGElementImpl
       extends
               SVGElementImpl
               //SVGTests,
               //SVGLangSpace,
               //SVGExternalResourcesRequired,
               //SVGStylable,
               //SVGLocatable,
               //SVGFitToViewBox,
               //SVGZoomAndPan,
               //EventTarget,
               //DocumentEvent,
               //ViewCSS,
               //DocumentCSS
       implements org.w3c.dom.svg.SVGSVGElement
{
public SVGSVGElementImpl()
{
    imbue(_SVGTests = new SVGTestsImpl());
    imbue(_SVGLangSpace = new SVGLangSpaceImpl());
    imbue(_SVGExternalResourcesRequired = new SVGExternalResourcesRequiredImpl());
    imbue(_SVGStylable = new SVGStylableImpl());
    imbue(_SVGLocatable = new SVGLocatableImpl());
    imbue(_SVGFitToViewBox = new SVGFitToViewBoxImpl());
    imbue(_SVGZoomAndPan = new SVGZoomAndPanImpl());
    imbue(_EventTarget = new org.inkscape.dom.events.EventTargetImpl());
    imbue(_DocumentEvent = new org.inkscape.dom.events.DocumentEventImpl());
    imbue(_ViewCSS = new org.inkscape.dom.css.ViewCSSImpl());
    imbue(_DocumentCSS = new org.inkscape.dom.css.DocumentCSSImpl());
}


//from SVGURIReference
private SVGURIReferenceImpl _SVGURIReference;
public SVGAnimatedString getHref()
    { return _SVGURIReference.getHref(); }
//end SVGURIReference

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

//from SVGLocatable
private SVGLocatableImpl _SVGLocatable;
public SVGElement getNearestViewportElement()
    { return _SVGLocatable.getNearestViewportElement(); }
public SVGElement getFarthestViewportElement()
    { return _SVGLocatable.getFarthestViewportElement(); }
public SVGRect getBBox()
    { return _SVGLocatable.getBBox(); }
public SVGMatrix getCTM()
    { return _SVGLocatable.getCTM(); }
public SVGMatrix getScreenCTM()
    { return _SVGLocatable.getScreenCTM(); }
public SVGMatrix getTransformToElement (SVGElement element)
                  throws SVGException
    { return _SVGLocatable.getTransformToElement(element); }
//end SVGLocatable

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

//from SVGFitToViewBox
SVGFitToViewBoxImpl _SVGFitToViewBox;
public SVGAnimatedRect getViewBox()
    { return _SVGFitToViewBox.getViewBox(); }
public SVGAnimatedPreserveAspectRatio getPreserveAspectRatio()
    { return _SVGFitToViewBox.getPreserveAspectRatio(); }
//end SVGFitToViewBox

//from SVGZoomAndPan
SVGZoomAndPanImpl _SVGZoomAndPan;
public short getZoomAndPan()
    { return _SVGZoomAndPan.getZoomAndPan(); }
public void setZoomAndPan(short zoomAndPan) throws DOMException
    { _SVGZoomAndPan.setZoomAndPan(zoomAndPan); }
//end SVGZoomAndPan


//from DocumentEvent
org.inkscape.dom.events.DocumentEventImpl _DocumentEvent;
public Event createEvent(String eventType) throws DOMException
    { return _DocumentEvent.createEvent(eventType); }
public boolean canDispatch(String namespaceURI, String type)
    { return _DocumentEvent.canDispatch(namespaceURI, type); }
//end DocumentEvent

//from ViewCSS
org.inkscape.dom.css.ViewCSSImpl _ViewCSS;
public CSSStyleDeclaration getComputedStyle(Element elt, String pseudoElt)
    { return _ViewCSS.getComputedStyle(elt, pseudoElt); }
//end ViewCSS

//from AbstractView (from ViewCSS)
public DocumentView getDocument()
    { return _ViewCSS.getDocument(); }
//end AbstractView

//from DocumentCSS
org.inkscape.dom.css.DocumentCSSImpl _DocumentCSS;
public CSSStyleDeclaration getOverrideStyle(Element elt, String pseudoElt)
    { return _DocumentCSS.getOverrideStyle(elt, pseudoElt); }
//end DocumentCSS

//from DocumentStyle (from DocumentCSS)
public StyleSheetList getStyleSheets()
    { return _DocumentCSS.getStyleSheets(); }
//end DocumentStyle


public native SVGAnimatedLength getX( );
public native SVGAnimatedLength getY( );
public native SVGAnimatedLength getWidth( );
public native SVGAnimatedLength getHeight( );
public native String getContentScriptType( );
public native void setContentScriptType( String contentScriptType )
                       throws DOMException;
public native String getContentStyleType( );
public native void setContentStyleType( String contentStyleType )
                       throws DOMException;
public native SVGRect getViewport( );
public native float getPixelUnitToMillimeterX( );
public native float getPixelUnitToMillimeterY( );
public native float getScreenPixelToMillimeterX( );
public native float getScreenPixelToMillimeterY( );
public native boolean getUseCurrentView( );
public native void setUseCurrentView( boolean useCurrentView )
                       throws DOMException;
public native SVGViewSpec getCurrentView( );
public native float getCurrentScale( );
public native void setCurrentScale( float currentScale )
                       throws DOMException;
public native SVGPoint getCurrentTranslate( );

public native int suspendRedraw ( int max_wait_milliseconds );
public native void unsuspendRedraw ( int suspend_handle_id )
                  throws DOMException;
public native void unsuspendRedrawAll (  );
public native void forceRedraw (  );
public native void pauseAnimations (  );
public native void unpauseAnimations (  );
public native boolean animationsPaused (  );
public native float getCurrentTime (  );
public native void setCurrentTime ( float seconds );
public native NodeList getIntersectionList ( SVGRect rect, SVGElement referenceElement );
public native NodeList getEnclosureList ( SVGRect rect, SVGElement referenceElement );
public native boolean checkIntersection ( SVGElement element, SVGRect rect );
public native boolean checkEnclosure ( SVGElement element, SVGRect rect );
public native void deselectAll (  );
public native SVGNumber createSVGNumber (  );
public native SVGLength createSVGLength (  );
public native SVGAngle createSVGAngle (  );
public native SVGPoint createSVGPoint (  );
public native SVGMatrix createSVGMatrix (  );
public native SVGRect createSVGRect (  );
public native SVGTransform createSVGTransform (  );
public native SVGTransform createSVGTransformFromMatrix ( SVGMatrix matrix );
public native Element getElementById ( String elementId );
}
