
package org.w3c.dom.svg;

import org.w3c.dom.DOMException;

public interface SVGPathSegList {
  public int getNumberOfItems( );

  public void   clear (  )
                  throws DOMException;
  public SVGPathSeg initialize ( SVGPathSeg newItem )
                  throws DOMException, SVGException;
  public SVGPathSeg getItem ( int index )
                  throws DOMException;
  public SVGPathSeg insertItemBefore ( SVGPathSeg newItem, int index )
                  throws DOMException, SVGException;
  public SVGPathSeg replaceItem ( SVGPathSeg newItem, int index )
                  throws DOMException, SVGException;
  public SVGPathSeg removeItem ( int index )
                  throws DOMException;
  public SVGPathSeg appendItem ( SVGPathSeg newItem )
                  throws DOMException, SVGException;
}
