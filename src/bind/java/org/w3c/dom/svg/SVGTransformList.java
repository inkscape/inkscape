
package org.w3c.dom.svg;

import org.w3c.dom.DOMException;

public interface SVGTransformList {
  public int getNumberOfItems( );

  public void   clear (  )
                  throws DOMException;
  public SVGTransform initialize ( SVGTransform newItem )
                  throws DOMException, SVGException;
  public SVGTransform getItem ( int index )
                  throws DOMException;
  public SVGTransform insertItemBefore ( SVGTransform newItem, int index )
                  throws DOMException, SVGException;
  public SVGTransform replaceItem ( SVGTransform newItem, int index )
                  throws DOMException, SVGException;
  public SVGTransform removeItem ( int index )
                  throws DOMException;
  public SVGTransform appendItem ( SVGTransform newItem )
                  throws DOMException, SVGException;
  public SVGTransform createSVGTransformFromMatrix ( SVGMatrix matrix );
  public SVGTransform consolidate (  );
}
