
package org.w3c.dom.svg;

public interface SVGPathSeg {
  // Path Segment Types
  public static final short PATHSEG_UNKNOWN                      = 0;
  public static final short PATHSEG_CLOSEPATH                    = 1;
  public static final short PATHSEG_MOVETO_ABS                   = 2;
  public static final short PATHSEG_MOVETO_REL                   = 3;
  public static final short PATHSEG_LINETO_ABS                   = 4;
  public static final short PATHSEG_LINETO_REL                   = 5;
  public static final short PATHSEG_CURVETO_CUBIC_ABS            = 6;
  public static final short PATHSEG_CURVETO_CUBIC_REL            = 7;
  public static final short PATHSEG_CURVETO_QUADRATIC_ABS        = 8;
  public static final short PATHSEG_CURVETO_QUADRATIC_REL        = 9;
  public static final short PATHSEG_ARC_ABS                      = 10;
  public static final short PATHSEG_ARC_REL                      = 11;
  public static final short PATHSEG_LINETO_HORIZONTAL_ABS        = 12;
  public static final short PATHSEG_LINETO_HORIZONTAL_REL        = 13;
  public static final short PATHSEG_LINETO_VERTICAL_ABS          = 14;
  public static final short PATHSEG_LINETO_VERTICAL_REL          = 15;
  public static final short PATHSEG_CURVETO_CUBIC_SMOOTH_ABS     = 16;
  public static final short PATHSEG_CURVETO_CUBIC_SMOOTH_REL     = 17;
  public static final short PATHSEG_CURVETO_QUADRATIC_SMOOTH_ABS = 18;
  public static final short PATHSEG_CURVETO_QUADRATIC_SMOOTH_REL = 19;

  public short getPathSegType( );
  public String      getPathSegTypeAsLetter( );
}
