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
//from SVGTests
public native SVGStringList getRequiredFeatures( );
public native SVGStringList getRequiredExtensions( );
public native SVGStringList getSystemLanguage( );
public native boolean hasExtension ( String extension );
//end SVGTests

//from SVGLangSpace
public native String getXMLlang( );
public native void setXMLlang( String xmllang )
                       throws DOMException;
public native String getXMLspace( );
public native void setXMLspace( String xmlspace )
                       throws DOMException;
//end SVGLangSpace

//from SVGExternalResourcesRequired
public native SVGAnimatedBoolean getExternalResourcesRequired( );
//end SVGExternalResourcesRequired

//from SVGStylable
public native SVGAnimatedString getClassName( );
public native CSSStyleDeclaration getStyle( );
public native CSSValue getPresentationAttribute ( String name );
//end SVGStylable

//from SVGLocatable
public native SVGElement getNearestViewportElement( );
public native SVGElement getFarthestViewportElement( );

public native SVGRect   getBBox (  );
public native SVGMatrix getCTM (  );
public native SVGMatrix getScreenCTM (  );
public native SVGMatrix getTransformToElement ( SVGElement element )
                  throws SVGException;
//end SVGLocatable

//from SVGFitToViewBox
public native SVGAnimatedRect getViewBox( );
public native SVGAnimatedPreserveAspectRatio getPreserveAspectRatio( );
//end SVGFitToViewBox

//from SVGZoomAndPan
public native short getZoomAndPan( );
public native void setZoomAndPan( short zoomAndPan )
                       throws DOMException;
//end SVGZoomAndPan

//from EventTarget
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
//end EventTarget

//from DocumentEvent
public native Event createEvent(String eventType)
                             throws DOMException;
public native boolean canDispatch(String namespaceURI,
                               String type);
//end DocumentEvent

//from ViewCSS
public native CSSStyleDeclaration getComputedStyle(Element elt,
                                                String pseudoElt);
//end ViewCSS

//from AbstractView (from ViewCSS)
public native DocumentView getDocument();
//end AbstractView

//from DocumentCSS
public native CSSStyleDeclaration getOverrideStyle(Element elt,
                                                String pseudoElt);
//end DocumentCSS

//from DocumentStyle (from DocumentCSS)
public native StyleSheetList getStyleSheets();
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
