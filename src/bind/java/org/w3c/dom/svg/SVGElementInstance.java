
package org.w3c.dom.svg;

import org.w3c.dom.events.EventTarget;

public interface SVGElementInstance extends 
               EventTarget {
  public SVGElement getCorrespondingElement( );
  public SVGUseElement getCorrespondingUseElement( );
  public SVGElementInstance getParentNode( );
  public SVGElementInstanceList getChildNodes( );
  public SVGElementInstance getFirstChild( );
  public SVGElementInstance getLastChild( );
  public SVGElementInstance getPreviousSibling( );
  public SVGElementInstance getNextSibling( );
}
