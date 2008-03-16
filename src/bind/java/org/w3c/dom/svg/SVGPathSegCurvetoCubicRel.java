
package org.w3c.dom.svg;

import org.w3c.dom.DOMException;

public interface SVGPathSegCurvetoCubicRel extends 
               SVGPathSeg {
  public float   getX( );
  public void      setX( float x )
                       throws DOMException;
  public float   getY( );
  public void      setY( float y )
                       throws DOMException;
  public float   getX1( );
  public void      setX1( float x1 )
                       throws DOMException;
  public float   getY1( );
  public void      setY1( float y1 )
                       throws DOMException;
  public float   getX2( );
  public void      setX2( float x2 )
                       throws DOMException;
  public float   getY2( );
  public void      setY2( float y2 )
                       throws DOMException;
}
