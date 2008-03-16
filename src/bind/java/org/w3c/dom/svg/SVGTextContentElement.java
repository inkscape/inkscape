
package org.w3c.dom.svg;

import org.w3c.dom.events.EventTarget;
import org.w3c.dom.DOMException;

public interface SVGTextContentElement extends 
               SVGElement,
               SVGTests,
               SVGLangSpace,
               SVGExternalResourcesRequired,
               SVGStylable,
               EventTarget {
  // lengthAdjust Types
  public static final short LENGTHADJUST_UNKNOWN   = 0;
  public static final short LENGTHADJUST_SPACING     = 1;
  public static final short LENGTHADJUST_SPACINGANDGLYPHS     = 2;

  public SVGAnimatedLength      getTextLength( );
  public SVGAnimatedEnumeration getLengthAdjust( );

  public int      getNumberOfChars (  );
  public float    getComputedTextLength (  );
  public float    getSubStringLength ( int charnum, int nchars )
                  throws DOMException;
  public SVGPoint getStartPositionOfChar ( int charnum )
                  throws DOMException;
  public SVGPoint getEndPositionOfChar ( int charnum )
                  throws DOMException;
  public SVGRect  getExtentOfChar ( int charnum )
                  throws DOMException;
  public float    getRotationOfChar ( int charnum )
                  throws DOMException;
  public int      getCharNumAtPosition ( SVGPoint point );
  public void     selectSubString ( int charnum, int nchars )
                  throws DOMException;
}
