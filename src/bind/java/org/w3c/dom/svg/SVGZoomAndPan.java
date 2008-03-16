
package org.w3c.dom.svg;

import org.w3c.dom.DOMException;

public interface SVGZoomAndPan {
  // Zoom and Pan Types
  public static final short SVG_ZOOMANDPAN_UNKNOWN   = 0;
  public static final short SVG_ZOOMANDPAN_DISABLE = 1;
  public static final short SVG_ZOOMANDPAN_MAGNIFY = 2;

  public short getZoomAndPan( );
  public void      setZoomAndPan( short zoomAndPan )
                       throws DOMException;
}
