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

//from SVGTransformable
public native SVGAnimatedTransformList getTransform( );
//end SVGTransformable

//from SVGLocatable (from SVGTransformable)
public native SVGElement getNearestViewportElement( );
public native SVGElement getFarthestViewportElement( );

public native SVGRect   getBBox (  );
public native SVGMatrix getCTM (  );
public native SVGMatrix getScreenCTM (  );
public native SVGMatrix getTransformToElement ( SVGElement element )
                  throws SVGException;
//end SVGLocatable

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

//from SVGAnimatedPathData
public native SVGPathSegList getPathSegList( );
public native SVGPathSegList getNormalizedPathSegList( );
public native SVGPathSegList getAnimatedPathSegList( );
public native SVGPathSegList getAnimatedNormalizedPathSegList( );
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
