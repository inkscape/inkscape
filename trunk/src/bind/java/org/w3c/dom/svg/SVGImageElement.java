
package org.w3c.dom.svg;

import org.w3c.dom.events.EventTarget;

public interface SVGImageElement extends 
               SVGElement,
               SVGURIReference,
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
  public SVGAnimatedPreserveAspectRatio getPreserveAspectRatio( );
}
