
package org.w3c.dom.svg;

import org.w3c.dom.DOMException;

public interface SVGGlyphRefElement extends 
               SVGElement,
               SVGURIReference,
               SVGStylable {
  public String getGlyphRef( );
  public void      setGlyphRef( String glyphRef )
                       throws DOMException;
  public String getFormat( );
  public void      setFormat( String format )
                       throws DOMException;
  public float    getX( );
  public void      setX( float x )
                       throws DOMException;
  public float    getY( );
  public void      setY( float y )
                       throws DOMException;
  public float    getDx( );
  public void      setDx( float dx )
                       throws DOMException;
  public float    getDy( );
  public void      setDy( float dy )
                       throws DOMException;
}
