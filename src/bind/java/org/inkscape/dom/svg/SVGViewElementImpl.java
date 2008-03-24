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


public class SVGViewElementImpl
       extends
               SVGElementImpl
               //SVGExternalResourcesRequired,
               //SVGFitToViewBox,
               //SVGZoomAndPan
       implements org.w3c.dom.svg.SVGViewElement
{
public SVGViewElementImpl()
{
    imbue(_SVGExternalResourcesRequired = new SVGExternalResourcesRequiredImpl());
    imbue(_SVGFitToViewBox = new SVGFitToViewBoxImpl());
    imbue(_SVGZoomAndPan = new SVGZoomAndPanImpl());
}


//from SVGExternalResourcesRequired
private SVGExternalResourcesRequiredImpl _SVGExternalResourcesRequired;
public SVGAnimatedBoolean getExternalResourcesRequired()
    { return _SVGExternalResourcesRequired.getExternalResourcesRequired(); }
//end SVGExternalResourcesRequired


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


public native SVGStringList getViewTarget( );

}
