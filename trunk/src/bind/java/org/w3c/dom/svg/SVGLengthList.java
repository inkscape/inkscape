
package org.w3c.dom.svg;

import org.w3c.dom.DOMException;

public interface SVGLengthList {
  public int getNumberOfItems( );

  public void   clear (  )
                  throws DOMException;
  public SVGLength initialize ( SVGLength newItem )
                  throws DOMException, SVGException;
  public SVGLength getItem ( int index )
                  throws DOMException;
  public SVGLength insertItemBefore ( SVGLength newItem, int index )
                  throws DOMException, SVGException;
  public SVGLength replaceItem ( SVGLength newItem, int index )
                  throws DOMException, SVGException;
  public SVGLength removeItem ( int index )
                  throws DOMException;
  public SVGLength appendItem ( SVGLength newItem )
                  throws DOMException, SVGException;
}
