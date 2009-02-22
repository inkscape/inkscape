
package org.w3c.dom.svg;

public interface SVGViewSpec extends 
               SVGZoomAndPan,
               SVGFitToViewBox {
  public SVGTransformList getTransform( );
  public SVGElement       getViewTarget( );
  public String        getViewBoxString( );
  public String        getPreserveAspectRatioString( );
  public String        getTransformString( );
  public String        getViewTargetString( );
}
