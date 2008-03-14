
package org.w3c.dom.svg;

public interface SVGViewElement extends 
               SVGElement,
               SVGExternalResourcesRequired,
               SVGFitToViewBox,
               SVGZoomAndPan {
  public SVGStringList getViewTarget( );
}
