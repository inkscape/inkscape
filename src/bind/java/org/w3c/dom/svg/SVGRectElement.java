
package org.w3c.dom.svg;

import org.w3c.dom.events.EventTarget;

public interface SVGRectElement extends 
               SVGElement,
               SVGTests,
               SVGLangSpace,
               SVGExternalResourcesRequired,
               SVGStylable,
               SVGTransformable,
               EventTarget {
  public SVGAnimatedLength getX( );
  public SVGAnimatedLength getY( );
  public SVGAnimatedLength getWidth( );
  public SVGAnimatedLength getHeight( );
  public SVGAnimatedLength getRx( );
  public SVGAnimatedLength getRy( );
}
