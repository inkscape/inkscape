
package org.w3c.dom.svg;

public interface SVGTransform {
  // Transform Types
  public static final short SVG_TRANSFORM_UNKNOWN   = 0;
  public static final short SVG_TRANSFORM_MATRIX    = 1;
  public static final short SVG_TRANSFORM_TRANSLATE = 2;
  public static final short SVG_TRANSFORM_SCALE     = 3;
  public static final short SVG_TRANSFORM_ROTATE    = 4;
  public static final short SVG_TRANSFORM_SKEWX     = 5;
  public static final short SVG_TRANSFORM_SKEWY     = 6;

  public short getType( );
  public SVGMatrix getMatrix( );
  public float getAngle( );

  public void setMatrix ( SVGMatrix matrix );
  public void setTranslate ( float tx, float ty );
  public void setScale ( float sx, float sy );
  public void setRotate ( float angle, float cx, float cy );
  public void setSkewX ( float angle );
  public void setSkewY ( float angle );
}
