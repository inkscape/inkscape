
package org.w3c.dom.svg;

import org.w3c.dom.css.CSSStyleDeclaration;
import org.w3c.dom.css.CSSValue;

public interface SVGStylable {
  public SVGAnimatedString getClassName( );
  public CSSStyleDeclaration getStyle( );

  public CSSValue getPresentationAttribute ( String name );
}
