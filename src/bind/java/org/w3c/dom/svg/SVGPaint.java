
package org.w3c.dom.svg;

import org.w3c.dom.css.RGBColor;

public interface SVGPaint extends 
               SVGColor {
  // Paint Types
  public static final short SVG_PAINTTYPE_UNKNOWN               = 0;
  public static final short SVG_PAINTTYPE_RGBCOLOR              = 1;
  public static final short SVG_PAINTTYPE_RGBCOLOR_ICCCOLOR     = 2;
  public static final short SVG_PAINTTYPE_NONE                  = 101;
  public static final short SVG_PAINTTYPE_CURRENTCOLOR          = 102;
  public static final short SVG_PAINTTYPE_URI_NONE              = 103;
  public static final short SVG_PAINTTYPE_URI_CURRENTCOLOR      = 104;
  public static final short SVG_PAINTTYPE_URI_RGBCOLOR          = 105;
  public static final short SVG_PAINTTYPE_URI_RGBCOLOR_ICCCOLOR = 106;
  public static final short SVG_PAINTTYPE_URI                   = 107;

  public short getPaintType( );
  public String      getUri( );

  public void setUri ( String uri );
  public void setPaint ( short paintType, String uri, String rgbColor, String iccColor )
                  throws SVGException;
}
