
package org.w3c.dom.svg;

public interface SVGFEConvolveMatrixElement extends 
               SVGElement,
               SVGFilterPrimitiveStandardAttributes {
  // Edge Mode Values
  public static final short SVG_EDGEMODE_UNKNOWN   = 0;
  public static final short SVG_EDGEMODE_DUPLICATE = 1;
  public static final short SVG_EDGEMODE_WRAP      = 2;
  public static final short SVG_EDGEMODE_NONE      = 3;

  public SVGAnimatedInteger     getOrderX( );
  public SVGAnimatedInteger     getOrderY( );
  public SVGAnimatedNumberList  getKernelMatrix( );
  public SVGAnimatedNumber      getDivisor( );
  public SVGAnimatedNumber      getBias( );
  public SVGAnimatedInteger     getTargetX( );
  public SVGAnimatedInteger     getTargetY( );
  public SVGAnimatedEnumeration getEdgeMode( );
  public SVGAnimatedNumber      getKernelUnitLengthX( );
  public SVGAnimatedNumber      getKernelUnitLengthY( );
  public SVGAnimatedBoolean     getPreserveAlpha( );
}
