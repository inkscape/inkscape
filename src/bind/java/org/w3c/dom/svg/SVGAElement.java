
package org.w3c.dom.svg;

import org.w3c.dom.events.EventTarget;

public interface SVGAElement extends 
               SVGElement,
               SVGURIReference,
               SVGTests,
               SVGLangSpace,
               SVGExternalResourcesRequired,
               SVGStylable,
               SVGTransformable,
               EventTarget {
  public SVGAnimatedString getTarget( );
}
