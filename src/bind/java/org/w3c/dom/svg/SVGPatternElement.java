
package org.w3c.dom.svg;

public interface SVGPatternElement extends 
               SVGElement,
               SVGURIReference,
               SVGTests,
               SVGLangSpace,
               SVGExternalResourcesRequired,
               SVGStylable,
               SVGFitToViewBox,
               SVGUnitTypes {
  public SVGAnimatedEnumeration   getPatternUnits( );
  public SVGAnimatedEnumeration   getPatternContentUnits( );
  public SVGAnimatedTransformList getPatternTransform( );
  public SVGAnimatedLength        getX( );
  public SVGAnimatedLength        getY( );
  public SVGAnimatedLength        getWidth( );
  public SVGAnimatedLength        getHeight( );
}
