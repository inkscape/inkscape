
package org.w3c.dom.svg;

public interface SVGFEMorphologyElement extends 
               SVGElement,
               SVGFilterPrimitiveStandardAttributes {
  // Morphology Operators
  public static final short SVG_MORPHOLOGY_OPERATOR_UNKNOWN = 0;
  public static final short SVG_MORPHOLOGY_OPERATOR_ERODE   = 1;
  public static final short SVG_MORPHOLOGY_OPERATOR_DILATE  = 2;

  public SVGAnimatedString      getIn1( );
  public SVGAnimatedEnumeration getOperator( );
  public SVGAnimatedNumber      getRadiusX( );
  public SVGAnimatedNumber      getRadiusY( );
}
