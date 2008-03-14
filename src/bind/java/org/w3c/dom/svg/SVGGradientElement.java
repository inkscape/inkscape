
package org.w3c.dom.svg;

public interface SVGGradientElement extends 
               SVGElement,
               SVGURIReference,
               SVGExternalResourcesRequired,
               SVGStylable,
               SVGUnitTypes {
  // Spread Method Types
  public static final short SVG_SPREADMETHOD_UNKNOWN = 0;
  public static final short SVG_SPREADMETHOD_PAD     = 1;
  public static final short SVG_SPREADMETHOD_REFLECT = 2;
  public static final short SVG_SPREADMETHOD_REPEAT  = 3;

  public SVGAnimatedEnumeration   getGradientUnits( );
  public SVGAnimatedTransformList getGradientTransform( );
  public SVGAnimatedEnumeration   getSpreadMethod( );
}
