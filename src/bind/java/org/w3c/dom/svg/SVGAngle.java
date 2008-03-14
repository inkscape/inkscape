
package org.w3c.dom.svg;

import org.w3c.dom.DOMException;
public interface SVGAngle {
  // Angle Unit Types
  public static final short SVG_ANGLETYPE_UNKNOWN     = 0;
  public static final short SVG_ANGLETYPE_UNSPECIFIED = 1;
  public static final short SVG_ANGLETYPE_DEG         = 2;
  public static final short SVG_ANGLETYPE_RAD         = 3;
  public static final short SVG_ANGLETYPE_GRAD        = 4;

  public short getUnitType( );
  public float          getValue( );
  public void           setValue( float value )
                       throws DOMException;
  public float          getValueInSpecifiedUnits( );
  public void           setValueInSpecifiedUnits( float valueInSpecifiedUnits )
                       throws DOMException;
  public String      getValueAsString( );
  public void           setValueAsString( String valueAsString )
                       throws DOMException;

  public void newValueSpecifiedUnits ( short unitType, float valueInSpecifiedUnits );
  public void convertToSpecifiedUnits ( short unitType );
}
