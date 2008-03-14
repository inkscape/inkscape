
package org.w3c.dom.svg;

import org.w3c.dom.events.EventTarget;

public interface SVGCircleElement extends 
               SVGElement,
               SVGTests,
               SVGLangSpace,
               SVGExternalResourcesRequired,
               SVGStylable,
               SVGTransformable,
               EventTarget {
  public SVGAnimatedLength getCx( );
  public SVGAnimatedLength getCy( );
  public SVGAnimatedLength getR( );
}
