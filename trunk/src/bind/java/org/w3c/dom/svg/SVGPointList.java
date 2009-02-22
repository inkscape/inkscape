
package org.w3c.dom.svg;

import org.w3c.dom.DOMException;

public interface SVGPointList {
  public int getNumberOfItems( );

  public void   clear (  )
                  throws DOMException;
  public SVGPoint initialize ( SVGPoint newItem )
                  throws DOMException, SVGException;
  public SVGPoint getItem ( int index )
                  throws DOMException;
  public SVGPoint insertItemBefore ( SVGPoint newItem, int index )
                  throws DOMException, SVGException;
  public SVGPoint replaceItem ( SVGPoint newItem, int index )
                  throws DOMException, SVGException;
  public SVGPoint removeItem ( int index )
                  throws DOMException;
  public SVGPoint appendItem ( SVGPoint newItem )
                  throws DOMException, SVGException;
}
