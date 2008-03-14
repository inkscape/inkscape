
package org.w3c.dom.svg;

import org.w3c.dom.DOMException;

public interface SVGLangSpace {
  public String getXMLlang( );
  public void      setXMLlang( String xmllang )
                       throws DOMException;
  public String getXMLspace( );
  public void      setXMLspace( String xmlspace )
                       throws DOMException;
}
