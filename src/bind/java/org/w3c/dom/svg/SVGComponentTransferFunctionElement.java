
package org.w3c.dom.svg;

public interface SVGComponentTransferFunctionElement extends 
               SVGElement {
  // Component Transfer Types
  public static final short SVG_FECOMPONENTTRANSFER_TYPE_UNKNOWN  = 0;
  public static final short SVG_FECOMPONENTTRANSFER_TYPE_IDENTITY = 1;
  public static final short SVG_FECOMPONENTTRANSFER_TYPE_TABLE    = 2;
  public static final short SVG_FECOMPONENTTRANSFER_TYPE_DISCRETE    = 3;
  public static final short SVG_FECOMPONENTTRANSFER_TYPE_LINEAR   = 4;
  public static final short SVG_FECOMPONENTTRANSFER_TYPE_GAMMA    = 5;

  public SVGAnimatedEnumeration getType( );
  public SVGAnimatedNumberList  getTableValues( );
  public SVGAnimatedNumber      getSlope( );
  public SVGAnimatedNumber      getIntercept( );
  public SVGAnimatedNumber      getAmplitude( );
  public SVGAnimatedNumber      getExponent( );
  public SVGAnimatedNumber      getOffset( );
}
