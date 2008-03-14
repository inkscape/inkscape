
package org.w3c.dom.svg;

import org.w3c.dom.DOMException;

public interface SVGPathSegArcRel extends 
               SVGPathSeg {
  public float   getX( );
  public void      setX( float x )
                       throws DOMException;
  public float   getY( );
  public void      setY( float y )
                       throws DOMException;
  public float   getR1( );
  public void      setR1( float r1 )
                       throws DOMException;
  public float   getR2( );
  public void      setR2( float r2 )
                       throws DOMException;
  public float   getAngle( );
  public void      setAngle( float angle )
                       throws DOMException;
  public boolean getLargeArcFlag( );
  public void      setLargeArcFlag( boolean largeArcFlag )
                       throws DOMException;
  public boolean getSweepFlag( );
  public void      setSweepFlag( boolean sweepFlag )
                       throws DOMException;
}
