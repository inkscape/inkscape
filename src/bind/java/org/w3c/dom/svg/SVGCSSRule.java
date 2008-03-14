
package org.w3c.dom.svg;

import org.w3c.dom.css.CSSRule;

public interface SVGCSSRule extends 
               CSSRule {
  // Additional CSS RuleType to support ICC color specifications
  public static final short COLOR_PROFILE_RULE = 7;
}
