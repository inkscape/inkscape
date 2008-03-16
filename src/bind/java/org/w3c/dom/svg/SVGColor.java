
package org.w3c.dom.svg;

import org.w3c.dom.css.RGBColor;
import org.w3c.dom.css.CSSValue;

public interface SVGColor extends 
               CSSValue {
  // Color Types
  public static final short SVG_COLORTYPE_UNKNOWN           = 0;
  public static final short SVG_COLORTYPE_RGBCOLOR          = 1;
  public static final short SVG_COLORTYPE_RGBCOLOR_ICCCOLOR = 2;
  public static final short SVG_COLORTYPE_CURRENTCOLOR      = 3;

  public short getColorType( );
  public RGBColor  getRGBColor( );
  public SVGICCColor    getICCColor( );

  public void        setRGBColor ( String rgbColor )
                  throws SVGException;
  public void        setRGBColorICCColor ( String rgbColor, String iccColor )
                  throws SVGException;
  public void        setColor ( short colorType, String rgbColor, String iccColor )
                  throws SVGException;
}
