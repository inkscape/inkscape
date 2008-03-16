
package org.w3c.dom.svg;

import org.w3c.dom.DOMException;

public interface SVGStyleElement extends 
               SVGElement {
  public String getXMLspace( );
  public void      setXMLspace( String xmlspace )
                       throws DOMException;
  public String getType( );
  public void      setType( String type )
                       throws DOMException;
  public String getMedia( );
  public void      setMedia( String media )
                       throws DOMException;
  public String getTitle( );
  public void      setTitle( String title )
                       throws DOMException;
}
