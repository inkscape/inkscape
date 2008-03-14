
package org.w3c.dom.svg;

import org.w3c.dom.DOMException;

public interface SVGStringList {
  public int getNumberOfItems( );

  public void   clear (  )
                  throws DOMException;
  public String initialize ( String newItem )
                  throws DOMException, SVGException;
  public String getItem ( int index )
                  throws DOMException;
  public String insertItemBefore ( String newItem, int index )
                  throws DOMException, SVGException;
  public String replaceItem ( String newItem, int index )
                  throws DOMException, SVGException;
  public String removeItem ( int index )
                  throws DOMException;
  public String appendItem ( String newItem )
                  throws DOMException, SVGException;
}
