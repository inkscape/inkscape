
package org.w3c.dom.svg;

import org.w3c.dom.DOMException;

public interface SVGMatrix {
  public float getA( );
  public void      setA( float a )
                       throws DOMException;
  public float getB( );
  public void      setB( float b )
                       throws DOMException;
  public float getC( );
  public void      setC( float c )
                       throws DOMException;
  public float getD( );
  public void      setD( float d )
                       throws DOMException;
  public float getE( );
  public void      setE( float e )
                       throws DOMException;
  public float getF( );
  public void      setF( float f )
                       throws DOMException;

  public SVGMatrix multiply ( SVGMatrix secondMatrix );
  public SVGMatrix inverse (  )
                  throws SVGException;
  public SVGMatrix translate ( float x, float y );
  public SVGMatrix scale ( float scaleFactor );
  public SVGMatrix scaleNonUniform ( float scaleFactorX, float scaleFactorY );
  public SVGMatrix rotate ( float angle );
  public SVGMatrix rotateFromVector ( float x, float y )
                  throws SVGException;
  public SVGMatrix flipX (  );
  public SVGMatrix flipY (  );
  public SVGMatrix skewX ( float angle );
  public SVGMatrix skewY ( float angle );
}
