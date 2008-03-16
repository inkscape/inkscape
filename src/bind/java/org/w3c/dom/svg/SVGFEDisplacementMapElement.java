
package org.w3c.dom.svg;

public interface SVGFEDisplacementMapElement extends 
               SVGElement,
               SVGFilterPrimitiveStandardAttributes {
  // Channel Selectors
  public static final short SVG_CHANNEL_UNKNOWN = 0;
  public static final short SVG_CHANNEL_R       = 1;
  public static final short SVG_CHANNEL_G       = 2;
  public static final short SVG_CHANNEL_B       = 3;
  public static final short SVG_CHANNEL_A       = 4;

  public SVGAnimatedString      getIn1( );
  public SVGAnimatedString      getIn2( );
  public SVGAnimatedNumber      getScale( );
  public SVGAnimatedEnumeration getXChannelSelector( );
  public SVGAnimatedEnumeration getYChannelSelector( );
}
