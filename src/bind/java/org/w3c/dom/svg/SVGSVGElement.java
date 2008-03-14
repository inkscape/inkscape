
package org.w3c.dom.svg;

import org.w3c.dom.events.DocumentEvent;
import org.w3c.dom.events.EventTarget;
import org.w3c.dom.DOMException;
import org.w3c.dom.NodeList;
import org.w3c.dom.Element;
import org.w3c.dom.css.ViewCSS;
import org.w3c.dom.css.DocumentCSS;
import org.w3c.dom.css.RGBColor;

public interface SVGSVGElement extends 
               SVGElement,
               SVGTests,
               SVGLangSpace,
               SVGExternalResourcesRequired,
               SVGStylable,
               SVGLocatable,
               SVGFitToViewBox,
               SVGZoomAndPan,
               EventTarget,
               DocumentEvent,
               ViewCSS,
               DocumentCSS {
  public SVGAnimatedLength getX( );
  public SVGAnimatedLength getY( );
  public SVGAnimatedLength getWidth( );
  public SVGAnimatedLength getHeight( );
  public String         getContentScriptType( );
  public void      setContentScriptType( String contentScriptType )
                       throws DOMException;
  public String         getContentStyleType( );
  public void      setContentStyleType( String contentStyleType )
                       throws DOMException;
  public SVGRect           getViewport( );
  public float getPixelUnitToMillimeterX( );
  public float getPixelUnitToMillimeterY( );
  public float getScreenPixelToMillimeterX( );
  public float getScreenPixelToMillimeterY( );
  public boolean getUseCurrentView( );
  public void      setUseCurrentView( boolean useCurrentView )
                       throws DOMException;
  public SVGViewSpec getCurrentView( );
  public float getCurrentScale( );
  public void      setCurrentScale( float currentScale )
                       throws DOMException;
  public SVGPoint getCurrentTranslate( );

  public int          suspendRedraw ( int max_wait_milliseconds );
  public void          unsuspendRedraw ( int suspend_handle_id )
                  throws DOMException;
  public void          unsuspendRedrawAll (  );
  public void          forceRedraw (  );
  public void          pauseAnimations (  );
  public void          unpauseAnimations (  );
  public boolean       animationsPaused (  );
  public float         getCurrentTime (  );
  public void          setCurrentTime ( float seconds );
  public NodeList      getIntersectionList ( SVGRect rect, SVGElement referenceElement );
  public NodeList      getEnclosureList ( SVGRect rect, SVGElement referenceElement );
  public boolean       checkIntersection ( SVGElement element, SVGRect rect );
  public boolean       checkEnclosure ( SVGElement element, SVGRect rect );
  public void          deselectAll (  );
  public SVGNumber              createSVGNumber (  );
  public SVGLength              createSVGLength (  );
  public SVGAngle               createSVGAngle (  );
  public SVGPoint               createSVGPoint (  );
  public SVGMatrix              createSVGMatrix (  );
  public SVGRect                createSVGRect (  );
  public SVGTransform           createSVGTransform (  );
  public SVGTransform     createSVGTransformFromMatrix ( SVGMatrix matrix );
  public Element         getElementById ( String elementId );
}
