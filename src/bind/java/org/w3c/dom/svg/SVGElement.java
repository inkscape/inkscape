
package org.w3c.dom.svg;

import org.w3c.dom.DOMException;
import org.w3c.dom.Element;

public interface SVGElement extends 
               Element {
  public String getId( );
  public void      setId( String id )
                       throws DOMException;
  public String getXMLbase( );
  public void      setXMLbase( String xmlbase )
                       throws DOMException;
  public SVGSVGElement getOwnerSVGElement( );
  public SVGElement getViewportElement( );
}
