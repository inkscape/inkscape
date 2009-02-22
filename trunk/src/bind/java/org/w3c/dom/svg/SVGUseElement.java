
package org.w3c.dom.svg;

import org.w3c.dom.events.EventTarget;

public interface SVGUseElement extends 
               SVGElement,
               SVGURIReference,
               SVGTests,
               SVGLangSpace,
               SVGExternalResourcesRequired,
               SVGStylable,
               SVGTransformable,
               EventTarget {
  public SVGAnimatedLength   getX( );
  public SVGAnimatedLength   getY( );
  public SVGAnimatedLength   getWidth( );
  public SVGAnimatedLength   getHeight( );
  public SVGElementInstance getInstanceRoot( );
  public SVGElementInstance getAnimatedInstanceRoot( );
}
