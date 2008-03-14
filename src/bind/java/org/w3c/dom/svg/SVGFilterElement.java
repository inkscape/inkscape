
package org.w3c.dom.svg;

public interface SVGFilterElement extends 
               SVGElement,
               SVGURIReference,
               SVGLangSpace,
               SVGExternalResourcesRequired,
               SVGStylable,
               SVGUnitTypes {
  public SVGAnimatedEnumeration getFilterUnits( );
  public SVGAnimatedEnumeration getPrimitiveUnits( );
  public SVGAnimatedLength      getX( );
  public SVGAnimatedLength      getY( );
  public SVGAnimatedLength      getWidth( );
  public SVGAnimatedLength      getHeight( );
  public SVGAnimatedInteger    getFilterResX( );
  public SVGAnimatedInteger    getFilterResY( );

  public void setFilterRes ( int filterResX, int filterResY );
}
