
package org.w3c.dom.svg;

import org.w3c.dom.DOMException;
import org.w3c.dom.events.EventTarget;
import org.w3c.dom.smil.ElementTimeControl;

public interface SVGAnimationElement extends 
               SVGElement,
               SVGTests,
               SVGExternalResourcesRequired,
               ElementTimeControl,
               EventTarget {
  public SVGElement getTargetElement( );

  public float getStartTime (  );
  public float getCurrentTime (  );
  public float getSimpleDuration (  )
                  throws DOMException;
}
