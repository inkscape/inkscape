
package org.w3c.dom.svg;

public interface SVGMaskElement extends 
               SVGElement,
               SVGTests,
               SVGLangSpace,
               SVGExternalResourcesRequired,
               SVGStylable,
               SVGUnitTypes {
  public SVGAnimatedEnumeration getMaskUnits( );
  public SVGAnimatedEnumeration getMaskContentUnits( );
  public SVGAnimatedLength      getX( );
  public SVGAnimatedLength      getY( );
  public SVGAnimatedLength      getWidth( );
  public SVGAnimatedLength      getHeight( );
}
