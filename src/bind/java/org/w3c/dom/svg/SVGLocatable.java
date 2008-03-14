
package org.w3c.dom.svg;

public interface SVGLocatable {
  public SVGElement              getNearestViewportElement( );
  public SVGElement              getFarthestViewportElement( );

  public SVGRect   getBBox (  );
  public SVGMatrix getCTM (  );
  public SVGMatrix getScreenCTM (  );
  public SVGMatrix getTransformToElement ( SVGElement element )
                  throws SVGException;
}
