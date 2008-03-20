#ifndef __SVG_H__
#define __SVG_H__

/**
 * Phoebe DOM Implementation.
 *
 * This is a C++ approximation of the W3C DOM model, which follows
 * fairly closely the specifications in the various .idl files, copies of
 * which are provided for reference.  Most important is this one:
 *
 * http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/idl-definitions.html
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2005-2008 Bob Jamison
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


// For access to DOM2 core
#include "dom/dom.h"

// For access to DOM2 events
#include "dom/events.h"

// For access to those parts from DOM2 CSS OM used by SVG DOM.
#include "dom/css.h"

// For access to those parts from DOM2 Views OM used by SVG DOM.
#include "dom/views.h"

// For access to the SMIL OM used by SVG DOM.
#include "dom/smil.h"



#include "svgtypes.h"

#include <math.h>



namespace org
{
namespace w3c
{
namespace dom
{
namespace svg
{


//local definitions
typedef dom::DOMString DOMString;
typedef dom::DOMException DOMException;
typedef dom::Element Element;
typedef dom::ElementPtr ElementPtr;
typedef dom::Document Document;
typedef dom::DocumentPtr DocumentPtr;
typedef dom::NodeList NodeList;




class SVGElement;
typedef Ptr<SVGElement> SVGElementPtr;
class SVGSVGElement;
typedef Ptr<SVGSVGElement> SVGSVGElementPtr;
class SVGDocument;
typedef Ptr<SVGDocument> SVGDocumentPtr;


/*#########################################################################
## SVGElement
#########################################################################*/

/**
 *
 */
class SVGElement : virtual public Element
{
public:

    /**
     *
     */
    virtual DOMString getId() =0;

    /**
     *
     */
    virtual void setId(const DOMString &val)
                       throw (DOMException) =0;

    /**
     *
     */
    virtual DOMString getXmlBase() = 0;

    /**
     *
     */
    virtual void setXmlBase(const DOMString &val)
                            throw (DOMException) = 0;

    /**
     *
     */
    virtual SVGSVGElementPtr getOwnerSVGElement() = 0;

    /**
     *
     */
    virtual SVGElementPtr getViewportElement() = 0;


    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    virtual ~SVGElement() {}


};



/*#########################################################################
## SVGDocument
#########################################################################*/

/**
 *
 */
class SVGDocument : virtual public Document,
                    virtual public events::DocumentEvent
{
public:


    /**
     *
     */
    virtual DOMString getTitle() =0;

    /**
     *
     */
    virtual DOMString getReferrer() =0;

    /**
     *
     */
    virtual DOMString getDomain() =0;

    /**
     *
     */
    virtual DOMString getURL() =0;

    /**
     *
     */
    virtual SVGSVGElementPtr getRootElement() =0;


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGDocument() {}

};



/*#########################################################################
## SVGSVGElement
#########################################################################*/

/**
 *
 */
class SVGSVGElement : virtual public SVGElement,
                      public SVGTests,
                      public SVGLangSpace,
                      public SVGExternalResourcesRequired,
                      public SVGStylable,
                      public SVGLocatable,
                      public SVGFitToViewBox,
                      public SVGZoomAndPan,
                      public events::EventTarget,
                      public events::DocumentEvent,
                      public css::ViewCSS,
                      public css::DocumentCSS
{
public:

    /**
     *
     */
    virtual SVGAnimatedLength getX() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getY() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getWidth() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getHeight() =0;

    /**
     *
     */
    virtual DOMString getContentScriptType() =0;

    /**
     *
     */
    virtual void setContentScriptType(const DOMString &val)
                                     throw (DOMException) =0;


    /**
     *
     */
    virtual DOMString getContentStyleType() =0;

    /**
     *
     */
    virtual void setContentStyleType(const DOMString &val)
                                     throw (DOMException) =0;

    /**
     *
     */
    virtual SVGRect getViewport() =0;

    /**
     *
     */
    virtual double getPixelUnitToMillimeterX() =0;

    /**
     *
     */
    virtual double getPixelUnitToMillimeterY() =0;

    /**
     *
     */
    virtual double getScreenPixelToMillimeterX() =0;

    /**
     *
     */
    virtual double getScreenPixelToMillimeterY() =0;


    /**
     *
     */
    virtual bool getUseCurrentView() =0;

    /**
     *
     */
    virtual void setUseCurrentView(bool val) throw (DOMException) =0;

    /**
     *
     */
    virtual SVGViewSpec getCurrentView() =0;


    /**
     *
     */
    virtual double getCurrentScale() =0;

    /**
     *
     */
    virtual void setCurrentScale(double val)
                                 throw (DOMException) =0;


    /**
     *
     */
    virtual SVGPoint getCurrentTranslate() =0;


    /**
     *
     */
    virtual unsigned long suspendRedraw (unsigned long max_wait_milliseconds ) =0;

    /**
     *
     */
    virtual void unsuspendRedraw (unsigned long suspend_handle_id )
                                  throw( DOMException ) =0;

    /**
     *
     */
    virtual void unsuspendRedrawAll (  ) =0;

    /**
     *
     */
    virtual void forceRedraw (  ) =0;

    /**
     *
     */
    virtual void pauseAnimations (  ) =0;

    /**
     *
     */
    virtual void unpauseAnimations (  ) =0;

    /**
     *
     */
    virtual bool animationsPaused (  ) =0;

    /**
     *
     */
    virtual double getCurrentTime (  ) =0;

    /**
     *
     */
    virtual void setCurrentTime (double seconds ) =0;

    /**
     *
     */
    virtual NodeList getIntersectionList(const SVGRect &rect,
                                         const SVGElementPtr referenceElement ) =0;

    /**
     *
     */
    virtual NodeList getEnclosureList (const SVGRect &rect,
                                       const SVGElementPtr referenceElement ) =0;

    /**
     *
     */
    virtual bool checkIntersection (const SVGElementPtr element, const SVGRect &rect ) =0;

    /**
     *
     */
    virtual bool checkEnclosure (const SVGElementPtr element, const SVGRect &rect ) =0;

    /**
     *
     */
    virtual void deselectAll (  ) =0;

    /**
     *
     */
    virtual SVGNumber createSVGNumber (  ) =0;

    /**
     *
     */
    virtual SVGLength createSVGLength (  ) =0;

    /**
     *
     */
    virtual SVGAngle createSVGAngle (  ) =0;

    /**
     *
     */
    virtual SVGPoint createSVGPoint (  ) =0;

    /**
     *
     */
    virtual SVGMatrix createSVGMatrix (  ) =0;

    /**
     *
     */
    virtual SVGRect createSVGRect (  ) =0;

    /**
     *
     */
    virtual SVGTransform createSVGTransform (  ) =0;

    /**
     *
     */
    virtual SVGTransform createSVGTransformFromMatrix(const SVGMatrix &matrix ) =0;

    /**
     *
     */
    virtual ElementPtr getElementById (const DOMString& elementId ) =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGSVGElement() {}

};



/*#########################################################################
## SVGGElement
#########################################################################*/

/**
 *
 */
class SVGGElement : virtual public SVGElement,
                    public SVGTests,
                    public SVGLangSpace,
                    public SVGExternalResourcesRequired,
                    public SVGStylable,
                    public SVGTransformable,
                    public events::EventTarget
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGGElement() {}

};




/*#########################################################################
## SVGDefsElement
#########################################################################*/

/**
 *
 */
class SVGDefsElement :
                    virtual public SVGElement,
                    public SVGTests,
                    public SVGLangSpace,
                    public SVGExternalResourcesRequired,
                    public SVGStylable,
                    public SVGTransformable,
                    public events::EventTarget
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGDefsElement() {}

};




/*#########################################################################
## SVGDescElement
#########################################################################*/

/**
 *
 */
class SVGDescElement :
                    virtual public SVGElement,
                    public SVGLangSpace,
                    public SVGStylable
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGDescElement() {}

};




/*#########################################################################
## SVGTitleElement
#########################################################################*/

/**
 *
 */
class SVGTitleElement :
                    virtual public SVGElement,
                    public SVGLangSpace,
                    public SVGStylable
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGTitleElement() {}

};




/*#########################################################################
## SVGSymbolElement
#########################################################################*/

/**
 *
 */
class SVGSymbolElement :
                    virtual public SVGElement,
                    public SVGLangSpace,
                    public SVGExternalResourcesRequired,
                    public SVGStylable,
                    public SVGFitToViewBox,
                    public events::EventTarget
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGSymbolElement() {}

};




/*#########################################################################
## SVGUseElement
#########################################################################*/

/**
 *
 */
class SVGUseElement :
                    virtual public SVGElement,
                    public SVGURIReference,
                    public SVGTests,
                    public SVGLangSpace,
                    public SVGExternalResourcesRequired,
                    public SVGStylable,
                    public SVGTransformable,
                    public events::EventTarget
{
public:




    /**
     *
     */
    virtual SVGAnimatedLength getX() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getY() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getWidth() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getHeight() =0;

    /**
     *
     */
    virtual SVGElementInstance getInstanceRoot() =0;

    /**
     *
     */
    virtual SVGElementInstance getAnimatedInstanceRoot() =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGUseElement() {}

};







/*#########################################################################
## SVGImageElement
#########################################################################*/

/**
 *
 */
class SVGImageElement :
                    virtual public SVGElement,
                    public SVGURIReference,
                    public SVGTests,
                    public SVGLangSpace,
                    public SVGExternalResourcesRequired,
                    public SVGStylable,
                    public SVGTransformable,
                    public events::EventTarget
{
public:


    /**
     *
     */
    virtual SVGAnimatedLength getX() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getY() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getWidth() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getHeight() =0;


    /**
     *
     */
    virtual SVGAnimatedPreserveAspectRatio getPreserveAspectRatio() =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGImageElement() {}

};






/*#########################################################################
## SVGSwitchElement
#########################################################################*/

/**
 *
 */
class SVGSwitchElement :
                    virtual public SVGElement,
                    public SVGTests,
                    public SVGLangSpace,
                    public SVGExternalResourcesRequired,
                    public SVGStylable,
                    public SVGTransformable,
                    public events::EventTarget
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGSwitchElement() {}

};




/*#########################################################################
## GetSVGDocument
#########################################################################*/

/**
 *
 */
class GetSVGDocument
{
public:

    /**
     *
     */
    virtual SVGDocumentPtr getSVGDocument (  )
                    throw( DOMException ) =0;

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~GetSVGDocument() {}

};






/*#########################################################################
## SVGStyleElement
#########################################################################*/

/**
 *
 */
class SVGStyleElement : virtual public SVGElement
{
public:

    /**
     *
     */
    virtual DOMString getXmlspace() = 0;

    /**
     *
     */
    virtual void setXmlspace(const DOMString &val)
                             throw (DOMException) =0;

    /**
     *
     */
    virtual DOMString getType() = 0;

    /**
     *
     */
    virtual void setType(const DOMString &val)
                         throw (DOMException) =0;

    /**
     *
     */
    virtual DOMString getMedia() = 0;

    /**
     *
     */
    virtual void setMedia(const DOMString &val)
                          throw (DOMException) =0;

    /**
     *
     */
    virtual DOMString getTitle() = 0;

    /**
     *
     */
    virtual void setTitle(const DOMString &val)
                          throw (DOMException) =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGStyleElement() {}

};






/*#########################################################################
## SVGPathElement
#########################################################################*/

/**
 *
 */
class SVGPathElement :
                    virtual public SVGElement,
                    public SVGTests,
                    public SVGLangSpace,
                    public SVGExternalResourcesRequired,
                    public SVGStylable,
                    public SVGTransformable,
                    public events::EventTarget,
                    public SVGAnimatedPathData
{
public:




    /**
     *
     */
    virtual SVGAnimatedNumber getPathLength() =0;

    /**
     *
     */
    virtual double getTotalLength (  ) =0;

    /**
     *
     */
    virtual SVGPoint getPointAtLength (double distance ) =0;

    /**
     *
     */
    virtual unsigned long getPathSegAtLength (double distance ) =0;

    /**
     *
     */
    virtual SVGPathSegClosePath
              createSVGPathSegClosePath (  ) =0;

    /**
     *
     */
    virtual SVGPathSegMovetoAbs
              createSVGPathSegMovetoAbs (double x, double y ) =0;

    /**
     *
     */
    virtual SVGPathSegMovetoRel
              createSVGPathSegMovetoRel (double x, double y ) =0;

    /**
     *
     */
    virtual SVGPathSegLinetoAbs
              createSVGPathSegLinetoAbs (double x, double y ) =0;

    /**
     *
     */
    virtual SVGPathSegLinetoRel
              createSVGPathSegLinetoRel (double x, double y ) =0;

    /**
     *
     */
    virtual SVGPathSegCurvetoCubicAbs
              createSVGPathSegCurvetoCubicAbs (double x, double y,
                        double x1, double y1, double x2, double y2 ) =0;

    /**
     *
     */
    virtual SVGPathSegCurvetoCubicRel
              createSVGPathSegCurvetoCubicRel (double x, double y,
                        double x1, double y1, double x2, double y2 ) =0;

    /**
     *
     */
    virtual SVGPathSegCurvetoQuadraticAbs
              createSVGPathSegCurvetoQuadraticAbs (double x, double y,
                         double x1, double y1 ) =0;

    /**
     *
     */
    virtual SVGPathSegCurvetoQuadraticRel
              createSVGPathSegCurvetoQuadraticRel (double x, double y,
                         double x1, double y1 ) =0;

    /**
     *
     */
    virtual SVGPathSegArcAbs
              createSVGPathSegArcAbs (double x, double y,
                         double r1, double r2, double angle,
                         bool largeArcFlag, bool sweepFlag ) =0;

    /**
     *
     */
    virtual SVGPathSegArcRel
              createSVGPathSegArcRel (double x, double y, double r1,
                         double r2, double angle, bool largeArcFlag,
                         bool sweepFlag ) =0;

    /**
     *
     */
    virtual SVGPathSegLinetoHorizontalAbs
              createSVGPathSegLinetoHorizontalAbs (double x ) =0;

    /**
     *
     */
    virtual SVGPathSegLinetoHorizontalRel
              createSVGPathSegLinetoHorizontalRel (double x ) =0;

    /**
     *
     */
    virtual SVGPathSegLinetoVerticalAbs
              createSVGPathSegLinetoVerticalAbs (double y ) =0;

    /**
     *
     */
    virtual SVGPathSegLinetoVerticalRel
              createSVGPathSegLinetoVerticalRel (double y ) =0;

    /**
     *
     */
    virtual SVGPathSegCurvetoCubicSmoothAbs
              createSVGPathSegCurvetoCubicSmoothAbs (double x, double y,
                                             double x2, double y2 ) =0;

    /**
     *
     */
    virtual SVGPathSegCurvetoCubicSmoothRel
              createSVGPathSegCurvetoCubicSmoothRel (double x, double y,
                                                      double x2, double y2 ) =0;

    /**
     *
     */
    virtual SVGPathSegCurvetoQuadraticSmoothAbs
              createSVGPathSegCurvetoQuadraticSmoothAbs (double x, double y ) =0;

    /**
     *
     */
    virtual SVGPathSegCurvetoQuadraticSmoothRel
              createSVGPathSegCurvetoQuadraticSmoothRel (double x, double y ) =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGPathElement() {}

};






/*#########################################################################
## SVGRectElement
#########################################################################*/

/**
 *
 */
class SVGRectElement :
                    virtual public SVGElement,
                    public SVGTests,
                    public SVGLangSpace,
                    public SVGExternalResourcesRequired,
                    public SVGStylable,
                    public SVGTransformable,
                    public events::EventTarget
{
public:

    /**
     *
     */
    virtual SVGAnimatedLength getX() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getY() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getWidth() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getHeight() =0;


    /**
     *
     */
    virtual SVGAnimatedLength getRx() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getRy() =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGRectElement() {}

};






/*#########################################################################
## SVGCircleElement
#########################################################################*/

/**
 *
 */
class SVGCircleElement :
                    virtual public SVGElement,
                    public SVGTests,
                    public SVGLangSpace,
                    public SVGExternalResourcesRequired,
                    public SVGStylable,
                    public SVGTransformable,
                    public events::EventTarget
{
public:

    /**
     *
     */
    virtual SVGAnimatedLength getCx() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getCy() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getR() =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGCircleElement() {}

};






/*#########################################################################
## SVGEllipseElement
#########################################################################*/

/**
 *
 */
class SVGEllipseElement :
                    virtual public SVGElement,
                    public SVGTests,
                    public SVGLangSpace,
                    public SVGExternalResourcesRequired,
                    public SVGStylable,
                    public SVGTransformable,
                    public events::EventTarget
{
public:
    /**
     *
     */
    virtual SVGAnimatedLength getCx() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getCy() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getRx() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getRy() =0;


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGEllipseElement() {}

};






/*#########################################################################
## SVGLineElement
#########################################################################*/

/**
 *
 */
class SVGLineElement :
                    virtual public SVGElement,
                    public SVGTests,
                    public SVGLangSpace,
                    public SVGExternalResourcesRequired,
                    public SVGStylable,
                    public SVGTransformable,
                    public events::EventTarget
{
public:
    /**
     *
     */
    virtual SVGAnimatedLength getX1() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getY1() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getX2() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getY2() =0;

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGLineElement() {}

};




/*#########################################################################
## SVGPolylineElement
#########################################################################*/

/**
 *
 */
class SVGPolylineElement :
                    virtual public SVGElement,
                    public SVGTests,
                    public SVGLangSpace,
                    public SVGExternalResourcesRequired,
                    public SVGStylable,
                    public SVGTransformable,
                    public events::EventTarget,
                    public SVGAnimatedPoints
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGPolylineElement() {}

};




/*#########################################################################
## SVGPolygonElement
#########################################################################*/

/**
 *
 */
class SVGPolygonElement :
                    virtual public SVGElement,
                    public SVGTests,
                    public SVGLangSpace,
                    public SVGExternalResourcesRequired,
                    public SVGStylable,
                    public SVGTransformable,
                    public events::EventTarget,
                    public SVGAnimatedPoints
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGPolygonElement() {}

};




/*#########################################################################
## SVGTextContentElement
#########################################################################*/

/**
 *
 */
class SVGTextContentElement :
                    virtual public SVGElement,
                    public SVGTests,
                    public SVGLangSpace,
                    public SVGExternalResourcesRequired,
                    public SVGStylable,
                    public events::EventTarget
{
public:



    /**
     * lengthAdjust Types
     */
    typedef enum
        {
        LENGTHADJUST_UNKNOWN          = 0,
        LENGTHADJUST_SPACING          = 1,
        LENGTHADJUST_SPACINGANDGLYPHS = 2
        } LengthAdjustType;


    /**
     *
     */
    virtual SVGAnimatedLength getTextLength() =0;


    /**
     *
     */
    virtual SVGAnimatedEnumeration getLengthAdjust() =0;


    /**
     *
     */
    virtual long getNumberOfChars (  ) =0;

    /**
     *
     */
    virtual double getComputedTextLength (  ) =0;

    /**
     *
     */
    virtual double getSubStringLength (unsigned long charnum, unsigned long nchars )
                                     throw( DOMException ) =0;

    /**
     *
     */
    virtual SVGPoint getStartPositionOfChar (unsigned long charnum )
                                              throw( DOMException ) =0;

    /**
     *
     */
    virtual SVGPoint getEndPositionOfChar (unsigned long charnum )
                                           throw( DOMException ) =0;

    /**
     *
     */
    virtual SVGRect getExtentOfChar (unsigned long charnum )
                                      throw( DOMException ) =0;

    /**
     *
     */
    virtual double getRotationOfChar (unsigned long charnum )
                                     throw( DOMException ) =0;

    /**
     *
     */
    virtual long getCharNumAtPosition (const SVGPoint &point ) =0;

    /**
     *
     */
    virtual void selectSubString (unsigned long charnum, unsigned long nchars )
                                  throw( DOMException ) =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGTextContentElement() {}

};






/*#########################################################################
## SVGTextPositioningElement
#########################################################################*/

/**
 *
 */
class SVGTextPositioningElement : virtual public SVGTextContentElement
{
public:

    /**
     *
     */
    virtual SVGAnimatedLength getX() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getY() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getDx() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getDy() =0;


    /**
     *
     */
    virtual SVGAnimatedNumberList getRotate() =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGTextPositioningElement() {}

};






/*#########################################################################
## SVGTextElement
#########################################################################*/

/**
 *
 */
class SVGTextElement : virtual public SVGTextPositioningElement,
                       public SVGTransformable
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGTextElement() {}

};




/*#########################################################################
## SVGTSpanElement
#########################################################################*/

/**
 *
 */
class SVGTSpanElement : virtual public SVGTextPositioningElement
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGTSpanElement() {}

};




/*#########################################################################
## SVGTRefElement
#########################################################################*/

/**
 *
 */
class SVGTRefElement :
                    virtual public SVGTextPositioningElement,
                    public SVGURIReference
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGTRefElement() {}

};




/*#########################################################################
## SVGTextPathElement
#########################################################################*/

/**
 *
 */
class SVGTextPathElement :
                    virtual public SVGTextContentElement,
                    public SVGURIReference
{
public:



    /**
     * textPath Method Types
     */
    typedef enum
        {
        TEXTPATH_METHODTYPE_UNKNOWN   = 0,
        TEXTPATH_METHODTYPE_ALIGN     = 1,
        TEXTPATH_METHODTYPE_STRETCH   = 2
        } TextPathMethodType;

    /**
     * textPath Spacing Types
     */
    typedef enum
        {
        TEXTPATH_SPACINGTYPE_UNKNOWN  = 0,
        TEXTPATH_SPACINGTYPE_AUTO     = 1,
        TEXTPATH_SPACINGTYPE_EXACT    = 2
        } TextPathSpacingType;


    /**
     *
     */
    virtual SVGAnimatedLength getStartOffset() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration getMethod() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration getSpacing() =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGTextPathElement() {}

};






/*#########################################################################
## SVGAltGlyphElement
#########################################################################*/

/**
 *
 */
class SVGAltGlyphElement :
                    virtual public SVGTextPositioningElement,
                    public SVGURIReference
{
public:

    /**
     *
     */
    virtual DOMString getGlyphRef() =0;

    /**
     *
     */
    virtual void setGlyphRef(const DOMString &val)
                                     throw (DOMException) =0;

    /**
     *
     */
    virtual DOMString getFormat() =0;

    /**
     *
     */
    virtual void setFormat(const DOMString &val)
                                     throw (DOMException) =0;




    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGAltGlyphElement() {}

};






/*#########################################################################
## SVGAltGlyphDefElement
#########################################################################*/

/**
 *
 */
class SVGAltGlyphDefElement : virtual public SVGElement
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGAltGlyphDefElement() {}

};




/*#########################################################################
## SVGAltGlyphItemElement
#########################################################################*/

/**
 *
 */
class SVGAltGlyphItemElement : virtual public SVGElement
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGAltGlyphItemElement() {}

};




/*#########################################################################
## SVGGlyphRefElement
#########################################################################*/

/**
 *
 */
class SVGGlyphRefElement : virtual public SVGElement,
                           public SVGURIReference,
                           public SVGStylable
{
public:
    /**
     *
     */
    virtual DOMString getGlyphRef() =0;

    /**
     *
     */
    virtual void setGlyphRef(const DOMString &val)
                             throw (DOMException) =0;

    /**
     *
     */
    virtual DOMString getFormat() =0;

    /**
     *
     */
    virtual void setFormat(const DOMString &val)
                           throw (DOMException) =0;

    /**
     *
     */
    virtual double getX() = 0;

    /**
     *
     */
    virtual void setX(double val) throw (DOMException) =0;

    /**
     *
     */
    virtual double getY() = 0;

    /**
     *
     */
    virtual void setY(double val) throw (DOMException) =0;

    /**
     *
     */
    virtual double getDx() = 0;

    /**
     *
     */
    virtual void setDx(double val) throw (DOMException) =0;

    /**
     *
     */
    virtual double getDy() = 0;

    /**
     *
     */
    virtual void setDy(double val) throw (DOMException) =0;




    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGGlyphRefElement() {}

};





/*#########################################################################
## SVGMarkerElement
#########################################################################*/

/**
 *
 */
class SVGMarkerElement :
                    virtual public SVGElement,
                    public SVGLangSpace,
                    public SVGExternalResourcesRequired,
                    public SVGStylable,
                    public SVGFitToViewBox
{
public:



    /**
     * Marker Unit Types
     */
    typedef enum
        {
        SVG_MARKERUNITS_UNKNOWN        = 0,
        SVG_MARKERUNITS_USERSPACEONUSE = 1,
        SVG_MARKERUNITS_STROKEWIDTH    = 2
        } MarkerUnitType;

    /**
     * Marker Orientation Types
     */
    typedef enum
        {
        SVG_MARKER_ORIENT_UNKNOWN      = 0,
        SVG_MARKER_ORIENT_AUTO         = 1,
        SVG_MARKER_ORIENT_ANGLE        = 2
        } MarkerOrientationType;


    /**
     *
     */
    virtual SVGAnimatedLength getRefX() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getRefY() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration getMarkerUnits() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getMarkerWidth() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getMarkerHeight() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration getOrientType() =0;

    /**
     *
     */
    virtual SVGAnimatedAngle getOrientAngle() =0;


    /**
     *
     */
    virtual void setOrientToAuto (  ) =0;

    /**
     *
     */
    virtual void setOrientToAngle (const SVGAngle &angle) =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGMarkerElement() {}

};






/*#########################################################################
## SVGColorProfileElement
#########################################################################*/

/**
 *
 */
class SVGColorProfileElement :
                    virtual public SVGElement,
                    public SVGURIReference,
                    public SVGRenderingIntent
{
public:
    /**
     *
     */
    virtual DOMString getLocal() =0;

    /**
     *
     */
    virtual void setLocal(const DOMString &val)
                          throw (DOMException) =0;

    /**
     *
     */
    virtual DOMString getName() =0;

    /**
     *
     */
    virtual void setName(const DOMString &val)
                         throw (DOMException) =0;

    /**
     *
     */
    virtual unsigned short getRenderingIntent() =0;

    /**
     *
     */
    virtual void setRenderingIntent(unsigned short val)
                                    throw (DOMException) =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGColorProfileElement() {}

};




/*#########################################################################
## SVGGradientElement
#########################################################################*/

/**
 *
 */
class SVGGradientElement :
                    virtual public SVGElement,
                    public SVGURIReference,
                    public SVGExternalResourcesRequired,
                    public SVGStylable,
                    public SVGUnitTypes
{
public:



    /**
     * Spread Method Types
     */
    typedef enum
        {
        SVG_SPREADMETHOD_UNKNOWN = 0,
        SVG_SPREADMETHOD_PAD     = 1,
        SVG_SPREADMETHOD_REFLECT = 2,
        SVG_SPREADMETHOD_REPEAT  = 3
        } SpreadMethodType;


    /**
     *
     */
    virtual SVGAnimatedEnumeration getGradientUnits() =0;

    /**
     *
     */
    virtual SVGAnimatedTransformList getGradientTransform() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration getSpreadMethod() =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGGradientElement() {}

};






/*#########################################################################
## SVGLinearGradientElement
#########################################################################*/

/**
 *
 */
class SVGLinearGradientElement : virtual public SVGGradientElement
{
public:


    /**
     *
     */
    virtual SVGAnimatedLength getX1() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getY1() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getX2() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getY2() =0;


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGLinearGradientElement() {}

};






/*#########################################################################
## SVGRadialGradientElement
#########################################################################*/

/**
 *
 */
class SVGRadialGradientElement : virtual public SVGGradientElement
{
public:


    /**
     *
     */
    virtual SVGAnimatedLength getCx() =0;


    /**
     *
     */
    virtual SVGAnimatedLength getCy() =0;


    /**
     *
     */
    virtual SVGAnimatedLength getR() =0;


    /**
     *
     */
    virtual SVGAnimatedLength getFx() =0;


    /**
     *
     */
    virtual SVGAnimatedLength getFy() =0;




    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGRadialGradientElement() {}

};






/*#########################################################################
## SVGStopElement
#########################################################################*/

/**
 *
 */
class SVGStopElement :
                    virtual public SVGElement,
                    public SVGStylable
{
public:

    /**
     *
     */
    virtual SVGAnimatedNumber getOffset() =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGStopElement() {}

};






/*#########################################################################
## SVGPatternElement
#########################################################################*/

/**
 *
 */
class SVGPatternElement :
                    virtual public SVGElement,
                    public SVGURIReference,
                    public SVGTests,
                    public SVGLangSpace,
                    public SVGExternalResourcesRequired,
                    public SVGStylable,
                    public SVGFitToViewBox,
                    public SVGUnitTypes
{
public:




    /**
     *
     */
    virtual SVGAnimatedEnumeration getPatternUnits() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration getPatternContentUnits() =0;

    /**
     *
     */
    virtual SVGAnimatedTransformList getPatternTransform() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getX() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getY() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getWidth() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getHeight() =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGPatternElement() {}

};






/*#########################################################################
## SVGClipPathElement
#########################################################################*/

/**
 *
 */
class SVGClipPathElement :
                    virtual public SVGElement,
                    public SVGTests,
                    public SVGLangSpace,
                    public SVGExternalResourcesRequired,
                    public SVGStylable,
                    public SVGTransformable,
                    public SVGUnitTypes
{
public:
    /**
     *
     */
    virtual SVGAnimatedEnumeration getClipPathUnits() =0;




    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGClipPathElement() {}

};






/*#########################################################################
## SVGMaskElement
#########################################################################*/

/**
 *
 */
class SVGMaskElement :
                    virtual public SVGElement,
                    public SVGTests,
                    public SVGLangSpace,
                    public SVGExternalResourcesRequired,
                    public SVGStylable,
                    public SVGUnitTypes
{
public:



    /**
     *
     */
    virtual SVGAnimatedEnumeration getMaskUnits() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration getMaskContentUnits() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getX() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getY() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getWidth() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getHeight() =0;

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGMaskElement() {}

};






/*#########################################################################
## SVGFilterElement
#########################################################################*/

/**
 *
 */
class SVGFilterElement :
                    virtual public SVGElement,
                    public SVGURIReference,
                    public SVGLangSpace,
                    public SVGExternalResourcesRequired,
                    public SVGStylable,
                    public SVGUnitTypes
{
public:



    /**
     *
     */
    virtual SVGAnimatedEnumeration getFilterUnits() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration getPrimitiveUnits() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getX() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getY() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getWidth() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getHeight() =0;


    /**
     *
     */
    virtual SVGAnimatedInteger getFilterResX() =0;

    /**
     *
     */
    virtual SVGAnimatedInteger getFilterResY() =0;

    /**
     *
     */
    virtual void setFilterRes (unsigned long filterResX,
                               unsigned long filterResY ) =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFilterElement() {}

};





/*#########################################################################
## SVGFEBlendElement
#########################################################################*/

/**
 *
 */
class SVGFEBlendElement :
                    virtual public SVGElement,
                    public SVGFilterPrimitiveStandardAttributes
{
public:


    /**
     * Blend Mode Types
     */
    typedef enum
        {
        SVG_FEBLEND_MODE_UNKNOWN  = 0,
        SVG_FEBLEND_MODE_NORMAL   = 1,
        SVG_FEBLEND_MODE_MULTIPLY = 2,
        SVG_FEBLEND_MODE_SCREEN   = 3,
        SVG_FEBLEND_MODE_DARKEN   = 4,
        SVG_FEBLEND_MODE_LIGHTEN  = 5
        } BlendModeType;

    /**
     *
     */
    virtual SVGAnimatedString getIn1() =0;

    /**
     *
     */
    virtual SVGAnimatedString getIn2() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration getMode() =0;


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEBlendElement() {}

};






/*#########################################################################
## SVGFEColorMatrixElement
#########################################################################*/

/**
 *
 */
class SVGFEColorMatrixElement :
                    virtual public SVGElement,
                    public SVGFilterPrimitiveStandardAttributes
{
public:



    /**
     * Color Matrix Types
     */
    typedef enum
        {
        SVG_FECOLORMATRIX_TYPE_UNKNOWN          = 0,
        SVG_FECOLORMATRIX_TYPE_MATRIX           = 1,
        SVG_FECOLORMATRIX_TYPE_SATURATE         = 2,
        SVG_FECOLORMATRIX_TYPE_HUEROTATE        = 3,
        SVG_FECOLORMATRIX_TYPE_LUMINANCETOALPHA = 4
        } ColorMatrixType;


    /**
     *
     */
    virtual SVGAnimatedString getIn1() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration getType() =0;

    /**
     *
     */
    virtual SVGAnimatedNumberList getValues() =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEColorMatrixElement() {}

};






/*#########################################################################
## SVGFEComponentTransferElement
#########################################################################*/

/**
 *
 */
class SVGFEComponentTransferElement :
                    virtual public SVGElement,
                    public SVGFilterPrimitiveStandardAttributes
{
public:
    /**
     *
     */
    virtual SVGAnimatedString getIn1() =0;

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEComponentTransferElement() {}

};






/*#########################################################################
## SVGComponentTransferFunctionElement
#########################################################################*/

/**
 *
 */
class SVGComponentTransferFunctionElement : virtual public SVGElement
{
public:


    /**
     * Component Transfer Types
     */
    typedef enum
        {
        SVG_FECOMPONENTTRANSFER_TYPE_UNKNOWN  = 0,
        SVG_FECOMPONENTTRANSFER_TYPE_IDENTITY = 1,
        SVG_FECOMPONENTTRANSFER_TYPE_TABLE    = 2,
        SVG_FECOMPONENTTRANSFER_TYPE_DISCRETE = 3,
        SVG_FECOMPONENTTRANSFER_TYPE_LINEAR   = 4,
        SVG_FECOMPONENTTRANSFER_TYPE_GAMMA    = 5
        } ComponentTransferType;


    /**
     *
     */
    virtual SVGAnimatedEnumeration getType() =0;

    /**
     *
     */
    virtual SVGAnimatedNumberList getTableValues() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber getSlope() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber getIntercept() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber getAmplitude() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber getExponent() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber getOffset() =0;


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGComponentTransferFunctionElement() {}

};






/*#########################################################################
## SVGFEFuncRElement
#########################################################################*/

/**
 *
 */
class SVGFEFuncRElement : virtual public SVGComponentTransferFunctionElement
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEFuncRElement() {}

};




/*#########################################################################
## SVGFEFuncGElement
#########################################################################*/

/**
 *
 */
class SVGFEFuncGElement : public virtual SVGComponentTransferFunctionElement
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEFuncGElement() {}

};




/*#########################################################################
## SVGFEFuncBElement
#########################################################################*/

/**
 *
 */
class SVGFEFuncBElement : virtual public SVGComponentTransferFunctionElement
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEFuncBElement() {}

};




/*#########################################################################
## SVGFEFuncAElement
#########################################################################*/

/**
 *
 */
class SVGFEFuncAElement : virtual public SVGComponentTransferFunctionElement
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEFuncAElement() {}

};




/*#########################################################################
## SVGFECompositeElement
#########################################################################*/

/**
 *
 */
class SVGFECompositeElement :
                    virtual public SVGElement,
                    public SVGFilterPrimitiveStandardAttributes
{
public:



    /**
     *  Composite Operators
     */
    typedef enum
        {
        SVG_FECOMPOSITE_OPERATOR_UNKNOWN    = 0,
        SVG_FECOMPOSITE_OPERATOR_OVER       = 1,
        SVG_FECOMPOSITE_OPERATOR_IN         = 2,
        SVG_FECOMPOSITE_OPERATOR_OUT        = 3,
        SVG_FECOMPOSITE_OPERATOR_ATOP       = 4,
        SVG_FECOMPOSITE_OPERATOR_XOR        = 5,
        SVG_FECOMPOSITE_OPERATOR_ARITHMETIC = 6
        } CompositeOperatorType;

    /**
     *
     */
    virtual SVGAnimatedString getIn1() =0;

    /**
     *
     */
    virtual SVGAnimatedString getIn2() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration getOperator() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber getK1() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber getK2() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber getK3() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber getK4() =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFECompositeElement() {}

};






/*#########################################################################
## SVGFEConvolveMatrixElement
#########################################################################*/

/**
 *
 */
class SVGFEConvolveMatrixElement :
                    virtual public SVGElement,
                    public SVGFilterPrimitiveStandardAttributes
{
public:



    /**
     * Edge Mode Values
     */
    typedef enum
        {
        SVG_EDGEMODE_UNKNOWN   = 0,
        SVG_EDGEMODE_DUPLICATE = 1,
        SVG_EDGEMODE_WRAP      = 2,
        SVG_EDGEMODE_NONE      = 3
        } EdgeModeType;


    /**
     *
     */
    virtual SVGAnimatedInteger getOrderX() =0;

    /**
     *
     */
    virtual SVGAnimatedInteger getOrderY() =0;

    /**
     *
     */
    virtual SVGAnimatedNumberList getKernelMatrix() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber getDivisor() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber getBias() =0;

    /**
     *
     */
    virtual SVGAnimatedInteger getTargetX() =0;

    /**
     *
     */
    virtual SVGAnimatedInteger getTargetY() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration getEdgeMode() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getKernelUnitLengthX() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getKernelUnitLengthY() =0;

    /**
     *
     */
    virtual SVGAnimatedBoolean getPreserveAlpha() =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEConvolveMatrixElement() {}

};






/*#########################################################################
## SVGFEDiffuseLightingElement
#########################################################################*/

/**
 *
 */
class SVGFEDiffuseLightingElement :
                    virtual public SVGElement,
                    public SVGFilterPrimitiveStandardAttributes
{
public:

    /**
     *
     */
    virtual SVGAnimatedString getIn1() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber getSurfaceScale() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber getDiffuseConstant() =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEDiffuseLightingElement() {}

};






/*#########################################################################
## SVGFEDistantLightElement
#########################################################################*/

/**
 *
 */
class SVGFEDistantLightElement : virtual public SVGElement
{
public:

    /**
     *
     */
    virtual SVGAnimatedNumber getAzimuth() =0;


    /**
     *
     */
    virtual SVGAnimatedNumber getElevation() =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEDistantLightElement() {}

};






/*#########################################################################
## SVGFEPointLightElement
#########################################################################*/

/**
 *
 */
class SVGFEPointLightElement : virtual public SVGElement
{
public:
    /**
     *
     */
    virtual SVGAnimatedNumber getX() =0;


    /**
     *
     */
    virtual SVGAnimatedNumber getY() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber getZ() =0;

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEPointLightElement() {}

};






/*#########################################################################
## SVGFESpotLightElement
#########################################################################*/

/**
 *
 */
class SVGFESpotLightElement : virtual public SVGElement
{
public:

    /**
     *
     */
    virtual SVGAnimatedNumber getX() =0;


    /**
     *
     */
    virtual SVGAnimatedNumber getY() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber getZ() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber getPointsAtX() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber getPointsAtY() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber getPointsAtZ() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber getSpecularExponent() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber getLimitingConeAngle() =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFESpotLightElement() {}

};






/*#########################################################################
## SVGFEDisplacementMapElement
#########################################################################*/

/**
 *
 */
class SVGFEDisplacementMapElement :
                    virtual public SVGElement,
                    public SVGFilterPrimitiveStandardAttributes
{
public:



    /**
     *  Channel Selectors
     */
    typedef enum
        {
        SVG_CHANNEL_UNKNOWN = 0,
        SVG_CHANNEL_R       = 1,
        SVG_CHANNEL_G       = 2,
        SVG_CHANNEL_B       = 3,
        SVG_CHANNEL_A       = 4
        } ChannelSelector;

    /**
     *
     */
    virtual SVGAnimatedString getIn1() =0;

    /**
     *
     */
    virtual SVGAnimatedString getIn2() =0;


    /**
     *
     */
    virtual SVGAnimatedNumber getScale() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration getXChannelSelector() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration getYChannelSelector() =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEDisplacementMapElement() {}

};






/*#########################################################################
## SVGFEFloodElement
#########################################################################*/

/**
 *
 */
class SVGFEFloodElement :
                    virtual public SVGElement,
                    public SVGFilterPrimitiveStandardAttributes
{
public:
    /**
     *
     */
    virtual SVGAnimatedString getIn1() =0;


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEFloodElement() {}

};






/*#########################################################################
## SVGFEGaussianBlurElement
#########################################################################*/

/**
 *
 */
class SVGFEGaussianBlurElement :
                    virtual public SVGElement,
                    public SVGFilterPrimitiveStandardAttributes
{
public:
    /**
     *
     */
    virtual SVGAnimatedString getIn1() =0;


    /**
     *
     */
    virtual SVGAnimatedNumber getStdDeviationX() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber getStdDeviationY() =0;


    /**
     *
     */
    virtual void setStdDeviation (double stdDeviationX, double stdDeviationY ) =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEGaussianBlurElement() {}

};






/*#########################################################################
## SVGFEImageElement
#########################################################################*/

/**
 *
 */
class SVGFEImageElement :
                    virtual public SVGElement,
                    public SVGURIReference,
                    public SVGLangSpace,
                    public SVGExternalResourcesRequired,
                    public SVGFilterPrimitiveStandardAttributes
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEImageElement() {}

};




/*#########################################################################
## SVGFEMergeElement
#########################################################################*/

/**
 *
 */
class SVGFEMergeElement :
                    virtual public SVGElement,
                    public SVGFilterPrimitiveStandardAttributes
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEMergeElement() {}

};




/*#########################################################################
## SVGFEMergeNodeElement
#########################################################################*/

/**
 *
 */
class SVGFEMergeNodeElement : virtual public SVGElement
{
public:
    /**
     *
     */
    virtual SVGAnimatedString getIn1() =0;


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEMergeNodeElement() {}

};






/*#########################################################################
## SVGFEMorphologyElement
#########################################################################*/

/**
 *
 */
class SVGFEMorphologyElement :
                    virtual public SVGElement,
                    public SVGFilterPrimitiveStandardAttributes
{
public:



    /**
     *  Morphology Operators
     */
    typedef enum
        {
        SVG_MORPHOLOGY_OPERATOR_UNKNOWN = 0,
        SVG_MORPHOLOGY_OPERATOR_ERODE   = 1,
        SVG_MORPHOLOGY_OPERATOR_DILATE  = 2
        } MorphologyOperatorType;


    /**
     *
     */
    virtual SVGAnimatedString getIn1() =0;


    /**
     *
     */
    virtual SVGAnimatedEnumeration getOperator() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getRadiusX() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getRadiusY() =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEMorphologyElement() {}

};






/*#########################################################################
## SVGFEOffsetElement
#########################################################################*/

/**
 *
 */
class SVGFEOffsetElement :
                    virtual public SVGElement,
                    public SVGFilterPrimitiveStandardAttributes
{
public:



    /**
     *
     */
    virtual SVGAnimatedString getIn1() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getDx() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getDy() =0;




    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEOffsetElement() {}

};






/*#########################################################################
## SVGFESpecularLightingElement
#########################################################################*/

/**
 *
 */
class SVGFESpecularLightingElement :
                    virtual public SVGElement,
                    public SVGFilterPrimitiveStandardAttributes
{
public:

    /**
     *
     */
    virtual SVGAnimatedString getIn1() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber getSurfaceScale() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber getSpecularConstant() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber getSpecularExponent() =0;


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFESpecularLightingElement() {}

};






/*#########################################################################
## SVGFETileElement
#########################################################################*/

/**
 *
 */
class SVGFETileElement :
                    virtual public SVGElement,
                    public SVGFilterPrimitiveStandardAttributes
{
public:


    /**
     *
     */
    virtual SVGAnimatedString getIn1() =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFETileElement() {}

};






/*#########################################################################
## SVGFETurbulenceElement
#########################################################################*/

/**
 *
 */
class SVGFETurbulenceElement :
                    virtual public SVGElement,
                    public SVGFilterPrimitiveStandardAttributes
{
public:



    /**
     *  Turbulence Types
     */
    typedef enum
        {
        SVG_TURBULENCE_TYPE_UNKNOWN      = 0,
        SVG_TURBULENCE_TYPE_FRACTALNOISE = 1,
        SVG_TURBULENCE_TYPE_TURBULENCE   = 2
        } TurbulenceType;

    /**
     *  Stitch Options
     */
    typedef enum
        {
        SVG_STITCHTYPE_UNKNOWN  = 0,
        SVG_STITCHTYPE_STITCH   = 1,
        SVG_STITCHTYPE_NOSTITCH = 2
        } StitchOption;



    /**
     *
     */
    virtual SVGAnimatedNumber getBaseFrequencyX() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber getBaseFrequencyY() =0;

    /**
     *
     */
    virtual SVGAnimatedInteger getNumOctaves() =0;

    /**
     *
     */
    virtual SVGAnimatedNumber getSeed() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration getStitchTiles() =0;

    /**
     *
     */
    virtual SVGAnimatedEnumeration getType() =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFETurbulenceElement() {}

};






/*#########################################################################
## SVGCursorElement
#########################################################################*/

/**
 *
 */
class SVGCursorElement :
                    virtual public SVGElement,
                    public SVGURIReference,
                    public SVGTests,
                    public SVGExternalResourcesRequired
{
public:
    /**
     *
     */
    virtual SVGAnimatedLength getX() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getY() =0;

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGCursorElement() {}

};






/*#########################################################################
## SVGAElement
#########################################################################*/

/**
 *
 */
class SVGAElement : virtual public SVGElement,
                    public SVGURIReference,
                    public SVGTests,
                    public SVGLangSpace,
                    public SVGExternalResourcesRequired,
                    public SVGStylable,
                    public SVGTransformable,
                    public events::EventTarget
{
public:

    /**
     *
     */
    virtual SVGAnimatedString getTarget() =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGAElement() {}

};






/*#########################################################################
## SVGViewElement
#########################################################################*/

/**
 *
 */
class SVGViewElement : virtual public SVGElement,
                       public SVGExternalResourcesRequired,
                       public SVGFitToViewBox,
                       public SVGZoomAndPan
{
public:

    /**
     *
     */
    virtual SVGStringList getViewTarget() =0;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGViewElement() {}

};






/*#########################################################################
## SVGScriptElement
#########################################################################*/

/**
 *
 */
class SVGScriptElement :
                    virtual public SVGElement,
                    public SVGURIReference,
                    public SVGExternalResourcesRequired
{
public:

    /**
     *
     */
    virtual DOMString getType() =0;

    /**
     *
     */
    virtual void setType(const DOMString &val)
                               throw (DOMException) =0;


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGScriptElement() {}

};





/*#########################################################################
## SVGAnimationElement
#########################################################################*/

/**
 *
 */
class SVGAnimationElement :
                    virtual public SVGElement,
                    public SVGTests,
                    public SVGExternalResourcesRequired,
                    public smil::ElementTimeControl,
                    public events::EventTarget
{
public:


    /**
     *
     */
    virtual SVGElementPtr getTargetElement() =0;


    /**
     *
     */
    virtual double getStartTime (  ) =0;

    /**
     *
     */
    virtual double getCurrentTime (  ) =0;

    /**
     *
     */
    virtual double getSimpleDuration (  )
                    throw( DOMException ) =0;
;



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGAnimationElement() {}

};






/*#########################################################################
## SVGAnimateElement
#########################################################################*/

/**
 *
 */
class SVGAnimateElement : virtual public SVGAnimationElement
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGAnimateElement() {}

};




/*#########################################################################
## SVGSetElement
#########################################################################*/

/**
 *
 */
class SVGSetElement : virtual public SVGAnimationElement
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGSetElement() {}

};




/*#########################################################################
## SVGAnimateMotionElement
#########################################################################*/

/**
 *
 */
class SVGAnimateMotionElement : virtual public SVGAnimationElement
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGAnimateMotionElement() {}

};




/*#########################################################################
## SVGMPathElement
#########################################################################*/

/**
 *
 */
class SVGMPathElement :
                    virtual public SVGElement,
                    public SVGURIReference,
                    public SVGExternalResourcesRequired
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGMPathElement() {}

};




/*#########################################################################
## SVGAnimateColorElement
#########################################################################*/

/**
 *
 */
class SVGAnimateColorElement : virtual public SVGAnimationElement
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGAnimateColorElement() {}

};




/*#########################################################################
## SVGAnimateTransformElement
#########################################################################*/

/**
 *
 */
class SVGAnimateTransformElement : virtual public SVGAnimationElement
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGAnimateTransformElement() {}

};




/*#########################################################################
## SVGFontElement
#########################################################################*/

/**
 *
 */
class SVGFontElement :  virtual public SVGElement,
                        public SVGExternalResourcesRequired,
                        public SVGStylable
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFontElement() {}

};




/*#########################################################################
## SVGGlyphElement
#########################################################################*/

/**
 *
 */
class SVGGlyphElement :  virtual public SVGElement,
                         public SVGStylable
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGGlyphElement() {}

};




/*#########################################################################
## SVGMissingGlyphElement
#########################################################################*/

/**
 *
 */
class SVGMissingGlyphElement :
                    virtual public SVGElement,
                    public SVGStylable
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGMissingGlyphElement() {}

};




/*#########################################################################
## SVGHKernElement
#########################################################################*/

/**
 *
 */
class SVGHKernElement : virtual public SVGElement
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGHKernElement() {}

};




/*#########################################################################
## SVGVKernElement
#########################################################################*/

/**
 *
 */
class SVGVKernElement : public virtual SVGElement
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGVKernElement() {}

};




/*#########################################################################
## SVGFontFaceElement
#########################################################################*/

/**
 *
 */
class SVGFontFaceElement : virtual public SVGElement
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFontFaceElement() {}

};




/*#########################################################################
## SVGFontFaceSrcElement
#########################################################################*/

/**
 *
 */
class SVGFontFaceSrcElement : virtual public SVGElement
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFontFaceSrcElement() {}

};




/*#########################################################################
## SVGFontFaceUriElement
#########################################################################*/

/**
 *
 */
class SVGFontFaceUriElement : virtual public SVGElement
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFontFaceUriElement() {}

};




/*#########################################################################
## SVGFontFaceFormatElement
#########################################################################*/

/**
 *
 */
class SVGFontFaceFormatElement : virtual public SVGElement
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFontFaceFormatElement() {}

};




/*#########################################################################
## SVGFontFaceNameElement
#########################################################################*/

/**
 *
 */
class SVGFontFaceNameElement : virtual public SVGElement
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFontFaceNameElement() {}

};




/*#########################################################################
## SVGDefinitionSrcElement
#########################################################################*/

/**
 *
 */
class SVGDefinitionSrcElement : virtual public SVGElement
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGDefinitionSrcElement() {}

};




/*#########################################################################
## SVGMetadataElement
#########################################################################*/

/**
 *
 */
class SVGMetadataElement : virtual public SVGElement
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGMetadataElement() {}

};



/*#########################################################################
## SVGForeignObjectElement
#########################################################################*/

/**
 *
 */
class SVGForeignObjectElement :
                    virtual public SVGElement,
                    public SVGTests,
                    public SVGLangSpace,
                    public SVGExternalResourcesRequired,
                    public SVGStylable,
                    public SVGTransformable,
                    public events::EventTarget
{
public:


    /**
     *
     */
    virtual SVGAnimatedLength getX() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getY() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getWidth() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getHeight() =0;



    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    virtual ~SVGForeignObjectElement() {}

};





}  //namespace svg
}  //namespace dom
}  //namespace w3c
}  //namespace org

#endif // __SVG_H__
/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/

