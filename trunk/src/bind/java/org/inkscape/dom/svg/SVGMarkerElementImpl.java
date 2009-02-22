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



public class SVGMarkerElementImpl
       extends
               SVGElementImpl
               //SVGLangSpace,
               //SVGExternalResourcesRequired,
               //SVGStylable,
               //SVGFitToViewBox
       implements org.w3c.dom.svg.SVGMarkerElement
{

public SVGMarkerElementImpl()
{
    imbue(_SVGLangSpace = new SVGLangSpaceImpl());
    imbue(_SVGExternalResourcesRequired = new SVGExternalResourcesRequiredImpl());
    imbue(_SVGStylable = new SVGStylableImpl());
    imbue(_SVGFitToViewBox = new SVGFitToViewBoxImpl());
}

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

//from SVGFitToViewBox
SVGFitToViewBoxImpl _SVGFitToViewBox;
public SVGAnimatedRect getViewBox()
    { return _SVGFitToViewBox.getViewBox(); }
public SVGAnimatedPreserveAspectRatio getPreserveAspectRatio()
    { return _SVGFitToViewBox.getPreserveAspectRatio(); }
//end SVGFitToViewBox


public native SVGAnimatedLength getRefX( );
public native SVGAnimatedLength getRefY( );
public native SVGAnimatedEnumeration getMarkerUnits( );
public native SVGAnimatedLength getMarkerWidth( );
public native SVGAnimatedLength getMarkerHeight( );
public native SVGAnimatedEnumeration getOrientType( );
public native SVGAnimatedAngle getOrientAngle( );
public native void setOrientToAuto (  );
public native void setOrientToAngle ( SVGAngle angle );
}
