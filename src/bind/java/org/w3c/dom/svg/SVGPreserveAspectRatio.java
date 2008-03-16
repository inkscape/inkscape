
package org.w3c.dom.svg;

import org.w3c.dom.DOMException;

public interface SVGPreserveAspectRatio {
  // Alignment Types
  public static final short SVG_PRESERVEASPECTRATIO_UNKNOWN   = 0;
  public static final short SVG_PRESERVEASPECTRATIO_NONE     = 1;
  public static final short SVG_PRESERVEASPECTRATIO_XMINYMIN = 2;
  public static final short SVG_PRESERVEASPECTRATIO_XMIDYMIN = 3;
  public static final short SVG_PRESERVEASPECTRATIO_XMAXYMIN = 4;
  public static final short SVG_PRESERVEASPECTRATIO_XMINYMID = 5;
  public static final short SVG_PRESERVEASPECTRATIO_XMIDYMID = 6;
  public static final short SVG_PRESERVEASPECTRATIO_XMAXYMID = 7;
  public static final short SVG_PRESERVEASPECTRATIO_XMINYMAX = 8;
  public static final short SVG_PRESERVEASPECTRATIO_XMIDYMAX = 9;
  public static final short SVG_PRESERVEASPECTRATIO_XMAXYMAX = 10;
  // Meet-or-slice Types
  public static final short SVG_MEETORSLICE_UNKNOWN   = 0;
  public static final short SVG_MEETORSLICE_MEET  = 1;
  public static final short SVG_MEETORSLICE_SLICE = 2;

  public short getAlign( );
  public void      setAlign( short align )
                       throws DOMException;
  public short getMeetOrSlice( );
  public void      setMeetOrSlice( short meetOrSlice )
                       throws DOMException;
}
