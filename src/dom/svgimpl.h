#ifndef __SVGIMPL_H__
#define __SVGIMPL_H__

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
 * Copyright (C) 2006-2008 Bob Jamison
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


#include "svg.h"
#include "dom/domimpl.h"
#include "dom/smilimpl.h"

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
typedef dom::Document Document;
typedef dom::NodeList NodeList;



class SVGSVGElementImpl;
typedef Ptr<SVGSVGElementImpl> SVGSVGElementImplPtr;
class SVGElementImpl;
typedef Ptr<SVGElementImpl> SVGElementImplPtr;
class SVGDocumentImpl;
typedef Ptr<SVGDocumentImpl> SVGDocumentImplPtr;

/*#########################################################################
## Types
#########################################################################*/

typedef enum
{
/**
 * Defines a hyperlink
 */
SVG_A_ELEMENT = 0,
/**
 * Allows control over glyphs used to render particular character
 *  data (e.g. for music symbols or Asian text)
 */
SVG_ALTGLYPH_ELEMENT,
/**
 * Defines a set of glyph substitutions (e.g. for music symbols or Asian text)
 */
SVG_ALTGLYPHDEF_ELEMENT,
/**
 * Defines a candidate set of glyph substitutions (e.g. for music symbols
 *  or Asian text)
 */
SVG_ALTGLYPHITEM_ELEMENT,
/**
 * Animates an attribute or property over time
 */
SVG_ANIMATE_ELEMENT,
/**
 * Specifies a color transformation over time
 */
SVG_ANIMATECOLOR_ELEMENT,
/**
 * Causes an element to move along a motion path
 */
SVG_ANIMATEMOTION_ELEMENT,
/**
 * Animates a transformation attribute on an element
 */
SVG_ANIMATETRANSFORM_ELEMENT,
/**
 * Defines a circle
 */
SVG_CIRCLE_ELEMENT,
/**
 *
 */
SVG_CLIPPATH_ELEMENT,
/**
 * Specifies a color profile description
 */
SVG_COLOR_PROFILE_ELEMENT,
/**
 * Defines a platform-independent cursor
 */
SVG_CURSOR_ELEMENT,
/**
 * Defines a separate font definition resource
 */
SVG_DEFINITION_SRC_ELEMENT,
/**
 * A container for referenced elements
 */
SVG_DEFS_ELEMENT,
/**
 * A text-only description for elements in SVG - not displayed as part
 *  of the graphics. User agents may display the text as a tooltip
 */
SVG_DESC_ELEMENT,
/**
 * Defines an ellipse
 */
SVG_ELLIPSE_ELEMENT,
/**
 * SVG filter. Composites two objects together using different blending modes
 */
SVG_FEBLEND_ELEMENT,
/**
 * SVG filter. Applies a matrix transformation
 */
SVG_FECOLORMATRIX_ELEMENT,
/**
 * SVG filter. Performs component-wise remapping of data
 */
SVG_FECOMPONENTTRANSFER_ELEMENT,
/**
 * SVG filter.
 */
SVG_FECOMPOSITE_ELEMENT,
/**
 * SVG filter.
 */
SVG_FECONVOLVEMATRIX_ELEMENT,
/**
 * SVG filter.
 */
SVG_FEDIFFUSELIGHTING_ELEMENT,
/**
 * SVG filter.
 */
SVG_FEDISPLACEMENTMAP_ELEMENT,
/**
 * SVG filter. Defines a light source
 */
SVG_FEDISTANTLIGHT_ELEMENT,
/**
 * SVG filter.
 */
SVG_FEFLOOD_ELEMENT,
/**
 * SVG filter. Sub-element to feComponentTransfer
 */
SVG_FEFUNCA_ELEMENT,
/**
 * SVG filter. Sub-element to feComponentTransfer
 */
SVG_FEFUNCB_ELEMENT,
/**
 * SVG filter. Sub-element to feComponentTransfer
 */
SVG_FEFUNCG_ELEMENT,
/**
 * SVG filter. Sub-element to feComponentTransfer
 */
SVG_FEFUNCR_ELEMENT,
/**
 * SVG filter. Performs a Gaussian blur on the image
 */
SVG_FEGAUSSIANBLUR_ELEMENT,
/**
 * SVG filter.
 */
SVG_FEIMAGE_ELEMENT,
/**
 * SVG filter. Creates image layers on top of each other
 */
SVG_FEMERGE_ELEMENT,
/**
 * SVG filter. Sub-element to feMerge
 */
SVG_FEMERGENODE_ELEMENT,
/**
 * SVG filter. Performs a "fattening" or "thinning" on a source graphic
 */
SVG_FEMORPHOLOGY_ELEMENT,
/**
 * SVG filter. Moves an image relative to its current position
 */
SVG_FEOFFSET_ELEMENT,
/**
 * SVG filter.
 */
SVG_FEPOINTLIGHT_ELEMENT,
/**
 * SVG filter.
 */
SVG_FESPECULARLIGHTING_ELEMENT,
/**
 * SVG filter.
 */
SVG_FESPOTLIGHT_ELEMENT,
/**
 * SVG filter.
 */
SVG_FETILE_ELEMENT,
/**
 * SVG filter.
 */
SVG_FETURBULENCE_ELEMENT,
/**
 * Container for filter effects
 */
SVG_FILTER_ELEMENT,
/**
 * Defines a font
 */
SVG_FONT_ELEMENT,
/**
 * Describes the characteristics of a font
 */
SVG_FONT_FACE_ELEMENT,
/**
 *
 */
SVG_FONT_FACE_FORMAT_ELEMENT,
/**
 *
 */
SVG_FONT_FACE_NAME_ELEMENT,
/**
 *
 */
SVG_FONT_FACE_SRC_ELEMENT,
/**
 *
 */
SVG_FONT_FACE_URI_ELEMENT,
/**
 *
 */
SVG_FOREIGNOBJECT_ELEMENT,
/**
 * A container element for grouping together related elements
 */
SVG_G_ELEMENT,
/**
 * Defines the graphics for a given glyph
 */
SVG_GLYPH_ELEMENT,
/**
 * Defines a possible glyph to use
 */
SVG_GLYPHREF_ELEMENT,
/**
 *
 */
SVG_HKERN_ELEMENT,
/**
 *
 */
SVG_IMAGE_ELEMENT,
/**
 * Defines a line
 */
SVG_LINE_ELEMENT,
/**
 * Defines a linear gradient
 */
SVG_LINEARGRADIENT_ELEMENT,
/**
 *
 */
SVG_MARKER_ELEMENT,
/**
 *
 */
SVG_MASK_ELEMENT,
/**
 * Specifies metadata
 */
SVG_METADATA_ELEMENT,
/**
 *
 */
SVG_MISSING_GLYPH_ELEMENT,
/**
 *
 */
SVG_MPATH_ELEMENT,
/**
 * Defines a path
 */
SVG_PATH_ELEMENT,
/**
 *
 */
SVG_PATTERN_ELEMENT,
/**
 * Defines a closed shape that consists of a set of connected straight lines
 */
SVG_POLYGON_ELEMENT,
/**
 * Defines a set of connected straight lines
 */
SVG_POLYLINE_ELEMENT,
/**
 * Defines a radial gradient
 */
SVG_RADIALGRADIENT_ELEMENT,
/**
 * Defines a rectangle
 */
SVG_RECT_ELEMENT,
/**
 * Container for scripts (e.g., ECMAScript)
 */
SVG_SCRIPT_ELEMENT,
/**
 * Sets the value of an attribute for a specified duration
 */
SVG_SET_ELEMENT,
/**
 *
 */
SVG_STOP_ELEMENT,
/**
 * Allows style sheets to be embedded directly within SVG content
 */
SVG_STYLE_ELEMENT,
/**
 * Defines an SVG document fragment
 */
SVG_SVG_ELEMENT,
/**
 *
 */
SVG_SWITCH_ELEMENT,
/**
 *
 */
SVG_SYMBOL_ELEMENT,
/**
 *
 */
SVG_TEXT_ELEMENT,
/**
 *
 */
SVG_TEXTPATH_ELEMENT,
/**
 * A text-only description for elements in SVG - not displayed as part of
 *  the graphics. User agents may display the text as a tooltip
 */
SVG_TITLE_ELEMENT,
/**
 *
 */
SVG_TREF_ELEMENT,
/**
 *
 */
SVG_TSPAN_ELEMENT,
/**
 *
 */
SVG_USE_ELEMENT,
/**
 *
 */
SVG_VIEW_ELEMENT,
/**
 *
 */
SVG_VKERN_ELEMENT,
/**
 *
 */
SVG_MAX_ELEMENT

} SVGElementType;




/**
 * Look up the SVG Element type enum for a given string
 * Return -1 if not found
 */
int svgElementStrToEnum(const char *str);


/**
 * Return the string corresponding to a given SVG element type enum
 * Return "unknown" if not found
 */
const char *svgElementEnumToStr(int type);


/*#########################################################################
## SVGDocumentImpl
#########################################################################*/

/**
 *
 */
class SVGDocumentImpl : virtual public SVGDocument, public DocumentImpl
{
public:


    /**
     *
     */
    virtual DOMString getTitle()
        { return title; }

    /**
     *
     */
    virtual DOMString getReferrer()
        { return referrer; }

    /**
     *
     */
    virtual DOMString getDomain()
        { return domain; }

    /**
     *
     */
    virtual DOMString getURL()
        { return url; }

    /**
     *
     */
    virtual SVGSVGElementPtr getRootElement()
        { return rootElement; }


    //####################################################
    //# Overload some createXXX() methods from DocumentImpl,
    //# To create our SVG-DOM types (in .cpp)
    //####################################################

    /**
     *
     */
    virtual ElementPtr createElement(const DOMString& tagName)
                           throw(DOMException);


    /**
     *
     */
    virtual ElementPtr createElementNS(const DOMString& namespaceURI,
                                       const DOMString& qualifiedName)
                                       throw(DOMException);

    //##################
    //# Non-API methods
    //##################

    SVGDocumentImpl(const DOMImplementation *domImpl,
                    const DOMString         &namespaceURI,
                    const DOMString         &qualifiedName,
                    const DocumentTypePtr   doctype)
                    : DocumentImpl(domImpl, namespaceURI,
                          qualifiedName, doctype)
        {
        init();
        }


    /**
     *
     */
    virtual ~SVGDocumentImpl()
        {
        }

protected:

friend class SvgParser;

    void init();

    DOMString title;
    DOMString referrer;
    DOMString domain;
    DOMString url;
    SVGSVGElementImplPtr rootElement;
};



/*#########################################################################
## SVGElementImpl
#########################################################################*/

/**
 *
 */
class SVGElementImpl : virtual public SVGElement,
                       public ElementImpl
{
public:

    /**
     *
     */
    virtual DOMString getId()
        { return id; }

    /**
     *
     */
    virtual void setId(const DOMString &val)
                       throw (DOMException)
        { id = val; }

    /**
     *
     */
    virtual DOMString getXmlBase()
        { return xmlBase; }

    /**
     *
     */
    virtual void setXmlBase(const DOMString &val)
                            throw (DOMException)
        { xmlBase = val; }

    /**
     *
     */
    virtual SVGSVGElementPtr getOwnerSVGElement()
        { return ownerSvgElement; }

    /**
     *
     */
    virtual SVGElementPtr getViewportElement()
        { return viewportElement; }


    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    SVGElementImpl()
        {}

    /**
     *
     */
    SVGElementImpl(SVGDocumentImplPtr owner, const DOMString &tagName)
                    : ElementImpl(owner, tagName)
        { init(); }

    /**
     *
     */
    SVGElementImpl(SVGDocumentImplPtr owner,
                   const DOMString &namespaceURI,
                   const DOMString &tagName)
                   : ElementImpl(owner, namespaceURI, tagName)
        { init(); }


    /**
     *
     */
    virtual ~SVGElementImpl()
        {}

protected:

    void init()
        {
        id              = "";
        xmlBase         = "";
        ownerSvgElement = NULL;
        viewportElement = NULL;
        }

    DOMString        id;
    DOMString        xmlBase;
    SVGSVGElementPtr ownerSvgElement;
    SVGElementPtr    viewportElement;

};



/*#########################################################################
## SVGSVGElementImpl
#########################################################################*/

/**
 *
 */
class SVGSVGElementImpl : virtual public SVGSVGElement,
                          public SVGElementImpl
{
public:

    /**
     *
     */
    virtual SVGAnimatedLength getX()
        { return x; }

    /**
     *
     */
    virtual SVGAnimatedLength getY()
        { return y; }

    /**
     *
     */
    virtual SVGAnimatedLength getWidth()
        { return width; }

    /**
     *
     */
    virtual SVGAnimatedLength getHeight()
        { return height; }

    /**
     *
     */
    virtual DOMString getContentScriptType()
        { return contentScriptType; }

    /**
     *
     */
    virtual void setContentScriptType(const DOMString &val)
                                     throw (DOMException)
        { contentScriptType = val; }


    /**
     *
     */
    virtual DOMString getContentStyleType()
        { return contentStyleType; }

    /**
     *
     */
    virtual void setContentStyleType(const DOMString &val)
                                     throw (DOMException)
        { contentStyleType = val; }

    /**
     *
     */
    virtual SVGRect getViewport()
        { return viewport; }

    /**
     *
     */
    virtual double getPixelUnitToMillimeterX()
        { return pixelUnitToMillimeterX; }

    /**
     *
     */
    virtual double getPixelUnitToMillimeterY()
        { return pixelUnitToMillimeterY; }

    /**
     *
     */
    virtual double getScreenPixelToMillimeterX()
        { return screenPixelToMillimeterX; }

    /**
     *
     */
    virtual double getScreenPixelToMillimeterY()
        { return screenPixelToMillimeterY; }


    /**
     *
     */
    virtual bool getUseCurrentView()
        { return useCurrentView; }

    /**
     *
     */
    virtual void setUseCurrentView(bool val) throw (DOMException)
        { useCurrentView = val; }

    /**
     *
     */
    virtual SVGViewSpec getCurrentView()
        { return currentView; }


    /**
     *
     */
    virtual double getCurrentScale()
        { return currentScale; }

    /**
     *
     */
    virtual void setCurrentScale(double val) throw (DOMException)
        { currentScale = val; }


    /**
     *
     */
    virtual SVGPoint getCurrentTranslate()
        { return currentTranslate; }


    /**
     *
     */
    virtual unsigned long suspendRedraw (unsigned long max_wait_milliseconds );

    /**
     *
     */
    virtual void unsuspendRedraw (unsigned long suspend_handle_id )
                                  throw( DOMException );

    /**
     *
     */
    virtual void unsuspendRedrawAll (  );

    /**
     *
     */
    virtual void forceRedraw (  );

    /**
     *
     */
    virtual void pauseAnimations (  );

    /**
     *
     */
    virtual void unpauseAnimations (  );

    /**
     *
     */
    virtual bool animationsPaused (  );

    /**
     *
     */
    virtual double getCurrentTime (  )
        { return currentTime; }

    /**
     *
     */
    virtual void setCurrentTime (double seconds )
        { currentTime = seconds; }

    /**
     *
     */
    virtual NodeList getIntersectionList (const SVGRect &rect,
                                          const SVGElementPtr referenceElement );

    /**
     *
     */
    virtual NodeList getEnclosureList (const SVGRect &rect,
                                       const SVGElementPtr referenceElement );

    /**
     *
     */
    virtual bool checkIntersection (const SVGElementPtr element, const SVGRect &rect );

    /**
     *
     */
    virtual bool checkEnclosure (const SVGElementPtr element, const SVGRect &rect );

    /**
     *
     */
    virtual void deselectAll (  );

    /**
     *
     */
    virtual SVGNumber createSVGNumber (  )
        {
        SVGNumber ret;
        return ret;
        }

    /**
     *
     */
    virtual SVGLength createSVGLength (  )
        {
        SVGLength ret;
        return ret;
        }

    /**
     *
     */
    virtual SVGAngle createSVGAngle (  )
        {
        SVGAngle ret;
        return ret;
        }

    /**
     *
     */
    virtual SVGPoint createSVGPoint (  )
        {
        SVGPoint ret;
        return ret;
        }

    /**
     *
     */
    virtual SVGMatrix createSVGMatrix (  )
        {
        SVGMatrix ret;
        return ret;
        }

    /**
     *
     */
    virtual SVGRect createSVGRect (  )
        {
        SVGRect ret;
        return ret;
        }

    /**
     *
     */
    virtual SVGTransform createSVGTransform (  )
        {
        SVGTransform ret;
        return ret;
        }

    /**
     *
     */
    virtual SVGTransform createSVGTransformFromMatrix(const SVGMatrix &matrix )
        {
        SVGTransform ret;
        ret.setMatrix(matrix);
        return ret;
        }


    /**
     *
     */
    virtual ElementPtr getElementById (const DOMString& elementId );



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGSVGElementImpl() : SVGElementImpl()
        {}



    /**
     *
     */
    virtual ~SVGSVGElementImpl() {}

protected:

    SVGAnimatedLength x;
    SVGAnimatedLength y;
    SVGAnimatedLength width;
    SVGAnimatedLength height;
    DOMString         contentScriptType;
    DOMString         contentStyleType;
    SVGRect           viewport;
    double            pixelUnitToMillimeterX;
    double            pixelUnitToMillimeterY;
    double            screenPixelToMillimeterX;
    double            screenPixelToMillimeterY;
    bool              useCurrentView;
    SVGViewSpec       currentView;
    double            currentScale;
    SVGPoint          currentTranslate;

    double currentTime;

};



/*#########################################################################
## SVGGElementImpl
#########################################################################*/

/**
 *
 */
class SVGGElementImpl : virtual public SVGGElement, public SVGElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGGElementImpl() {}

    /**
     *
     */
    virtual ~SVGGElementImpl() {}

protected:


};




/*#########################################################################
## SVGDefsElementImpl
#########################################################################*/

/**
 *
 */
class SVGDefsElementImpl : virtual public SVGDefsElement,
                           public SVGElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGDefsElementImpl() {}

    /**
     *
     */
    virtual ~SVGDefsElementImpl() {}

protected:


};





/*#########################################################################
## SVGDescElementImpl
#########################################################################*/

/**
 *
 */
class SVGDescElementImpl :  virtual public SVGDescElement,
                            public SVGElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGDescElementImpl() {}

    /**
     *
     */
    virtual ~SVGDescElementImpl() {}

protected:


};





/*#########################################################################
## SVGTitleElementImpl
#########################################################################*/

/**
 *
 */
class SVGTitleElementImpl : virtual public SVGTitleElement,
                            public SVGElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGTitleElementImpl() {}

    /**
     *
     */
    virtual ~SVGTitleElementImpl() {}

protected:


};





/*#########################################################################
## SVGSymbolElementImpl
#########################################################################*/

/**
 *
 */
class SVGSymbolElementImpl : virtual public SVGSymbolElement,
                             public SVGElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGSymbolElementImpl() {}

    /**
     *
     */
    virtual ~SVGSymbolElementImpl() {}

protected:


};





/*#########################################################################
## SVGUseElementImpl
#########################################################################*/

/**
 *
 */
class SVGUseElementImpl : public SVGElementImpl
{
public:


    /**
     *
     */
    virtual SVGAnimatedLength getX()
        { return x; }

    /**
     *
     */
    virtual SVGAnimatedLength getY()
        { return y; }

    /**
     *
     */
    virtual SVGAnimatedLength getWidth()
        { return width; }

    /**
     *
     */
    virtual SVGAnimatedLength getHeight()
        { return height; }

    /**
     *
     */
    virtual SVGElementInstance getInstanceRoot()
        { return instanceRoot; }

    /**
     *
     */
    virtual SVGElementInstance getAnimatedInstanceRoot()
        { return animatedInstanceRoot; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGUseElementImpl() {}

    /**
     *
     */
    virtual ~SVGUseElementImpl() {}

protected:

    SVGAnimatedLength x;
    SVGAnimatedLength y;
    SVGAnimatedLength width;
    SVGAnimatedLength height;
    SVGElementInstance instanceRoot;
    SVGElementInstance animatedInstanceRoot;
};







/*#########################################################################
## SVGImageElementImpl
#########################################################################*/

/**
 *
 */
class SVGImageElementImpl : virtual public SVGImageElement,
                            public SVGElementImpl
{
public:


    /**
     *
     */
    virtual SVGAnimatedLength getX()
        { return x; }

    /**
     *
     */
    virtual SVGAnimatedLength getY()
        { return y; }

    /**
     *
     */
    virtual SVGAnimatedLength getWidth()
        { return width; }

    /**
     *
     */
    virtual SVGAnimatedLength getHeight()
        { return height; }


    /**
     *
     */
    virtual SVGAnimatedPreserveAspectRatio getPreserveAspectRatio()
        { return preserveAspectRatio; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGImageElementImpl() {}

    /**
     *
     */
    virtual ~SVGImageElementImpl() {}

protected:

    SVGAnimatedLength x;
    SVGAnimatedLength y;
    SVGAnimatedLength width;
    SVGAnimatedLength height;
    SVGAnimatedPreserveAspectRatio preserveAspectRatio;
};






/*#########################################################################
## SVGSwitchElementImpl
#########################################################################*/

/**
 *
 */
class SVGSwitchElementImpl : virtual public SVGSwitchElement,
                             public SVGElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGSwitchElementImpl() {}

    /**
     *
     */
    virtual ~SVGSwitchElementImpl() {}

protected:


};





/*#########################################################################
## GetSVGDocumentImpl
#########################################################################*/

/**
 *
 */
class GetSVGDocumentImpl : public virtual GetSVGDocument
{
public:

    /**
     *
     */
    virtual SVGDocumentPtr getSVGDocument (  )
                    throw( DOMException );

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    GetSVGDocumentImpl() {}

    /**
     *
     */
    virtual ~GetSVGDocumentImpl() {}

protected:


};







/*#########################################################################
## SVGStyleElementImpl
#########################################################################*/

/**
 *
 */
class SVGStyleElementImpl : virtual public SVGStyleElement,
                            public SVGElementImpl
{
public:

    /**
     *
     */
    virtual DOMString getXmlspace()
        { return xmlSpace; }

    /**
     *
     */
    virtual void setXmlspace(const DOMString &val)
                             throw (DOMException)
        { xmlSpace = val; }

    /**
     *
     */
    virtual DOMString getType()
        { return type; }

    /**
     *
     */
    virtual void setType(const DOMString &val)
                         throw (DOMException)
        { type = val; }

    /**
     *
     */
    virtual DOMString getMedia()
        { return media; }

    /**
     *
     */
    virtual void setMedia(const DOMString &val)
                          throw (DOMException)
        { media = val; }

    /**
     *
     */
    virtual DOMString getTitle()
        { return title; }

    /**
     *
     */
    virtual void setTitle(const DOMString &val)
                          throw (DOMException)
        { title = val; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGStyleElementImpl() {}

    /**
     *
     */
    virtual ~SVGStyleElementImpl() {}

protected:

    DOMString xmlSpace;
    DOMString type;
    DOMString media;
    DOMString title;

};






/*#########################################################################
## SVGPathElementImpl
#########################################################################*/

/**
 *
 */
class SVGPathElementImpl : virtual public SVGPathElement,
                           public SVGElementImpl
{
public:

    /**
     *
     */
    virtual SVGAnimatedNumber getPathLength();

    /**
     *
     */
    virtual double getTotalLength (  );

    /**
     *
     */
    virtual SVGPoint getPointAtLength (double distance );

    /**
     *
     */
    virtual unsigned long getPathSegAtLength (double distance );

    /**
     *
     */
    virtual SVGPathSegClosePath
              createSVGPathSegClosePath (  )
         {
         SVGPathSegClosePath ret;
         return ret;
         }

    /**
     *
     */
    virtual SVGPathSegMovetoAbs
              createSVGPathSegMovetoAbs (double x, double y )
         {
         SVGPathSegMovetoAbs ret(x, y);
         return ret;
         }

    /**
     *
     */
    virtual SVGPathSegMovetoRel
              createSVGPathSegMovetoRel (double x, double y )
         {
         SVGPathSegMovetoRel ret(x, y);
         return ret;
         }

    /**
     *
     */
    virtual SVGPathSegLinetoAbs
              createSVGPathSegLinetoAbs (double x, double y )
         {
         SVGPathSegLinetoAbs ret(x, y);
         return ret;
         }

    /**
     *
     */
    virtual SVGPathSegLinetoRel
              createSVGPathSegLinetoRel (double x, double y )
         {
         SVGPathSegLinetoRel ret(x, y);
         return ret;
         }

    /**
     *
     */
    virtual SVGPathSegCurvetoCubicAbs
              createSVGPathSegCurvetoCubicAbs (double x, double y,
                        double x1, double y1, double x2, double y2 )
         {
         SVGPathSegCurvetoCubicAbs ret(x, y, x1, y1, x2, y2);
         return ret;
         }

    /**
     *
     */
    virtual SVGPathSegCurvetoCubicRel
              createSVGPathSegCurvetoCubicRel (double x, double y,
                        double x1, double y1, double x2, double y2 )
         {
         SVGPathSegCurvetoCubicRel ret(x, y, x1, y1, x2, y2);
         return ret;
         }

    /**
     *
     */
    virtual SVGPathSegCurvetoQuadraticAbs
              createSVGPathSegCurvetoQuadraticAbs (double x, double y,
                         double x1, double y1 )
         {
         SVGPathSegCurvetoQuadraticAbs ret(x, y, x1, y1);
         return ret;
         }

    /**
     *
     */
    virtual SVGPathSegCurvetoQuadraticRel
              createSVGPathSegCurvetoQuadraticRel (double x, double y,
                         double x1, double y1 )
         {
         SVGPathSegCurvetoQuadraticRel ret(x, y, x1, y1);
         return ret;
         }

    /**
     *
     */
    virtual SVGPathSegArcAbs
              createSVGPathSegArcAbs (double x, double y,
                         double r1, double r2, double angle,
                         bool largeArcFlag, bool sweepFlag )
         {
         SVGPathSegArcAbs ret(x, y, r1, r2, angle, largeArcFlag, sweepFlag);
         return ret;
         }

    /**
     *
     */
    virtual SVGPathSegArcRel
              createSVGPathSegArcRel (double x, double y, double r1,
                         double r2, double angle, bool largeArcFlag,
                         bool sweepFlag )
         {
         SVGPathSegArcRel ret(x, y, r1, r2, angle, largeArcFlag, sweepFlag);
         return ret;
         }

    /**
     *
     */
    virtual SVGPathSegLinetoHorizontalAbs
              createSVGPathSegLinetoHorizontalAbs (double x )
         {
         SVGPathSegLinetoHorizontalAbs ret(x);
         return ret;
         }

    /**
     *
     */
    virtual SVGPathSegLinetoHorizontalRel
              createSVGPathSegLinetoHorizontalRel (double x )
         {
         SVGPathSegLinetoHorizontalRel ret(x);
         return ret;
         }

    /**
     *
     */
    virtual SVGPathSegLinetoVerticalAbs
              createSVGPathSegLinetoVerticalAbs (double y )
         {
         SVGPathSegLinetoVerticalAbs ret(y);
         return ret;
         }

    /**
     *
     */
    virtual SVGPathSegLinetoVerticalRel
              createSVGPathSegLinetoVerticalRel (double y )
         {
         SVGPathSegLinetoVerticalRel ret(y);
         return ret;
         }

    /**
     *
     */
    virtual SVGPathSegCurvetoCubicSmoothAbs
              createSVGPathSegCurvetoCubicSmoothAbs (double x, double y,
                                             double x2, double y2 )
         {
         SVGPathSegCurvetoCubicSmoothAbs ret(x, y, x2, y2);
         return ret;
         }

    /**
     *
     */
    virtual SVGPathSegCurvetoCubicSmoothRel
              createSVGPathSegCurvetoCubicSmoothRel (double x, double y,
                                                     double x2, double y2 )
         {
         SVGPathSegCurvetoCubicSmoothRel ret(x, y, x2, y2);
         return ret;
         }

    /**
     *
     */
    virtual SVGPathSegCurvetoQuadraticSmoothAbs
              createSVGPathSegCurvetoQuadraticSmoothAbs (double x, double y )
         {
         SVGPathSegCurvetoQuadraticSmoothAbs ret(x, y);
         return ret;
         }

    /**
     *
     */
    virtual SVGPathSegCurvetoQuadraticSmoothRel
              createSVGPathSegCurvetoQuadraticSmoothRel (double x, double y )
         {
         SVGPathSegCurvetoQuadraticSmoothRel ret(x, y);
         return ret;
         }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGPathElementImpl() {}


    /**
     *
     */
    virtual ~SVGPathElementImpl() {}

protected:


};







/*#########################################################################
## SVGRectElementImpl
#########################################################################*/

/**
 *
 */
class SVGRectElementImpl : virtual public SVGRectElement,
                           public SVGElementImpl
{
public:

    /**
     *
     */
    virtual SVGAnimatedLength getX()
        { return x; }

    /**
     *
     */
    virtual SVGAnimatedLength getY()
        { return y; }

    /**
     *
     */
    virtual SVGAnimatedLength getWidth()
        { return width; }

    /**
     *
     */
    virtual SVGAnimatedLength getHeight()
        { return height; }


    /**
     *
     */
    virtual SVGAnimatedLength getRx()
        { return rx; }

    /**
     *
     */
    virtual SVGAnimatedLength getRy()
        { return ry; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGRectElementImpl() {}

    /**
     *
     */
    virtual ~SVGRectElementImpl() {}

protected:

    SVGAnimatedLength x;
    SVGAnimatedLength y;
    SVGAnimatedLength width;
    SVGAnimatedLength height;
    SVGAnimatedLength rx;
    SVGAnimatedLength ry;

};






/*#########################################################################
## SVGCircleElementImpl
#########################################################################*/

/**
 *
 */
class SVGCircleElementImpl : virtual public SVGCircleElement,
                             public SVGElementImpl
{
public:

    /**
     *
     */
    virtual SVGAnimatedLength getCx()
        { return cx; }

    /**
     *
     */
    virtual SVGAnimatedLength getCy()
        { return cy; }

    /**
     *
     */
    virtual SVGAnimatedLength getR()
        { return r; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGCircleElementImpl() {}

    /**
     *
     */
    virtual ~SVGCircleElementImpl() {}

protected:

    SVGAnimatedLength cx;
    SVGAnimatedLength cy;
    SVGAnimatedLength r;
};






/*#########################################################################
## SVGEllipseElementImpl
#########################################################################*/

/**
 *
 */
class SVGEllipseElementImpl : virtual public SVGEllipseElement,
                              public SVGElementImpl
{
public:

    /**
     *
     */
    virtual SVGAnimatedLength getCx()
        { return cx; }

    /**
     *
     */
    virtual SVGAnimatedLength getCy()
        { return cy; }

    /**
     *
     */
    virtual SVGAnimatedLength getRx()
        { return rx; }

    /**
     *
     */
    virtual SVGAnimatedLength getRy()
        { return ry; }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    SVGEllipseElementImpl() {}

    /**
     *
     */
    virtual ~SVGEllipseElementImpl() {}

protected:

    SVGAnimatedLength cx;
    SVGAnimatedLength cy;
    SVGAnimatedLength rx;
    SVGAnimatedLength ry;
};






/*#########################################################################
## SVGLineElement
#########################################################################*/

/**
 *
 */
class SVGLineElementImpl : virtual public SVGLineElement,
                           public SVGElementImpl
{
public:

    /**
     *
     */
    virtual SVGAnimatedLength getX1()
        { return x1; }

    /**
     *
     */
    virtual SVGAnimatedLength getY1()
        { return y1; }

    /**
     *
     */
    virtual SVGAnimatedLength getX2()
        { return x2; }

    /**
     *
     */
    virtual SVGAnimatedLength getY2()
        { return y2; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGLineElementImpl() {}

protected:

    SVGAnimatedLength x1;
    SVGAnimatedLength x2;
    SVGAnimatedLength y1;
    SVGAnimatedLength y2;
};




/*#########################################################################
## SVGPolylineElement
#########################################################################*/

/**
 *
 */
class SVGPolylineElementImpl : virtual public SVGPolylineElement,
                               public SVGElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGPolylineElementImpl() {}

protected:


};





/*#########################################################################
## SVGPolygonElementImpl
#########################################################################*/

/**
 *
 */
class SVGPolygonElementImpl : virtual public SVGPolygonElement,
                              public SVGElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGPolygonElementImpl() {}

protected:


};





/*#########################################################################
## SVGTextContentElement
#########################################################################*/

/**
 *
 */
class SVGTextContentElementImpl : virtual public SVGTextContentElement,
                                  public SVGElementImpl
{
public:

    /**
     *
     */
    virtual SVGAnimatedLength getTextLength();


    /**
     *
     */
    virtual SVGAnimatedEnumeration getLengthAdjust();


    /**
     *
     */
    virtual long getNumberOfChars(  );

    /**
     *
     */
    virtual double getComputedTextLength(  );

    /**
     *
     */
    virtual double getSubStringLength(unsigned long charnum,
                                      unsigned long nchars )
                                      throw( DOMException );

    /**
     *
     */
    virtual SVGPoint getStartPositionOfChar(unsigned long charnum )
                                            throw( DOMException );

    /**
     *
     */
    virtual SVGPoint getEndPositionOfChar(unsigned long charnum )
                                          throw( DOMException );

    /**
     *
     */
    virtual SVGRect getExtentOfChar(unsigned long charnum )
                                    throw( DOMException );

    /**
     *
     */
    virtual double getRotationOfChar(unsigned long charnum )
                                     throw( DOMException );

    /**
     *
     */
    virtual long getCharNumAtPosition(const SVGPoint &point );

    /**
     *
     */
    virtual void selectSubString(unsigned long charnum, unsigned long nchars )
                                 throw( DOMException );



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGTextContentElementImpl() {}

protected:


};






/*#########################################################################
## SVGTextPositioningElementImpl
#########################################################################*/

/**
 *
 */
class SVGTextPositioningElementImpl : virtual public SVGTextPositioningElement,
                                      public SVGTextContentElementImpl
{
public:



    /**
     *
     */
    virtual SVGAnimatedLength getX()
        { return x; }

    /**
     *
     */
    virtual SVGAnimatedLength getY()
        { return y; }

    /**
     *
     */
    virtual SVGAnimatedLength getDx()
        { return dx; }

    /**
     *
     */
    virtual SVGAnimatedLength getDy()
        { return dy; }


    /**
     *
     */
    virtual SVGAnimatedNumberList getRotate()
        { return rotate; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGTextPositioningElementImpl() {}

protected:

    SVGAnimatedLength x;
    SVGAnimatedLength y;
    SVGAnimatedLength dx;
    SVGAnimatedLength dy;
    SVGAnimatedNumberList rotate;

};







/*#########################################################################
## SVGTextElement
#########################################################################*/

/**
 *
 */
class SVGTextElementImpl : virtual public SVGTextElement,
                           public SVGTextPositioningElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGTextElementImpl() {}

protected:


};





/*#########################################################################
## SVGTSpanElement
#########################################################################*/

/**
 *
 */
class SVGTSpanElementImpl : virtual public SVGTSpanElement,
                            public SVGTextPositioningElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGTSpanElementImpl() {}

protected:


};





/*#########################################################################
## SVGTRefElement
#########################################################################*/

/**
 *
 */
class SVGTRefElementImpl : virtual public SVGTRefElement,
                           public SVGTextPositioningElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGTRefElementImpl() {}

protected:


};





/*#########################################################################
## SVGTextPathElement
#########################################################################*/

/**
 *
 */
class SVGTextPathElementImpl : virtual public SVGTextPathElement,
                               public SVGTextContentElementImpl
{
public:

    /**
     *
     */
    virtual SVGAnimatedLength getStartOffset()
        { return startOffset; }

    /**
     *
     */
    virtual SVGAnimatedEnumeration getMethod()
        { return method; }

    /**
     *
     */
    virtual SVGAnimatedEnumeration getSpacing()
        { return spacing; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGTextPathElementImpl() {}

protected:

    SVGAnimatedLength startOffset;
    SVGAnimatedEnumeration method;
    SVGAnimatedEnumeration spacing;
};







/*#########################################################################
## SVGAltGlyphElement
#########################################################################*/

/**
 *
 */
class SVGAltGlyphElementImpl : virtual public SVGAltGlyphElement,
                               public SVGTextPositioningElementImpl
{
public:

    /**
     *
     */
    virtual DOMString getGlyphRef()
        { return glyphRef; }

    /**
     *
     */
    virtual void setGlyphRef(const DOMString &val)
                             throw (DOMException)
        { glyphRef = val; }

    /**
     *
     */
    virtual DOMString getFormat()
        { return format; }

    /**
     *
     */
    virtual void setFormat(const DOMString &val)
                           throw (DOMException)
        { format = val; }




    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGAltGlyphElementImpl() {}

protected:

    DOMString glyphRef;
    DOMString format;

};







/*#########################################################################
## SVGAltGlyphDefElementImpl
#########################################################################*/

/**
 *
 */
class SVGAltGlyphDefElementImpl : virtual public SVGAltGlyphDefElement,
                                  public SVGElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGAltGlyphDefElementImpl() {}

protected:


};





/*#########################################################################
## SVGAltGlyphItemElementImpl
#########################################################################*/

/**
 *
 */
class SVGAltGlyphItemElementImpl : virtual public SVGAltGlyphItemElement,
                                   public SVGElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGAltGlyphItemElementImpl() {}

protected:


};





/*#########################################################################
## SVGGlyphRefElementImpl
#########################################################################*/

/**
 *
 */
class SVGGlyphRefElementImpl : virtual public SVGGlyphRefElement,
                               public SVGElementImpl
{
public:
    /**
     *
     */
    virtual DOMString getGlyphRef()
        { return glyphRef; }

    /**
     *
     */
    virtual void setGlyphRef(const DOMString &val) throw (DOMException)
        { glyphRef = val; }

    /**
     *
     */
    virtual DOMString getFormat()
        { return format; }

    /**
     *
     */
    virtual void setFormat(const DOMString &val) throw (DOMException)
        { format = val; }

    /**
     *
     */
    virtual double getX()
        { return x; }

    /**
     *
     */
    virtual void setX(double val) throw (DOMException)
        { x = val; }

    /**
     *
     */
    virtual double getY()
        { return y; }

    /**
     *
     */
    virtual void setY(double val) throw (DOMException)
        { y = val; }

    /**
     *
     */
    virtual double getDx()
        { return dx; }

    /**
     *
     */
    virtual void setDx(double val) throw (DOMException)
        { dx = val; }

    /**
     *
     */
    virtual double getDy()
        { return dy; }

    /**
     *
     */
    virtual void setDy(double val) throw (DOMException)
        { dy = val; }




    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGGlyphRefElementImpl() {}

protected:

    DOMString glyphRef;
    DOMString format;
    double x, y, dx, dy;

};






/*#########################################################################
## SVGMarkerElementImpl
#########################################################################*/

/**
 *
 */
class SVGMarkerElementImpl : virtual public SVGMarkerElement,
                             public SVGElementImpl
{
public:

    /**
     *
     */
    virtual SVGAnimatedLength getRefX()
        { return refX; }

    /**
     *
     */
    virtual SVGAnimatedLength getRefY()
        { return refY; }

    /**
     *
     */
    virtual SVGAnimatedEnumeration getMarkerUnits()
        { return markerUnits; }

    /**
     *
     */
    virtual SVGAnimatedLength getMarkerWidth()
        { return markerWidth; }

    /**
     *
     */
    virtual SVGAnimatedLength getMarkerHeight()
        { return markerHeight; }

    /**
     *
     */
    virtual SVGAnimatedEnumeration getOrientType()
        { return orientType; }

    /**
     *
     */
    virtual SVGAnimatedAngle getOrientAngle()
        { return orientAngle; }


    /**
     *
     */
    virtual void setOrientToAuto (  )
        { orientAuto = true; }

    /**
     *
     */
    virtual void setOrientToAngle (const SVGAngle &angle)
        {
        orientAuto = false;
        orientAngle = SVGAnimatedAngle(angle);
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGMarkerElementImpl() {}

protected:

    SVGAnimatedLength      refX;
    SVGAnimatedLength      refY;
    SVGAnimatedEnumeration markerUnits;
    SVGAnimatedLength      markerWidth;
    SVGAnimatedLength      markerHeight;
    SVGAnimatedEnumeration orientType;
    SVGAnimatedAngle       orientAngle;
    bool                   orientAuto;


};







/*#########################################################################
## SVGColorProfileElementImpl
#########################################################################*/

/**
 *
 */
class SVGColorProfileElementImpl : virtual public SVGColorProfileElement,
                                   public SVGElementImpl
{
public:
    /**
     *
     */
    virtual DOMString getLocal()
        { return local; }

    /**
     *
     */
    virtual void setLocal(const DOMString &val) throw (DOMException)
        { local = val; }

    /**
     *
     */
    virtual DOMString getName()
        { return name; }

    /**
     *
     */
    virtual void setName(const DOMString &val) throw (DOMException)
       { name = val; }

    /**
     *
     */
    virtual unsigned short getRenderingIntent()
        { return renderingIntent; }

    /**
     *
     */
    virtual void setRenderingIntent(unsigned short val) throw (DOMException)
       { renderingIntent = val; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGColorProfileElementImpl() {}

protected:

    DOMString local;
    DOMString name;
    unsigned short renderingIntent;

};





/*#########################################################################
## SVGGradientElementImpl
#########################################################################*/

/**
 *
 */
class SVGGradientElementImpl : virtual public SVGGradientElement,
                               public SVGElementImpl
{
public:

    /**
     *
     */
    virtual SVGAnimatedEnumeration getGradientUnits()
        { return gradientUnits; }

    /**
     *
     */
    virtual SVGAnimatedTransformList getGradientTransform()
        { return gradientTransform; }

    /**
     *
     */
    virtual SVGAnimatedEnumeration getSpreadMethod()
        { return spreadMethod; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGGradientElementImpl() {}

protected:


    SVGAnimatedEnumeration   gradientUnits;
    SVGAnimatedTransformList gradientTransform;
    SVGAnimatedEnumeration   spreadMethod;
};







/*#########################################################################
## SVGLinearGradientElementImpl
#########################################################################*/

/**
 *
 */
class SVGLinearGradientElementImpl : virtual public SVGLinearGradientElement,
                                     public SVGGradientElementImpl
{
public:


    /**
     *
     */
    virtual SVGAnimatedLength getX1()
        { return x1; }

    /**
     *
     */
    virtual SVGAnimatedLength getY1()
        { return y1; }

    /**
     *
     */
    virtual SVGAnimatedLength getX2()
        { return x2; }

    /**
     *
     */
    virtual SVGAnimatedLength getY2()
        { return y2; }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGLinearGradientElementImpl() {}

protected:

    SVGAnimatedLength x1, x2, y1, y2;

};







/*#########################################################################
## SVGRadialGradientElementImpl
#########################################################################*/

/**
 *
 */
class SVGRadialGradientElementImpl : virtual public SVGRadialGradientElement,
                                     public SVGGradientElementImpl
{
public:


    /**
     *
     */
    virtual SVGAnimatedLength getCx()
        { return cx; }


    /**
     *
     */
    virtual SVGAnimatedLength getCy()
        { return cy; }


    /**
     *
     */
    virtual SVGAnimatedLength getR()
        { return r; }


    /**
     *
     */
    virtual SVGAnimatedLength getFx()
        { return fx; }


    /**
     *
     */
    virtual SVGAnimatedLength getFy()
        { return fy; }




    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGRadialGradientElementImpl() {}

protected:

    SVGAnimatedLength cx, cy, r, fx, fy;

};







/*#########################################################################
## SVGStopElementImpl
#########################################################################*/

/**
 *
 */
class SVGStopElementImpl : virtual public SVGStopElement,
                           public SVGElementImpl
{
public:

    /**
     *
     */
    virtual SVGAnimatedNumber getOffset()
        { return offset; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGStopElementImpl() {}

protected:

    SVGAnimatedNumber offset;

};







/*#########################################################################
## SVGPatternElementImpl
#########################################################################*/

/**
 *
 */
class SVGPatternElementImpl : virtual public SVGPatternElement,
                              public SVGElementImpl
{
public:

    /**
     *
     */
    virtual SVGAnimatedEnumeration getPatternUnits()
        { return patternUnits; }

    /**
     *
     */
    virtual SVGAnimatedEnumeration getPatternContentUnits()
        { return patternContentUnits; }

    /**
     *
     */
    virtual SVGAnimatedTransformList getPatternTransform()
        { return patternTransform; }

    /**
     *
     */
    virtual SVGAnimatedLength getX()
        { return x; }

    /**
     *
     */
    virtual SVGAnimatedLength getY()
        { return y; }

    /**
     *
     */
    virtual SVGAnimatedLength getWidth()
        { return width; }

    /**
     *
     */
    virtual SVGAnimatedLength getHeight()
        { return height; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGPatternElementImpl() {}

protected:


    SVGAnimatedEnumeration   patternUnits;
    SVGAnimatedEnumeration   patternContentUnits;
    SVGAnimatedTransformList patternTransform;
    SVGAnimatedLength        x;
    SVGAnimatedLength        y;
    SVGAnimatedLength        width;
    SVGAnimatedLength        height;
};







/*#########################################################################
## SVGClipPathElementImpl
#########################################################################*/

/**
 *
 */
class SVGClipPathElementImpl : virtual public SVGClipPathElement,
                               public SVGElementImpl
{
public:

    /**
     *
     */
    virtual SVGAnimatedEnumeration getClipPathUnits()
        { return clipPathUnits; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGClipPathElementImpl() {}

protected:

    SVGAnimatedEnumeration clipPathUnits;

};







/*#########################################################################
## SVGMaskElementImpl
#########################################################################*/

/**
 *
 */
class SVGMaskElementImpl : virtual public SVGMaskElement,
                           public SVGElementImpl
{
public:

    /**
     *
     */
    virtual SVGAnimatedEnumeration getMaskUnits()
        { return maskUnits; }

    /**
     *
     */
    virtual SVGAnimatedEnumeration getMaskContentUnits()
        { return maskContentUnits; }

    /**
     *
     */
    virtual SVGAnimatedLength getX()
        { return x; }

    /**
     *
     */
    virtual SVGAnimatedLength getY()
        { return y; }

    /**
     *
     */
    virtual SVGAnimatedLength getWidth()
        { return width; }

    /**
     *
     */
    virtual SVGAnimatedLength getHeight()
        { return height; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGMaskElementImpl() {}

protected:


    SVGAnimatedEnumeration maskUnits;
    SVGAnimatedEnumeration maskContentUnits;
    SVGAnimatedLength      x;
    SVGAnimatedLength      y;
    SVGAnimatedLength      width;
    SVGAnimatedLength      height;
};







/*#########################################################################
## SVGFilterElementImpl
#########################################################################*/

/**
 *
 */
class SVGFilterElementImpl : virtual public SVGFilterElement,
                             public SVGElementImpl
{
public:

    /**
     *
     */
    virtual SVGAnimatedEnumeration getFilterUnits()
        { return filterUnits; }

    /**
     *
     */
    virtual SVGAnimatedEnumeration getPrimitiveUnits()
        { return filterUnits; }

    /**
     *
     */
    virtual SVGAnimatedLength getX()
        { return x; }

    /**
     *
     */
    virtual SVGAnimatedLength getY()
        { return y; }

    /**
     *
     */
    virtual SVGAnimatedLength getWidth()
        { return width; }

    /**
     *
     */
    virtual SVGAnimatedLength getHeight()
        { return height; }

    /**
     *
     */
    virtual SVGAnimatedInteger getFilterResX()
        { return filterResX; }

    /**
     *
     */
    virtual SVGAnimatedInteger getFilterResY()
        { return filterResY; }

    /**
     *
     */
    virtual void setFilterRes (unsigned long filterResXArg,
                               unsigned long filterResYArg )
        {
        filterResX = filterResXArg;
        filterResY = filterResYArg;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFilterElementImpl() {}

protected:

    SVGAnimatedEnumeration filterUnits;
    SVGAnimatedEnumeration primitiveUnits;
    SVGAnimatedLength      x;
    SVGAnimatedLength      y;
    SVGAnimatedLength      width;
    SVGAnimatedLength      height;
    SVGAnimatedInteger     filterResX;
    SVGAnimatedInteger     filterResY;

};






/*#########################################################################
## SVGFEBlendElementImpl
#########################################################################*/

/**
 *
 */
class SVGFEBlendElementImpl : virtual public SVGFEBlendElement,
                              public SVGElementImpl
{
public:

    /**
     *
     */
    virtual SVGAnimatedString getIn1()
        { return in1; }

    /**
     *
     */
    virtual SVGAnimatedString getIn2()
        { return in2; }

    /**
     *
     */
    virtual SVGAnimatedEnumeration getMode()
        { return mode; }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEBlendElementImpl() {}

protected:

    SVGAnimatedString in1, in2;
    SVGAnimatedEnumeration mode;
};







/*#########################################################################
## SVGFEColorMatrixElementImpl
#########################################################################*/

/**
 *
 */
class SVGFEColorMatrixElementImpl : virtual public SVGFEColorMatrixElement,
                                    public SVGElementImpl
{
public:

    /**
     *
     */
    virtual SVGAnimatedString getIn1()
        { return in1; }

    /**
     *
     */
    virtual SVGAnimatedEnumeration getType()
        { return type; }

    /**
     *
     */
    virtual SVGAnimatedNumberList getValues()
        { return values; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEColorMatrixElementImpl() {}

protected:

    SVGAnimatedString in1;
    SVGAnimatedEnumeration type;
    SVGAnimatedNumberList values;

};







/*#########################################################################
## SVGFEComponentTransferElementImpl
#########################################################################*/

/**
 *
 */
class SVGFEComponentTransferElementImpl :
                        virtual public SVGFEComponentTransferElement,
                        public SVGElementImpl
{
public:
    /**
     *
     */
    virtual SVGAnimatedString getIn1()
        { return in1; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEComponentTransferElementImpl() {}

protected:

    SVGAnimatedString in1;

};







/*#########################################################################
## SVGComponentTransferFunctionElementImpl
#########################################################################*/

/**
 *
 */
class SVGComponentTransferFunctionElementImpl :
                            virtual public SVGComponentTransferFunctionElement,
                            public SVGElementImpl
{
public:

    /**
     *
     */
    virtual SVGAnimatedEnumeration getType()
        { return type; }

    /**
     *
     */
    virtual SVGAnimatedNumberList getTableValues()
        { return tableValues; }

    /**
     *
     */
    virtual SVGAnimatedNumber getSlope()
        { return slope; }

    /**
     *
     */
    virtual SVGAnimatedNumber getIntercept()
        { return intercept; }

    /**
     *
     */
    virtual SVGAnimatedNumber getAmplitude()
        { return amplitude; }

    /**
     *
     */
    virtual SVGAnimatedNumber getExponent()
        { return exponent; }

    /**
     *
     */
    virtual SVGAnimatedNumber getOffset()
        { return offset; }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGComponentTransferFunctionElementImpl() {}

protected:

    SVGAnimatedEnumeration type;
    SVGAnimatedNumberList  tableValues;
    SVGAnimatedNumber      slope;
    SVGAnimatedNumber      intercept;
    SVGAnimatedNumber      amplitude;
    SVGAnimatedNumber      exponent;
    SVGAnimatedNumber      offset;

};







/*#########################################################################
## SVGFEFuncRElementImpl
#########################################################################*/

/**
 *
 */
class SVGFEFuncRElementImpl :
                       virtual public SVGFEFuncRElement,
                       public SVGComponentTransferFunctionElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEFuncRElementImpl() {}

protected:


};





/*#########################################################################
## SVGFEFuncGElementImpl
#########################################################################*/

/**
 *
 */
class SVGFEFuncGElementImpl : virtual public SVGFEFuncGElement,
                              public SVGComponentTransferFunctionElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEFuncGElementImpl() {}

protected:


};





/*#########################################################################
## SVGFEFuncBElementImpl
#########################################################################*/

/**
 *
 */
class SVGFEFuncBElementImpl : virtual public SVGFEFuncBElement,
                              public SVGComponentTransferFunctionElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEFuncBElementImpl() {}

protected:


};





/*#########################################################################
## SVGFEFuncAElementImpl
#########################################################################*/

/**
 *
 */
class SVGFEFuncAElementImpl : virtual public SVGFEFuncAElement,
                              public SVGComponentTransferFunctionElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEFuncAElementImpl() {}

protected:


};





/*#########################################################################
## SVGFECompositeElementImpl
#########################################################################*/

/**
 *
 */
class SVGFECompositeElementImpl : virtual public SVGFECompositeElement,
                                  public SVGElementImpl
{
public:


    /**
     *
     */
    virtual SVGAnimatedString getIn1()
        { return in1; }

    /**
     *
     */
    virtual SVGAnimatedString getIn2()
        { return in2; }

    /**
     *
     */
    virtual SVGAnimatedEnumeration getOperator()
        { return ae_operator; }

    /**
     *
     */
    virtual SVGAnimatedNumber getK1()
        { return k1; }

    /**
     *
     */
    virtual SVGAnimatedNumber getK2()
        { return k2; }

    /**
     *
     */
    virtual SVGAnimatedNumber getK3()
        { return k3; }

    /**
     *
     */
    virtual SVGAnimatedNumber getK4()
        { return k4; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFECompositeElementImpl() {}

protected:


    SVGAnimatedString      in1;
    SVGAnimatedString      in2;
    SVGAnimatedEnumeration ae_operator;
    SVGAnimatedNumber      k1;
    SVGAnimatedNumber      k2;
    SVGAnimatedNumber      k3;
    SVGAnimatedNumber      k4;

};







/*#########################################################################
## SVGFEConvolveMatrixElementImpl
#########################################################################*/

/**
 *
 */
class SVGFEConvolveMatrixElementImpl : virtual public SVGFEConvolveMatrixElement,
                                       public SVGElementImpl
{
public:

    /**
     *
     */
    virtual SVGAnimatedInteger getOrderX()
        { return orderX; }

    /**
     *
     */
    virtual SVGAnimatedInteger getOrderY()
        { return orderY; }

    /**
     *
     */
    virtual SVGAnimatedNumberList getKernelMatrix()
        { return kernelMatrix; }

    /**
     *
     */
    virtual SVGAnimatedNumber getDivisor()
        { return divisor; }

    /**
     *
     */
    virtual SVGAnimatedNumber getBias()
        { return bias; }

    /**
     *
     */
    virtual SVGAnimatedInteger getTargetX()
        { return targetX; }

    /**
     *
     */
    virtual SVGAnimatedInteger getTargetY()
        { return targetY; }

    /**
     *
     */
    virtual SVGAnimatedEnumeration getEdgeMode()
        { return edgeMode; }

    /**
     *
     */
    virtual SVGAnimatedLength getKernelUnitLengthX()
        { return kernelUnitLengthX; }

    /**
     *
     */
    virtual SVGAnimatedLength getKernelUnitLengthY()
        { return kernelUnitLengthY; }

    /**
     *
     */
    virtual SVGAnimatedBoolean getPreserveAlpha()
        { return preserveAlpha; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEConvolveMatrixElementImpl() {}

protected:

    SVGAnimatedInteger     orderX;
    SVGAnimatedInteger     orderY;
    SVGAnimatedNumberList  kernelMatrix;
    SVGAnimatedNumber      divisor;
    SVGAnimatedNumber      bias;
    SVGAnimatedInteger     targetX;
    SVGAnimatedInteger     targetY;
    SVGAnimatedEnumeration edgeMode;
    SVGAnimatedLength      kernelUnitLengthX;
    SVGAnimatedLength      kernelUnitLengthY;
    SVGAnimatedBoolean     preserveAlpha;

};







/*#########################################################################
## SVGFEDiffuseLightingElementImpl
#########################################################################*/

/**
 *
 */
class SVGFEDiffuseLightingElementImpl : virtual public SVGFEDiffuseLightingElement,
                                        public SVGElementImpl
{
public:

    /**
     *
     */
    virtual SVGAnimatedString getIn1()
        { return in1; }

    /**
     *
     */
    virtual SVGAnimatedNumber getSurfaceScale()
        { return surfaceScale; }

    /**
     *
     */
    virtual SVGAnimatedNumber getDiffuseConstant()
        { return diffuseConstant; }

    /**
     *
     */
    virtual SVGAnimatedNumber getKernelUnitLengthX()
        { return kernelUnitLengthX; }

    /**
     *
     */
    virtual SVGAnimatedNumber getKernelUnitLengthY()
        { return kernelUnitLengthY; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEDiffuseLightingElementImpl() {}

protected:

    SVGAnimatedString in1;
    SVGAnimatedNumber surfaceScale;
    SVGAnimatedNumber diffuseConstant;
    SVGAnimatedNumber kernelUnitLengthX;
    SVGAnimatedNumber kernelUnitLengthY;

};







/*#########################################################################
## SVGFEDistantLightElementImpl
#########################################################################*/

/**
 *
 */
class SVGFEDistantLightElementImpl : virtual public SVGFEDistantLightElement,
                                     public SVGElementImpl
{
public:

    /**
     *
     */
    virtual SVGAnimatedNumber getAzimuth()
        { return azimuth; }


    /**
     *
     */
    virtual SVGAnimatedNumber getElevation()
        { return elevation; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEDistantLightElementImpl() {}

protected:

    SVGAnimatedNumber azimuth;
    SVGAnimatedNumber elevation;

};







/*#########################################################################
## SVGFEPointLightElementImpl
#########################################################################*/

/**
 *
 */
class SVGFEPointLightElementImpl : public virtual SVGFEPointLightElement,
                                   public SVGElementImpl
{
public:

    /**
     *
     */
    virtual SVGAnimatedNumber getX()
        { return x; }


    /**
     *
     */
    virtual SVGAnimatedNumber getY()
        { return y; }

    /**
     *
     */
    virtual SVGAnimatedNumber getZ()
        { return z; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEPointLightElementImpl() {}

protected:

    SVGAnimatedNumber x, y, z;

};







/*#########################################################################
## SVGFESpotLightElementImpl
#########################################################################*/

/**
 *
 */
class SVGFESpotLightElementImpl : virtual public SVGFESpotLightElement,
                                  public SVGElementImpl
{
public:

    /**
     *
     */
    virtual SVGAnimatedNumber getX()
        { return x; }


    /**
     *
     */
    virtual SVGAnimatedNumber getY()
        { return y; }

    /**
     *
     */
    virtual SVGAnimatedNumber getZ()
        { return z; }

    /**
     *
     */
    virtual SVGAnimatedNumber getPointsAtX()
        { return pointsAtX; }

    /**
     *
     */
    virtual SVGAnimatedNumber getPointsAtY()
        { return pointsAtY; }

    /**
     *
     */
    virtual SVGAnimatedNumber getPointsAtZ()
        { return pointsAtZ; }

    /**
     *
     */
    virtual SVGAnimatedNumber getSpecularExponent()
        { return specularExponent; }

    /**
     *
     */
    virtual SVGAnimatedNumber getLimitingConeAngle()
        { return limitingConeAngle; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFESpotLightElementImpl() {}

protected:

    SVGAnimatedNumber x, y, z;
    SVGAnimatedNumber pointsAtX, pointsAtY, pointsAtZ;
    SVGAnimatedNumber specularExponent;
    SVGAnimatedNumber limitingConeAngle;
};







/*#########################################################################
## SVGFEDisplacementMapElementImpl
#########################################################################*/

/**
 *
 */
class SVGFEDisplacementMapElementImpl : virtual public SVGFEDisplacementMapElement,
                                        public SVGElementImpl
{
public:

    /**
     *
     */
    virtual SVGAnimatedString getIn1()
        { return in1; }

    /**
     *
     */
    virtual SVGAnimatedString getIn2()
        { return in2; }


    /**
     *
     */
    virtual SVGAnimatedNumber getScale()
        { return scale; }

    /**
     *
     */
    virtual SVGAnimatedEnumeration getXChannelSelector()
        { return xChannelSelector; }

    /**
     *
     */
    virtual SVGAnimatedEnumeration getYChannelSelector()
        { return yChannelSelector; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEDisplacementMapElementImpl() {}

protected:

    SVGAnimatedString      in1;
    SVGAnimatedString      in2;
    SVGAnimatedNumber      scale;
    SVGAnimatedEnumeration xChannelSelector;
    SVGAnimatedEnumeration yChannelSelector;

};







/*#########################################################################
## SVGFEFloodElementImpl
#########################################################################*/

/**
 *
 */
class SVGFEFloodElementImpl : virtual public SVGFEFloodElement,
                              public SVGElementImpl
{
public:
    /**
     *
     */
    virtual SVGAnimatedString getIn1()
        { return in1; }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEFloodElementImpl() {}

protected:

    SVGAnimatedString in1;

};







/*#########################################################################
## SVGFEGaussianBlurElementImpl
#########################################################################*/

/**
 *
 */
class SVGFEGaussianBlurElementImpl : virtual public SVGFEGaussianBlurElement,
                                     public SVGElementImpl
{
public:
    /**
     *
     */
    virtual SVGAnimatedString getIn1()
        { return in1; }


    /**
     *
     */
    virtual SVGAnimatedNumber getStdDeviationX()
        { return stdDeviationX; }

    /**
     *
     */
    virtual SVGAnimatedNumber getStdDeviationY()
        { return stdDeviationY; }


    /**
     *
     */
    virtual void setStdDeviation (double stdDeviationXArg, double stdDeviationYArg )
        {
        stdDeviationX = stdDeviationXArg;
        stdDeviationY = stdDeviationYArg;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEGaussianBlurElementImpl() {}

protected:

    SVGAnimatedString in1;
    SVGAnimatedNumber stdDeviationX, stdDeviationY;

};







/*#########################################################################
## SVGFEImageElementImpl
#########################################################################*/

/**
 *
 */
class SVGFEImageElementImpl : virtual public SVGFEImageElement,
                              public SVGElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEImageElementImpl() {}

protected:


};





/*#########################################################################
## SVGFEMergeElementImpl
#########################################################################*/

/**
 *
 */
class SVGFEMergeElementImpl : virtual public SVGFEMergeElement,
                              public SVGElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEMergeElementImpl() {}

protected:


};





/*#########################################################################
## SVGFEMergeNodeElementImpl
#########################################################################*/

/**
 *
 */
class SVGFEMergeNodeElementImpl : virtual public SVGFEMergeNodeElement,
                                  public SVGElementImpl
{
public:
    /**
     *
     */
    virtual SVGAnimatedString getIn1()
        { return in1; }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEMergeNodeElementImpl() {}

protected:

    SVGAnimatedString in1;

};







/*#########################################################################
## SVGFEMorphologyElementImpl
#########################################################################*/

/**
 *
 */
class SVGFEMorphologyElementImpl : virtual public SVGFEMorphologyElement,
                                   public SVGElementImpl
{
public:

    /**
     *
     */
    virtual SVGAnimatedString getIn1()
        { return in1; }


    /**
     *
     */
    virtual SVGAnimatedEnumeration getOperator()
        { return me_operator; }

    /**
     *
     */
    virtual SVGAnimatedLength getRadiusX()
        { return radiusX; }

    /**
     *
     */
    virtual SVGAnimatedLength getRadiusY()
        { return radiusY; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEMorphologyElementImpl() {}

protected:

    SVGAnimatedString      in1;
    SVGAnimatedEnumeration me_operator;
    SVGAnimatedLength      radiusX;
    SVGAnimatedLength      radiusY;

};







/*#########################################################################
## SVGFEOffsetElementImpl
#########################################################################*/

/**
 *
 */
class SVGFEOffsetElementImpl : virtual public SVGFEOffsetElement,
                               public SVGElementImpl
{
public:



    /**
     *
     */
    virtual SVGAnimatedString getIn1()
        { return in1; }

    /**
     *
     */
    virtual SVGAnimatedLength getDx()
        { return dx; }

    /**
     *
     */
    virtual SVGAnimatedLength getDy()
        { return dy; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFEOffsetElementImpl() {}

protected:

    SVGAnimatedString in1;
    SVGAnimatedLength dx, dy;

};







/*#########################################################################
## SVGFESpecularLightingElementImpl
#########################################################################*/

/**
 *
 */
class SVGFESpecularLightingElementImpl :
                       virtual public SVGFESpecularLightingElement,
                       public SVGElementImpl
{
public:

    /**
     *
     */
    virtual SVGAnimatedString getIn1()
        { return in1; }

    /**
     *
     */
    virtual SVGAnimatedNumber getSurfaceScale()
        { return surfaceScale; }

    /**
     *
     */
    virtual SVGAnimatedNumber getSpecularConstant()
        { return specularConstant; }

    /**
     *
     */
    virtual SVGAnimatedNumber getSpecularExponent()
        { return specularExponent; }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFESpecularLightingElementImpl() {}

protected:

    SVGAnimatedString in1;
    SVGAnimatedNumber surfaceScale;
    SVGAnimatedNumber specularConstant;
    SVGAnimatedNumber specularExponent;
};







/*#########################################################################
## SVGFETileElementImpl
#########################################################################*/

/**
 *
 */
class SVGFETileElementImpl : virtual public SVGFETileElement,
                             public SVGElementImpl
{
public:


    /**
     *
     */
    virtual SVGAnimatedString getIn1()
        { return in1; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFETileElementImpl() {}

protected:

    SVGAnimatedString in1;

};







/*#########################################################################
## SVGFETurbulenceElementImpl
#########################################################################*/

/**
 *
 */
class SVGFETurbulenceElementImpl : virtual public SVGFETurbulenceElement,
                                   public SVGElementImpl
{
public:

    /**
     *
     */
    virtual SVGAnimatedNumber getBaseFrequencyX()
        { return baseFrequencyX; }

    /**
     *
     */
    virtual SVGAnimatedNumber getBaseFrequencyY()
        { return baseFrequencyY; }

    /**
     *
     */
    virtual SVGAnimatedInteger getNumOctaves()
        { return numOctaves; }

    /**
     *
     */
    virtual SVGAnimatedNumber getSeed()
        { return seed; }

    /**
     *
     */
    virtual SVGAnimatedEnumeration getStitchTiles()
        { return stitchTiles; }

    /**
     *
     */
    virtual SVGAnimatedEnumeration getType()
        { return type; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFETurbulenceElementImpl() {}

protected:

    SVGAnimatedNumber      baseFrequencyX;
    SVGAnimatedNumber      baseFrequencyY;
    SVGAnimatedInteger     numOctaves;
    SVGAnimatedNumber      seed;
    SVGAnimatedEnumeration stitchTiles;
    SVGAnimatedEnumeration type;

};







/*#########################################################################
## SVGCursorElementImpl
#########################################################################*/

/**
 *
 */
class SVGCursorElementImpl : virtual public SVGCursorElement,
                             public SVGElementImpl
{
public:

    /**
     *
     */
    virtual SVGAnimatedLength getX()
        { return x; }

    /**
     *
     */
    virtual SVGAnimatedLength getY()
        { return x; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGCursorElementImpl() {}

protected:

    SVGAnimatedLength x, y;
};







/*#########################################################################
## SVGAElementImpl
#########################################################################*/

/**
 *
 */
class SVGAElementImpl : virtual public SVGAElement,
                        public SVGElementImpl
{
public:

    /**
     *
     */
    virtual SVGAnimatedString getTarget()
        { return target; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGAElementImpl() {}

protected:

    SVGAnimatedString target;
};







/*#########################################################################
## SVGViewElementImpl
#########################################################################*/

/**
 *
 */
class SVGViewElementImpl : virtual public SVGViewElement,
                           public SVGElementImpl
{
public:

    /**
     *
     */
    virtual SVGStringList getViewTarget()
        { return viewTarget; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGViewElementImpl() {}

protected:

    SVGStringList viewTarget;
};







/*#########################################################################
## SVGScriptElementImpl
#########################################################################*/

/**
 *
 */
class SVGScriptElementImpl : virtual public SVGScriptElement,
                             public SVGElementImpl
{
public:

    /**
     *
     */
    virtual DOMString getType()
        { return type; }

    /**
     *
     */
    virtual void setType(const DOMString &val) throw (DOMException)
        { type = val; }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGScriptElementImpl() {}

protected:

    DOMString type;
};






/*#########################################################################
## SVGAnimationElementImpl
#########################################################################*/

/**
 *
 */
class SVGAnimationElementImpl : virtual public SVGAnimationElement,
                                public SVGElementImpl
{
public:


    /**
     *
     */
    virtual SVGElementPtr getTargetElement()
        { return targetElement; }


    /**
     *
     */
    virtual double getStartTime (  )
        { return startTime; }

    /**
     *
     */
    virtual double getCurrentTime (  )
        { return currentTime; }

    /**
     *
     */
    virtual double getSimpleDuration (  ) throw( DOMException )
        { return simpleDuration; }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGAnimationElementImpl() {}

protected:

    SVGElementPtr targetElement;
    double startTime, currentTime, simpleDuration;
};







/*#########################################################################
## SVGAnimateElementImpl
#########################################################################*/

/**
 *
 */
class SVGAnimateElementImpl : virtual public SVGAnimateElement,
                              public SVGAnimationElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGAnimateElementImpl() {}

protected:


};





/*#########################################################################
## SVGSetElementImpl
#########################################################################*/

/**
 *
 */
class SVGSetElementImpl : virtual public SVGSetElement,
                          public SVGAnimationElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGSetElementImpl() {}

protected:


};





/*#########################################################################
## SVGAnimateMotionElementImpl
#########################################################################*/

/**
 *
 */
class SVGAnimateMotionElementImpl : virtual public SVGAnimateMotionElement,
                                    public SVGAnimationElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGAnimateMotionElementImpl() {}

protected:


};





/*#########################################################################
## SVGMPathElementImpl
#########################################################################*/

/**
 *
 */
class SVGMPathElementImpl : virtual public SVGMPathElement,
                            public SVGElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGMPathElementImpl() {}

protected:


};





/*#########################################################################
## SVGAnimateColorElementImpl
#########################################################################*/

/**
 *
 */
class SVGAnimateColorElementImpl : virtual public SVGAnimateColorElement,
                                   public SVGAnimationElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGAnimateColorElementImpl() {}

protected:


};





/*#########################################################################
## SVGAnimateTransformElementImpl
#########################################################################*/

/**
 *
 */
class SVGAnimateTransformElementImpl : virtual public SVGAnimateTransformElement,
                                       public SVGAnimationElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGAnimateTransformElementImpl() {}

protected:


};





/*#########################################################################
## SVGFontElementImpl
#########################################################################*/

/**
 *
 */
class SVGFontElementImpl :  virtual public SVGFontElement,
                            public SVGElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFontElementImpl() {}

protected:


};





/*#########################################################################
## SVGGlyphElementImpl
#########################################################################*/

/**
 *
 */
class SVGGlyphElementImpl : virtual public SVGGlyphElement,
                            public SVGElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGGlyphElementImpl() {}

protected:


};





/*#########################################################################
## SVGMissingGlyphElementImpl
#########################################################################*/

/**
 *
 */
class SVGMissingGlyphElementImpl : virtual public SVGMissingGlyphElement,
                                   public SVGElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGMissingGlyphElementImpl() {}

protected:


};





/*#########################################################################
## SVGHKernElementImpl
#########################################################################*/

/**
 *
 */
class SVGHKernElementImpl : virtual public SVGHKernElement,
                            public SVGElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGHKernElementImpl() {}

protected:


};





/*#########################################################################
## SVGVKernElementImpl
#########################################################################*/

/**
 *
 */
class SVGVKernElementImpl : virtual public SVGVKernElement,
                            public SVGElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGVKernElementImpl() {}

protected:


};





/*#########################################################################
## SVGFontFaceElementImpl
#########################################################################*/

/**
 *
 */
class SVGFontFaceElementImpl : virtual public SVGFontFaceElement,
                               public SVGElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFontFaceElementImpl() {}

protected:


};





/*#########################################################################
## SVGFontFaceSrcElementImpl
#########################################################################*/

/**
 *
 */
class SVGFontFaceSrcElementImpl : virtual public SVGFontFaceSrcElement,
                                  public SVGElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFontFaceSrcElementImpl() {}

protected:


};





/*#########################################################################
## SVGFontFaceUriElementImpl
#########################################################################*/

/**
 *
 */
class SVGFontFaceUriElementImpl : virtual public SVGFontFaceUriElement,
                                  public SVGElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFontFaceUriElementImpl() {}

protected:


};





/*#########################################################################
## SVGFontFaceFormatElementImpl
#########################################################################*/

/**
 *
 */
class SVGFontFaceFormatElementImpl : virtual public SVGFontFaceFormatElement,
                                     public SVGElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFontFaceFormatElementImpl() {}

protected:


};





/*#########################################################################
## SVGFontFaceNameElementImpl
#########################################################################*/

/**
 *
 */
class SVGFontFaceNameElementImpl : virtual public SVGFontFaceNameElement,
                                   public SVGElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGFontFaceNameElementImpl() {}

protected:


};





/*#########################################################################
## SVGDefinitionSrcElementImpl
#########################################################################*/

/**
 *
 */
class SVGDefinitionSrcElementImpl : virtual public SVGDefinitionSrcElement,
                                    public SVGElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGDefinitionSrcElementImpl() {}

protected:


};





/*#########################################################################
## SVGMetadataElementImpl
#########################################################################*/

/**
 *
 */
class SVGMetadataElementImpl : virtual public SVGMetadataElement,
                               public SVGElementImpl
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    virtual ~SVGMetadataElementImpl() {}

protected:


};




/*#########################################################################
## SVGForeignObjectElementImpl
#########################################################################*/

/**
 *
 */
class SVGForeignObjectElementImpl :  virtual public SVGForeignObjectElement,
                                     public SVGElementImpl
{
public:


    /**
     *
     */
    virtual SVGAnimatedLength getX()
        { return x; }

    /**
     *
     */
    virtual SVGAnimatedLength getY()
        { return y; }

    /**
     *
     */
    virtual SVGAnimatedLength getWidth()
        { return width; }

    /**
     *
     */
    virtual SVGAnimatedLength getHeight()
        { return height; }



    //##################
    //# Non-API methods
    //##################


    /**
     *
     */
    virtual ~SVGForeignObjectElementImpl() {}

protected:

    SVGAnimatedLength x, y, width, height;
};






}  //namespace svg
}  //namespace dom
}  //namespace w3c
}  //namespace org

#endif // __SVG_H__
/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/

