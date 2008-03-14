
package org.w3c.dom.svg;

import org.w3c.dom.DOMException;

public interface SVGPathSegCurvetoCubicSmoothAbs extends 
               SVGPathSeg {
  public float   getX( );
  public void      setX( float x )
                       throws DOMException;
  public float   getY( );
  public void      setY( float y )
                       throws DOMException;
  public float   getX2( );
  public void      setX2( float x2 )
                       throws DOMException;
  public float   getY2( );
  public void      setY2( float y2 )
                       throws DOMException;
}
