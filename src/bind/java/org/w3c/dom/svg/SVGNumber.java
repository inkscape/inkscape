
package org.w3c.dom.svg;

import org.w3c.dom.DOMException;

public interface SVGNumber {
  public float getValue( );
  public void           setValue( float value )
                       throws DOMException;
}
