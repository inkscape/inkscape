
package org.w3c.dom.svg;

import org.w3c.dom.events.UIEvent;

public interface SVGZoomEvent extends 
               UIEvent {
  public SVGRect getZoomRectScreen( );
  public float getPreviousScale( );
  public SVGPoint getPreviousTranslate( );
  public float getNewScale( );
  public SVGPoint getNewTranslate( );
}
