
package org.w3c.dom.svg;

public interface SVGFEColorMatrixElement extends 
               SVGElement,
               SVGFilterPrimitiveStandardAttributes {
  // Color Matrix Types
  public static final short SVG_FECOLORMATRIX_TYPE_UNKNOWN          = 0;
  public static final short SVG_FECOLORMATRIX_TYPE_MATRIX           = 1;
  public static final short SVG_FECOLORMATRIX_TYPE_SATURATE         = 2;
  public static final short SVG_FECOLORMATRIX_TYPE_HUEROTATE        = 3;
  public static final short SVG_FECOLORMATRIX_TYPE_LUMINANCETOALPHA = 4;

  public SVGAnimatedString      getIn1( );
  public SVGAnimatedEnumeration getType( );
  public SVGAnimatedNumberList  getValues( );
}
