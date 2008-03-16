
package org.w3c.dom.svg;

public interface SVGTextPathElement extends 
               SVGTextContentElement,
               SVGURIReference {
  // textPath Method Types
  public static final short TEXTPATH_METHODTYPE_UNKNOWN   = 0;
  public static final short TEXTPATH_METHODTYPE_ALIGN     = 1;
  public static final short TEXTPATH_METHODTYPE_STRETCH     = 2;
  // textPath Spacing Types
  public static final short TEXTPATH_SPACINGTYPE_UNKNOWN   = 0;
  public static final short TEXTPATH_SPACINGTYPE_AUTO     = 1;
  public static final short TEXTPATH_SPACINGTYPE_EXACT     = 2;

  public SVGAnimatedLength              getStartOffset( );
  public SVGAnimatedEnumeration getMethod( );
  public SVGAnimatedEnumeration getSpacing( );
}
