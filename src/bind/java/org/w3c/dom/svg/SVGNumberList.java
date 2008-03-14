
package org.w3c.dom.svg;

import org.w3c.dom.DOMException;

public interface SVGNumberList {
  public int getNumberOfItems( );

  public void   clear (  )
                  throws DOMException;
  public SVGNumber initialize ( SVGNumber newItem )
                  throws DOMException, SVGException;
  public SVGNumber getItem ( int index )
                  throws DOMException;
  public SVGNumber insertItemBefore ( SVGNumber newItem, int index )
                  throws DOMException, SVGException;
  public SVGNumber replaceItem ( SVGNumber newItem, int index )
                  throws DOMException, SVGException;
  public SVGNumber removeItem ( int index )
                  throws DOMException;
  public SVGNumber appendItem ( SVGNumber newItem )
                  throws DOMException, SVGException;
}
