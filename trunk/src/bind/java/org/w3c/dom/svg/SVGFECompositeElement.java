
package org.w3c.dom.svg;

public interface SVGFECompositeElement extends 
               SVGElement,
               SVGFilterPrimitiveStandardAttributes {
  // Composite Operators
  public static final short SVG_FECOMPOSITE_OPERATOR_UNKNOWN    = 0;
  public static final short SVG_FECOMPOSITE_OPERATOR_OVER       = 1;
  public static final short SVG_FECOMPOSITE_OPERATOR_IN         = 2;
  public static final short SVG_FECOMPOSITE_OPERATOR_OUT        = 3;
  public static final short SVG_FECOMPOSITE_OPERATOR_ATOP       = 4;
  public static final short SVG_FECOMPOSITE_OPERATOR_XOR        = 5;
  public static final short SVG_FECOMPOSITE_OPERATOR_ARITHMETIC = 6;

  public SVGAnimatedString      getIn1( );
  public SVGAnimatedString      getIn2( );
  public SVGAnimatedEnumeration getOperator( );
  public SVGAnimatedNumber      getK1( );
  public SVGAnimatedNumber      getK2( );
  public SVGAnimatedNumber      getK3( );
  public SVGAnimatedNumber      getK4( );
}
