
package org.w3c.dom.svg;

public interface SVGFEGaussianBlurElement extends 
               SVGElement,
               SVGFilterPrimitiveStandardAttributes {
  public SVGAnimatedString getIn1( );
  public SVGAnimatedNumber getStdDeviationX( );
  public SVGAnimatedNumber getStdDeviationY( );

  public void setStdDeviation ( float stdDeviationX, float stdDeviationY );
}
