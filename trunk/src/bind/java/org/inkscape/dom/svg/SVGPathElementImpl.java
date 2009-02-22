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

import org.w3c.dom.css.CSSStyleDeclaration;
import org.w3c.dom.css.CSSValue;

import org.w3c.dom.events.Event;
import org.w3c.dom.events.EventTarget;
import org.w3c.dom.events.EventException;
import org.w3c.dom.events.EventListener;


public class SVGPathElementImpl
       extends
               SVGElementImpl
               //SVGTests,
               //SVGLangSpace,
               //SVGExternalResourcesRequired,
               //SVGStylable,
               //SVGTransformable,
               //EventTarget,
               //SVGAnimatedPathData
       implements org.w3c.dom.svg.SVGPathElement
{

public SVGPathElementImpl()
{
    imbue(_SVGTests = new SVGTestsImpl());
    imbue(_SVGLangSpace = new SVGLangSpaceImpl());
    imbue(_SVGExternalResourcesRequired = new SVGExternalResourcesRequiredImpl());
    imbue(_SVGStylable = new SVGStylableImpl());
    imbue(_SVGTransformable = new SVGTransformableImpl());
    imbue(_EventTarget = new org.inkscape.dom.events.EventTargetImpl());
    imbue(_SVGAnimatedPathData = new SVGAnimatedPathDataImpl());
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

//from SVGTransformable
private SVGTransformableImpl _SVGTransformable;
public SVGAnimatedTransformList getTransform()
    { return _SVGTransformable.getTransform(); }
//end SVGTransformable

//from SVGLocatable (from SVGTransformable)
public SVGElement getNearestViewportElement()
    { return _SVGTransformable.getNearestViewportElement(); }
public SVGElement getFarthestViewportElement()
    { return _SVGTransformable.getFarthestViewportElement(); }
public SVGRect getBBox()
    { return _SVGTransformable.getBBox(); }
public SVGMatrix getCTM()
    { return _SVGTransformable.getCTM(); }
public SVGMatrix getScreenCTM()
    { return _SVGTransformable.getScreenCTM(); }
public SVGMatrix getTransformToElement (SVGElement element)
                  throws SVGException
    { return _SVGTransformable.getTransformToElement(element); }
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

//from SVGAnimatedPathData
SVGAnimatedPathDataImpl _SVGAnimatedPathData;
public SVGPathSegList getPathSegList()
     { return _SVGAnimatedPathData.getPathSegList(); }
public SVGPathSegList getNormalizedPathSegList()
     { return _SVGAnimatedPathData.getNormalizedPathSegList(); }
public SVGPathSegList getAnimatedPathSegList()
     { return _SVGAnimatedPathData.getAnimatedPathSegList(); }
public SVGPathSegList getAnimatedNormalizedPathSegList()
     { return _SVGAnimatedPathData.getAnimatedNormalizedPathSegList(); }
//end SVGAnimatedPathData

public native SVGAnimatedNumber getPathLength( );
public native float getTotalLength (  );
public native SVGPoint getPointAtLength ( float distance );
public native int getPathSegAtLength ( float distance );

//CREATEs
public native SVGPathSegClosePath    createSVGPathSegClosePath (  );
public native SVGPathSegMovetoAbs    createSVGPathSegMovetoAbs ( float x, float y );
public native SVGPathSegMovetoRel    createSVGPathSegMovetoRel ( float x, float y );
public native SVGPathSegLinetoAbs    createSVGPathSegLinetoAbs ( float x, float y );
public native SVGPathSegLinetoRel    createSVGPathSegLinetoRel ( float x, float y );

public native SVGPathSegCurvetoCubicAbs    createSVGPathSegCurvetoCubicAbs ( float x, float y, float x1, float y1, float x2, float y2 );
public native SVGPathSegCurvetoCubicRel    createSVGPathSegCurvetoCubicRel ( float x, float y, float x1, float y1, float x2, float y2 );

public native SVGPathSegCurvetoQuadraticAbs    createSVGPathSegCurvetoQuadraticAbs ( float x, float y, float x1, float y1 );
public native SVGPathSegCurvetoQuadraticRel    createSVGPathSegCurvetoQuadraticRel ( float x, float y, float x1, float y1 );

public native SVGPathSegArcAbs    createSVGPathSegArcAbs ( float x, float y, float r1, float r2, float angle, boolean largeArcFlag, boolean sweepFlag );
public native SVGPathSegArcRel    createSVGPathSegArcRel ( float x, float y, float r1, float r2, float angle, boolean largeArcFlag, boolean sweepFlag );

public native SVGPathSegLinetoHorizontalAbs    createSVGPathSegLinetoHorizontalAbs ( float x );
public native SVGPathSegLinetoHorizontalRel    createSVGPathSegLinetoHorizontalRel ( float x );

public native SVGPathSegLinetoVerticalAbs    createSVGPathSegLinetoVerticalAbs ( float y );
public native SVGPathSegLinetoVerticalRel    createSVGPathSegLinetoVerticalRel ( float y );

public native SVGPathSegCurvetoCubicSmoothAbs    createSVGPathSegCurvetoCubicSmoothAbs ( float x, float y, float x2, float y2 );
public native SVGPathSegCurvetoCubicSmoothRel    createSVGPathSegCurvetoCubicSmoothRel ( float x, float y, float x2, float y2 );

public native SVGPathSegCurvetoQuadraticSmoothAbs    createSVGPathSegCurvetoQuadraticSmoothAbs ( float x, float y );
public native SVGPathSegCurvetoQuadraticSmoothRel    createSVGPathSegCurvetoQuadraticSmoothRel ( float x, float y );
}
