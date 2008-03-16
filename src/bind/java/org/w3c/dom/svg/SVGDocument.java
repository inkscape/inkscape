
package org.w3c.dom.svg;

import org.w3c.dom.Document;
import org.w3c.dom.events.DocumentEvent;

public interface SVGDocument extends 
               Document,
               DocumentEvent {
  public String    getTitle( );
  public String     getReferrer( );
  public String      getDomain( );
  public String      getURL( );
  public SVGSVGElement getRootElement( );
}
