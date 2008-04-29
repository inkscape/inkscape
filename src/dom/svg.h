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
 *
 * =======================================================================
 * NOTES
 *
 * This API follows:
 * http://www.w3.org/TR/SVG11/svgdom.html
 *
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
 * All of the SVG DOM interfaces that correspond directly to elements in the SVG
 * language (e.g., the SVGPathElement interface corresponds directly to the
 * 'path' element in the language) are derivative from base class SVGElement.
 */
class SVGElement : virtual public Element
{
public:

    /**
     * Get the value of the id attribute on the given element.
     */
    virtual DOMString getId() =0;

    /**
     * Set the value of the id attribute on the given element.
     */
    virtual void setId(const DOMString &val)
                       throw (DOMException) =0;

    /**
     * Corresponds to attribute xml:base on the given element.
     */
    virtual DOMString getXmlBase() = 0;

    /**
     * Corresponds to attribute xml:base on the given element.
     */
    virtual void setXmlBase(const DOMString &val)
                            throw (DOMException) = 0;

    /**
     * The nearest ancestor 'svg' element. Null if the given element is the
     *      outermost 'svg' element.
     */
    virtual SVGSVGElementPtr getOwnerSVGElement() = 0;

    /**
     * The element which established the current viewport. Often, the nearest
     * ancestor 'svg' element. Null if the given element is the outermost 'svg'
     * element.
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
 * When an 'svg' element is embedded inline as a component of a document from
 * another namespace, such as when an 'svg' element is embedded inline within an
 * XHTML document [XHTML], then an SVGDocument object will not exist; instead,
 * the root object in the document object hierarchy will be a Document object of
 * a different type, such as an HTMLDocument object.
 *
 * However, an SVGDocument object will indeed exist when the root element of the
 * XML document hierarchy is an 'svg' element, such as when viewing a stand-alone
 * SVG file (i.e., a file with MIME type "image/svg+xml"). In this case, the
 * SVGDocument object will be the root object of the document object model
 * hierarchy.
 *
 * In the case where an SVG document is embedded by reference, such as when an
 * XHTML document has an 'object' element whose href attribute references an SVG
 * document (i.e., a document whose MIME type is "image/svg+xml" and whose root
 * element is thus an 'svg' element), there will exist two distinct DOM
 * hierarchies. The first DOM hierarchy will be for the referencing document
 * (e.g., an XHTML document). The second DOM hierarchy will be for the referenced
 * SVG document. In this second DOM hierarchy, the root object of the document
 * object model hierarchy is an SVGDocument object.
 */
class SVGDocument : virtual public Document,
                    virtual public events::DocumentEvent
{
public:


    /**
     * The title of a document as specified by the title sub-element of the 'svg'
     * root element (i.e., <svg><title>Here is the title</title>...</svg>)
     */
    virtual DOMString getTitle() =0;

    /**
     * Returns the URI of the page that linked to this page. The value is an empty
     * string if the user navigated to the page directly (not through a link, but,
     * for example, via a bookmark).
     */
    virtual DOMString getReferrer() =0;

    /**
     * The domain name of the server that served the document, or a null string if
     * the server cannot be identified by a domain name.
     */
    virtual DOMString getDomain() =0;

    /**
     * The complete URI of the document.
     */
    virtual DOMString getURL() =0;

    /**
     * The root 'svg'  element in the document hierarchy.
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
 * A key interface definition is the SVGSVGElement interface, which is the
 * interface that corresponds to the 'svg' element. This interface contains
 * various miscellaneous commonly-used utility methods, such as matrix operations
 * and the ability to control the time of redraw on visual rendering devices.
 *
 * SVGSVGElement extends ViewCSS and DocumentCSS to provide access to the
 * computed values of properties and the override style sheet as described in DOM2.
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
     * Corresponds to attribute x on the given 'svg' element.
     */
    virtual SVGAnimatedLength getX() =0;

    /**
     * Corresponds to attribute y on the given 'svg' element.
     */
    virtual SVGAnimatedLength getY() =0;

    /**
     * Corresponds to attribute width on the given 'svg' element.
     */
    virtual SVGAnimatedLength getWidth() =0;

    /**
     * Corresponds to attribute height on the given 'svg' element.
     */
    virtual SVGAnimatedLength getHeight() =0;

    /**
     * Get the attribute contentScriptType on the given 'svg' element.
     */
    virtual DOMString getContentScriptType() =0;

    /**
     * Set the attribute contentScriptType on the given 'svg' element.
     */
    virtual void setContentScriptType(const DOMString &val)
                                     throw (DOMException) =0;


    /**
     * Get the attribute contentStyleType on the given 'svg' element.
     */
    virtual DOMString getContentStyleType() =0;

    /**
     * Set the attribute contentStyleType on the given 'svg' element.
     */
    virtual void setContentStyleType(const DOMString &val)
                                     throw (DOMException) =0;

    /**
     * The position and size of the viewport (implicit or explicit) that corresponds
     * to this 'svg' element. When the user agent is actually rendering the content,
     * then the position and size values represent the actual values when rendering.
     * The position and size values are unitless values in the coordinate system of
     * the parent element. If no parent element exists (i.e., 'svg' element
     * represents the root of the document tree), if this SVG document is embedded as
     * part of another document (e.g., via the HTML 'object' element), then the
     * position and size are unitless values in the coordinate system of the parent
     * document. (If the parent uses CSS or XSL layout, then unitless values
     * represent pixel units for the current CSS or XSL viewport, as described in the
     * CSS2 specification.) If the parent element does not have a coordinate system,
     * then the user agent should provide reasonable default values for this attribute.
     *      */
    virtual SVGRect getViewport() =0;

    /**
     * Size of a pixel units (as defined by CSS2) along the x-axis of the viewport,
     * which represents a unit somewhere in the range of 70dpi to 120dpi, and, on
     * systems that support this, might actually match the characteristics of the
     * target medium. On systems where it is impossible to know the size of a pixel,
     * a suitable default pixel size is provided.
     */
    virtual double getPixelUnitToMillimeterX() =0;

    /**
     * Corresponding size of a pixel unit along the y-axis of the viewport.
     */
    virtual double getPixelUnitToMillimeterY() =0;

    /**
     * User interface (UI) events in DOM Level 2 indicate the screen positions at
     * which the given UI event occurred. When the user agent actually knows the
     * physical size of a "screen unit", this attribute will express that information;
     *  otherwise, user agents will provide a suitable default value such as .28mm.
     */
    virtual double getScreenPixelToMillimeterX() =0;

    /**
     * Corresponding size of a screen pixel along the y-axis of the viewport.
     */
    virtual double getScreenPixelToMillimeterY() =0;


    /**
     * The initial view (i.e., before magnification and panning) of the current
     * innermost SVG document fragment can be either the "standard" view (i.e., based
     * on attributes on the 'svg' element such as fitBoxToViewport) or to a "custom"
     * view (i.e., a hyperlink into a particular 'view' or other element - see
     * Linking into SVG content: URI fragments and SVG views). If the initial view is
     * the "standard" view, then this attribute is false. If the initial view is a
     * "custom" view, then this attribute is true.
     */
    virtual bool getUseCurrentView() =0;

    /**
     * Set the value above
     */
    virtual void setUseCurrentView(bool val) throw (DOMException) =0;

    /**
     * The definition of the initial view (i.e., before magnification and panning) of
     * the current innermost SVG document fragment. The meaning depends on the
     * situation:
     * 
     *    * If the initial view was a "standard" view, then:
     *      o the values for viewBox, preserveAspectRatio and zoomAndPan within
     *        currentView will match the values for the corresponding DOM attributes that
     *        are on SVGSVGElement directly
     *      o the values for transform and viewTarget within currentView will be null
     *    * If the initial view was a link into a 'view' element, then:
     *      o the values for viewBox, preserveAspectRatio and zoomAndPan within
     *        currentView will correspond to the corresponding attributes for the given
     *        'view' element
     *      o the values for transform and viewTarget within currentView will be null
     *    * If the initial view was a link into another element (i.e., other than a
     *      'view'), then:
     *      o the values for viewBox, preserveAspectRatio and zoomAndPan within
     *        currentView will match the values for the corresponding DOM attributes that
     *        are on SVGSVGElement directly for the closest ancestor 'svg' element
     *      o the values for transform within currentView will be null
     *      o the viewTarget within currentView will represent the target of the link
     *    * If the initial view was a link into the SVG document fragment using an SVG
     *      view specification fragment identifier (i.e., #svgView(...)), then:
     *      o the values for viewBox, preserveAspectRatio, zoomAndPan, transform and
     *        viewTarget within currentView will correspond to the values from the SVG view
     *        specification fragment identifier
     * 
     */
    virtual SVGViewSpec getCurrentView() =0;


    /**
     * This attribute indicates the current scale factor relative to the initial view
     * to take into account user magnification and panning operations, as described
     * under Magnification and panning. DOM attributes currentScale and
     * currentTranslate are equivalent to the 2x3 matrix [a b c d e f] =
     * [currentScale 0 0 currentScale currentTranslate.x currentTranslate.y]. If
     * "magnification" is enabled (i.e., zoomAndPan="magnify"), then the effect is as
     * if an extra transformation were placed at the outermost level on the SVG
     * document fragment (i.e., outside the outermost 'svg' element).
     */
    virtual double getCurrentScale() =0;

    /**
     *  Set the value above.
     */
    virtual void setCurrentScale(double val)
                                 throw (DOMException) =0;


    /**
     * The corresponding translation factor that takes into account
     *      user "magnification".
     */
    virtual SVGPoint getCurrentTranslate() =0;


    /**
     * Takes a time-out value which indicates that redraw shall not occur until: (a)
     * the corresponding unsuspendRedraw(suspend_handle_id) call has been made, (b)
     * an unsuspendRedrawAll() call has been made, or (c) its timer has timed out. In
     * environments that do not support interactivity (e.g., print media), then
     * redraw shall not be suspended. suspend_handle_id =
     * suspendRedraw(max_wait_milliseconds) and unsuspendRedraw(suspend_handle_id)
     * must be packaged as balanced pairs. When you want to suspend redraw actions as
     * a collection of SVG DOM changes occur, then precede the changes to the SVG DOM
     * with a method call similar to suspend_handle_id =
     * suspendRedraw(max_wait_milliseconds) and follow the changes with a method call
     * similar to unsuspendRedraw(suspend_handle_id). Note that multiple
     * suspendRedraw calls can be used at once and that each such method call is
     * treated independently of the other suspendRedraw method calls.
     */
    virtual unsigned long suspendRedraw (unsigned long max_wait_milliseconds ) =0;

    /**
     * Cancels a specified suspendRedraw() by providing a unique suspend_handle_id.
     */
    virtual void unsuspendRedraw (unsigned long suspend_handle_id )
                                  throw( DOMException ) =0;

    /**
     * Cancels all currently active suspendRedraw() method calls. This method is most
     * useful at the very end of a set of SVG DOM calls to ensure that all pending
     * suspendRedraw() method calls have been cancelled.
     */
    virtual void unsuspendRedrawAll (  ) =0;

    /**
     * In rendering environments supporting interactivity, forces the user agent to
     * immediately redraw all regions of the viewport that require updating.
     */
    virtual void forceRedraw (  ) =0;

    /**
     * Suspends (i.e., pauses) all currently running animations that are defined
     * within the SVG document fragment corresponding to this 'svg' element, causing
     * the animation clock corresponding to this document fragment to stand still
     * until it is unpaused.
     */
    virtual void pauseAnimations (  ) =0;

    /**
     * Unsuspends (i.e., unpauses) currently running animations that are defined
     * within the SVG document fragment, causing the animation clock to continue from
     * the time at which it was suspended.
     */
    virtual void unpauseAnimations (  ) =0;

    /**
     * Returns true if this SVG document fragment is in a paused state.
     */
    virtual bool animationsPaused (  ) =0;

    /**
     * Returns the current time in seconds relative to the start time for
     *      the current SVG document fragment.
     */
    virtual double getCurrentTime (  ) =0;

    /**
     * Adjusts the clock for this SVG document fragment, establishing
     *      a new current time.
     */
    virtual void setCurrentTime (double seconds ) =0;

    /**
     * Returns the list of graphics elements whose rendered content intersects the
     * supplied rectangle, honoring the 'pointer-events' property value on each
     * candidate graphics element.
     */
    virtual NodeList getIntersectionList(const SVGRect &rect,
                                         const SVGElementPtr referenceElement ) =0;

    /**
     * Returns the list of graphics elements whose rendered content is entirely
     * contained within the supplied rectangle, honoring the 'pointer-events'
     * property value on each candidate graphics element.
     */
    virtual NodeList getEnclosureList (const SVGRect &rect,
                                       const SVGElementPtr referenceElement ) =0;

    /**
     * Returns true if the rendered content of the given element intersects the
     * supplied rectangle, honoring the 'pointer-events' property value on each
     * candidate graphics element.
     */
    virtual bool checkIntersection (const SVGElementPtr element, const SVGRect &rect ) =0;

    /**
     * Returns true if the rendered content of the given element is entirely
     * contained within the supplied rectangle, honoring the 'pointer-events'
     * property value on each candidate graphics element.
     */
    virtual bool checkEnclosure (const SVGElementPtr element, const SVGRect &rect ) =0;

    /**
     * Unselects any selected objects, including any selections of text
     *      strings and type-in bars.
     */
    virtual void deselectAll (  ) =0;

    /**
     * Creates an SVGNumber object outside of any document trees. The object
     *      is initialized to a value of zero.
     */
    virtual SVGNumber createSVGNumber (  ) =0;

    /**
     * Creates an SVGLength object outside of any document trees. The object
     *      is initialized to the value of 0 user units.
     */
    virtual SVGLength createSVGLength (  ) =0;

    /**
     * Creates an SVGAngle object outside of any document trees. The object
     *      is initialized to the value 0 degrees (unitless).
     */
    virtual SVGAngle createSVGAngle (  ) =0;

    /**
     * Creates an SVGPoint object outside of any document trees. The object
     * is initialized to the point (0,0) in the user coordinate system.
     */
    virtual SVGPoint createSVGPoint (  ) =0;

    /**
     * Creates an SVGMatrix object outside of any document trees. The object
     *      is initialized to the identity matrix.
     */
    virtual SVGMatrix createSVGMatrix (  ) =0;

    /**
     * Creates an SVGRect object outside of any document trees. The object
     *      is initialized such that all values are set to 0 user units.
     */
    virtual SVGRect createSVGRect (  ) =0;

    /**
     * Creates an SVGTransform object outside of any document trees.
     * The object is initialized to an identity matrix transform
     *      (SVG_TRANSFORM_MATRIX).
     */
    virtual SVGTransform createSVGTransform (  ) =0;

    /**
     * Creates an SVGTransform object outside of any document trees.
     * The object is initialized to the given matrix transform
     *      (i.e., SVG_TRANSFORM_MATRIX).
     */
    virtual SVGTransform createSVGTransformFromMatrix(const SVGMatrix &matrix ) =0;

    /**
     * Searches this SVG document fragment (i.e., the search is restricted to a
     * subset of the document tree) for an Element whose id is given by elementId. If
     * an Element is found, that Element is returned. If no such element exists,
     * returns null. Behavior is not defined if more than one element has this id.
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
 * The SVGGElement  interface corresponds to the 'g' element.
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
 * The SVGDefsElement  interface corresponds to the 'defs' element.
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
 * The SVGDescElement  interface corresponds to the 'desc' element.
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
 * The SVGTitleElement  interface corresponds to the 'title' element.
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
 * The SVGSymbolElement  interface corresponds to the 'symbol' element.
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
 * The SVGUseElement  interface corresponds to the 'use' element.
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
     * Corresponds to attribute x on the given 'use' element.
     */
    virtual SVGAnimatedLength getX() =0;

    /**
     * Corresponds to attribute y on the given 'use' element.
     */
    virtual SVGAnimatedLength getY() =0;

    /**
     * Corresponds to attribute width on the given 'use' element.
     */
    virtual SVGAnimatedLength getWidth() =0;

    /**
     * Corresponds to attribute height on the given 'use' element.
     */
    virtual SVGAnimatedLength getHeight() =0;

    /**
     * The root of the "instance tree". See description of SVGElementInstance for
     * a discussion on the instance tree.
     *      */
    virtual SVGElementInstance getInstanceRoot() =0;

    /**
     * If the 'href' attribute is being animated, contains the current animated root
     * of the "instance tree". If the 'href' attribute is not currently being
     * animated, contains the same value as 'instanceRoot'. The root of the "instance
     * tree". See description of SVGElementInstance for a discussion on the instance
     * tree.
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
 * The SVGImageElement interface corresponds to the 'image' element.
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
     * Corresponds to attribute x on the given 'image' element.
     */
    virtual SVGAnimatedLength getX() =0;

    /**
     * Corresponds to attribute y on the given 'image' element.
     */
    virtual SVGAnimatedLength getY() =0;

    /**
     * Corresponds to attribute width on the given 'image' element.
     */
    virtual SVGAnimatedLength getWidth() =0;

    /**
     * Corresponds to attribute height on the given 'image' element.
     */
    virtual SVGAnimatedLength getHeight() =0;


    /**
     * Corresponds to attribute preserveAspectRatio on the given element.
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
 * The SVGSwitchElement  interface corresponds to the 'switch' element.
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
 * In the case where an SVG document is embedded by reference, such as when an
 * XHTML document has an 'object' element whose href (or equivalent) attribute
 * references an SVG document (i.e., a document whose MIME type is
 * "image/svg+xml" and whose root element is thus an 'svg' element), the SVG user
 * agent is required to implement the GetSVGDocument interface for the element
 * which references the SVG document (e.g., the HTML 'object' or comparable
 * referencing elements).
 */
class GetSVGDocument
{
public:

    /**
     * Returns the SVGDocument  object for the referenced SVG document.
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
 * The SVGStyleElement interface corresponds to the 'style' element.
 */
class SVGStyleElement : virtual public SVGElement
{
public:

    /**
     * Get the attribute xml:space on the given element.
     */
    virtual DOMString getXmlspace() = 0;

    /**
     * Set the attribute xml:space on the given element.
     */
    virtual void setXmlspace(const DOMString &val)
                             throw (DOMException) =0;

    /**
     * Get the attribute type on the given 'style' element.
     */
    virtual DOMString getType() = 0;

    /**
     * Set the attribute type on the given 'style' element.
     */
    virtual void setType(const DOMString &val)
                         throw (DOMException) =0;

    /**
     * Get the attribute media on the given 'style' element.
     */
    virtual DOMString getMedia() = 0;

    /**
     * Set the attribute media on the given 'style' element.
     */
    virtual void setMedia(const DOMString &val)
                          throw (DOMException) =0;

    /**
     * Get the attribute title on the given 'style' element.
     */
    virtual DOMString getTitle() = 0;

    /**
     * Set the attribute title on the given 'style' element.
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
 * The SVGPathElement  interface corresponds to the 'path' element.
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
     * Corresponds to attribute pathLength on the given 'path' element.
     */
    virtual SVGAnimatedNumber getPathLength() =0;

    /**
     * Returns the user agent's computed value for the total length of the path using
     * the user agent's distance-along-a-path algorithm, as a distance in the current
     * user coordinate system.
     */
    virtual double getTotalLength (  ) =0;

    /**
     * Returns the (x,y) coordinate in user space which is distance units along the
     * path, utilizing the user agent's distance-along-a-path algorithm.
     */
    virtual SVGPoint getPointAtLength (double distance ) =0;

    /**
     * Returns the index into pathSegList which is distance units along the path,
     * utilizing the user agent's distance-along-a-path algorithm.
     */
    virtual unsigned long getPathSegAtLength (double distance ) =0;

    /**
     * Returns a stand-alone, parentless SVGPathSegClosePath object.
     */
    virtual SVGPathSegClosePath
              createSVGPathSegClosePath (  ) =0;

    /**
     * Returns a stand-alone, parentless SVGPathSegMovetoAbs object.
     */
    virtual SVGPathSegMovetoAbs
              createSVGPathSegMovetoAbs (double x, double y ) =0;

    /**
     * Returns a stand-alone, parentless SVGPathSegMovetoRel object.
     */
    virtual SVGPathSegMovetoRel
              createSVGPathSegMovetoRel (double x, double y ) =0;

    /**
     * Returns a stand-alone, parentless SVGPathSegLinetoAbs object.
     */
    virtual SVGPathSegLinetoAbs
              createSVGPathSegLinetoAbs (double x, double y ) =0;

    /**
     * Returns a stand-alone, parentless SVGPathSegLinetoRel object.
     */
    virtual SVGPathSegLinetoRel
              createSVGPathSegLinetoRel (double x, double y ) =0;

    /**
     * Returns a stand-alone, parentless SVGPathSegCurvetoCubicAbs object.
     */
    virtual SVGPathSegCurvetoCubicAbs
              createSVGPathSegCurvetoCubicAbs (double x, double y,
                        double x1, double y1, double x2, double y2 ) =0;

    /**
     * Returns a stand-alone, parentless SVGPathSegCurvetoCubicRel object.
     */
    virtual SVGPathSegCurvetoCubicRel
              createSVGPathSegCurvetoCubicRel (double x, double y,
                        double x1, double y1, double x2, double y2 ) =0;

    /**
     * Returns a stand-alone, parentless SVGPathSegCurvetoQuadraticAbs object.
     */
    virtual SVGPathSegCurvetoQuadraticAbs
              createSVGPathSegCurvetoQuadraticAbs (double x, double y,
                         double x1, double y1 ) =0;

    /**
     * Returns a stand-alone, parentless SVGPathSegCurvetoQuadraticRel object.
     */
    virtual SVGPathSegCurvetoQuadraticRel
              createSVGPathSegCurvetoQuadraticRel (double x, double y,
                         double x1, double y1 ) =0;

    /**
     * Returns a stand-alone, parentless SVGPathSegArcAbs object.
     */
    virtual SVGPathSegArcAbs
              createSVGPathSegArcAbs (double x, double y,
                         double r1, double r2, double angle,
                         bool largeArcFlag, bool sweepFlag ) =0;

    /**
     * Returns a stand-alone, parentless SVGPathSegArcRel object.
     */
    virtual SVGPathSegArcRel
              createSVGPathSegArcRel (double x, double y, double r1,
                         double r2, double angle, bool largeArcFlag,
                         bool sweepFlag ) =0;

    /**
     * Returns a stand-alone, parentless SVGPathSegLinetoHorizontalAbs object.
     */
    virtual SVGPathSegLinetoHorizontalAbs
              createSVGPathSegLinetoHorizontalAbs (double x ) =0;

    /**
     * Returns a stand-alone, parentless SVGPathSegLinetoHorizontalRel object.
     */
    virtual SVGPathSegLinetoHorizontalRel
              createSVGPathSegLinetoHorizontalRel (double x ) =0;

    /**
     * Returns a stand-alone, parentless SVGPathSegLinetoVerticalAbs object.
     */
    virtual SVGPathSegLinetoVerticalAbs
              createSVGPathSegLinetoVerticalAbs (double y ) =0;

    /**
     * Returns a stand-alone, parentless SVGPathSegLinetoVerticalRel object.
     */
    virtual SVGPathSegLinetoVerticalRel
              createSVGPathSegLinetoVerticalRel (double y ) =0;

    /**
     * Returns a stand-alone, parentless SVGPathSegCurvetoCubicSmoothAbs object.
     */
    virtual SVGPathSegCurvetoCubicSmoothAbs
              createSVGPathSegCurvetoCubicSmoothAbs (double x, double y,
                                             double x2, double y2 ) =0;

    /**
     * Returns a stand-alone, parentless SVGPathSegCurvetoCubicSmoothRel object.
     */
    virtual SVGPathSegCurvetoCubicSmoothRel
              createSVGPathSegCurvetoCubicSmoothRel (double x, double y,
                                                      double x2, double y2 ) =0;

    /**
     * Returns a stand-alone, parentless SVGPathSegCurvetoQuadraticSmoothAbs
     *      object.
     */
    virtual SVGPathSegCurvetoQuadraticSmoothAbs
              createSVGPathSegCurvetoQuadraticSmoothAbs (double x, double y ) =0;

    /**
     * Returns a stand-alone, parentless SVGPathSegCurvetoQuadraticSmoothRel
     *      object.
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
 * The SVGRectElement  interface corresponds to the 'rect' element.
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
     * Corresponds to attribute x on the given 'rect' element.
     */
    virtual SVGAnimatedLength getX() =0;

    /**
     * Corresponds to attribute y on the given 'rect' element.
     */
    virtual SVGAnimatedLength getY() =0;

    /**
     * Corresponds to attribute width on the given 'rect' element.
     */
    virtual SVGAnimatedLength getWidth() =0;

    /**
     * Corresponds to attribute height on the given 'rect' element.
     */
    virtual SVGAnimatedLength getHeight() =0;


    /**
     * Corresponds to attribute rx on the given 'rect' element.
     */
    virtual SVGAnimatedLength getRx() =0;

    /**
     * Corresponds to attribute ry on the given 'rect' element.
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
 * The SVGCircleElement  interface corresponds to the 'circle' element.
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
     * Corresponds to attribute cx on the given 'circle' element.
     */
    virtual SVGAnimatedLength getCx() =0;

    /**
     * Corresponds to attribute cy on the given 'circle' element.
     */
    virtual SVGAnimatedLength getCy() =0;

    /**
     * Corresponds to attribute r on the given 'circle' element.
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
 * The SVGEllipseElement  interface corresponds to the 'ellipse' element.
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
     * Corresponds to attribute cx on the given 'ellipse' element.
     */
    virtual SVGAnimatedLength getCx() =0;

    /**
     * Corresponds to attribute cy on the given 'ellipse' element.
     */
    virtual SVGAnimatedLength getCy() =0;

    /**
     * Corresponds to attribute rx on the given 'ellipse' element.
     */
    virtual SVGAnimatedLength getRx() =0;

    /**
     * Corresponds to attribute ry on the given 'ellipse' element.
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
 * The SVGLineElement  interface corresponds to the 'line' element.
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
     * Corresponds to attribute x1 on the given 'line' element.
     */
    virtual SVGAnimatedLength getX1() =0;

    /**
     * Corresponds to attribute y1 on the given 'line' element.
     */
    virtual SVGAnimatedLength getY1() =0;

    /**
     * Corresponds to attribute x2 on the given 'line' element.
     */
    virtual SVGAnimatedLength getX2() =0;

    /**
     * Corresponds to attribute y2 on the given 'line' element.
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
 * The SVGPolylineElement  interface corresponds to the 'polyline' element.
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
 * The SVGPolygonElement  interface corresponds to the 'polygon' element.
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
 * The SVGTextContentElement interface is inherited by various text-related
 * interfaces, such as SVGTextElement, SVGTSpanElement, SVGTRefElement,
 * SVGAltGlyphElement and SVGTextPathElement.
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
     * Corresponds to attribute textLength on the given element.
     */
    virtual SVGAnimatedLength getTextLength() =0;


    /**
     * Corresponds to attribute lengthAdjust on the given element. The value must be
     * one of the length adjust constants specified above.
     */
    virtual SVGAnimatedEnumeration getLengthAdjust() =0;


    /**
     * Returns the total number of characters to be rendered within the current
     * element. Includes characters which are included via a 'tref' reference.
     */
    virtual long getNumberOfChars (  ) =0;

    /**
     * The total sum of all of the advance values from rendering all of the
     * characters within this element, including the advance value on the glyphs
     * (horizontal or vertical), the effect of properties 'kerning', 'letter-spacing'
     * and 'word-spacing' and adjustments due to attributes dx and dy on 'tspan'
     * elements. For non-rendering environments, the user agent shall make reasonable
     * assumptions about glyph metrics.
     */
    virtual double getComputedTextLength (  ) =0;

    /**
     * The total sum of all of the advance values from rendering the specified
     * substring of the characters, including the advance value on the glyphs
     * (horizontal or vertical), the effect of properties 'kerning', 'letter-spacing'
     * and 'word-spacing' and adjustments due to attributes dx and dy on 'tspan'
     * elements. For non-rendering environments, the user agent shall make reasonable
     * assumptions about glyph metrics.
     */
    virtual double getSubStringLength (unsigned long charnum, unsigned long nchars )
                                     throw( DOMException ) =0;

    /**
     * Returns the current text position before rendering the character in the user
     * coordinate system for rendering the glyph(s) that correspond to the specified
     * character. The current text position has already taken into account the
     * effects of any inter-character adjustments due to properties 'kerning',
     * 'letter-spacing' and 'word-spacing' and adjustments due to attributes x, y, dx
     * and dy. If multiple consecutive characters are rendered inseparably (e.g., as
     * a single glyph or a sequence of glyphs), then each of the inseparable
     * characters will return the start position for the first glyph.
     */
    virtual SVGPoint getStartPositionOfChar (unsigned long charnum )
                                              throw( DOMException ) =0;

    /**
     * Returns the current text position after rendering the character in the user
     * coordinate system for rendering the glyph(s) that correspond to the specified
     * character. This current text position does not take into account the effects
     * of any inter-character adjustments to prepare for the next character, such as
     * properties 'kerning', 'letter-spacing' and 'word-spacing' and adjustments due
     * to attributes x, y, dx and dy. If multiple consecutive characters are rendered
     * inseparably (e.g., as a single glyph or a sequence of glyphs), then each of
     * the inseparable characters will return the end position for the last glyph.
     */
    virtual SVGPoint getEndPositionOfChar (unsigned long charnum )
                                           throw( DOMException ) =0;

    /**
     * Returns a tightest rectangle which defines the minimum and maximum X and Y
     * values in the user coordinate system for rendering the glyph(s) that
     * correspond to the specified character. The calculations assume that all glyphs
     * occupy the full standard glyph cell for the font. If multiple consecutive
     * characters are rendered inseparably (e.g., as a single glyph or a sequence of
     * glyphs), then each of the inseparable characters will return the same extent.
     */
    virtual SVGRect getExtentOfChar (unsigned long charnum )
                                      throw( DOMException ) =0;

    /**
     * Returns the rotation value relative to the current user coordinate system used
     * to render the glyph(s) corresponding to the specified character. If multiple
     * glyph(s) are used to render the given character and the glyphs each have
     * different rotations (e.g., due to text-on-a-path), the user agent shall return
     * an average value (e.g., the rotation angle at the midpoint along the path for
     * all glyphs used to render this character). The rotation value represents the
     * rotation that is supplemental to any rotation due to properties
     * 'glyph-orientation-horizontal' and 'glyph-orientation-vertical'; thus, any
     * glyph rotations due to these properties are not included into the returned
     * rotation value. If multiple consecutive characters are rendered inseparably
     * (e.g., as a single glyph or a sequence of glyphs), then each of the
     * inseparable characters will return the same rotation value.
     */
    virtual double getRotationOfChar (unsigned long charnum )
                                     throw( DOMException ) =0;

    /**
     * Returns the index of the character whose corresponding glyph cell bounding box
     * contains the specified point. The calculations assume that all glyphs occupy
     * the full standard glyph cell for the font. If no such character exists, a
     * value of -1 is returned. If multiple such characters exist, the character
     * within the element whose glyphs were rendered last (i.e., take into account
     * any reordering such as for bidirectional text) is used. If multiple
     * consecutive characters are rendered inseparably (e.g., as a single glyph or a
     * sequence of glyphs), then the user agent shall allocate an equal percentage of
     * the text advance amount to each of the contributing characters in determining
     * which of the characters is chosen.
     */
    virtual long getCharNumAtPosition (const SVGPoint &point ) =0;

    /**
     * Causes the specified substring to be selected just as if the user
     *      selected the substring interactively.
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
 * The SVGTextPositioningElement interface is inherited by text-related
 * interfaces: SVGTextElement, SVGTSpanElement, SVGTRefElement and
 * SVGAltGlyphElement.
 */
class SVGTextPositioningElement : virtual public SVGTextContentElement
{
public:

    /**
     * Corresponds to attribute x on the given element.
     */
    virtual SVGAnimatedLength getX() =0;

    /**
     * Corresponds to attribute y on the given element.
     */
    virtual SVGAnimatedLength getY() =0;

    /**
     * Corresponds to attribute dx on the given element.
     */
    virtual SVGAnimatedLength getDx() =0;

    /**
     * Corresponds to attribute dy on the given element.
     */
    virtual SVGAnimatedLength getDy() =0;


    /**
     * Corresponds to attribute rotate on the given element.
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
 * The SVGTextElement  interface corresponds to the 'text' element.
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
 * The SVGTSpanElement  interface corresponds to the 'tspan' element.
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
 * The SVGTRefElement  interface corresponds to the 'tref' element.
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
 * The SVGTextPathElement  interface corresponds to the 'textPath' element.
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
     * Corresponds to attribute startOffset on the given 'textPath' element.
     */
    virtual SVGAnimatedLength getStartOffset() =0;

    /**
     * Corresponds to attribute method on the given 'textPath' element. The value
     * must be one of the method type constants specified above.
     */
    virtual SVGAnimatedEnumeration getMethod() =0;

    /**
     * Corresponds to attribute spacing on the given 'textPath' element.
     *  The value must be one of the spacing type constants specified above.
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
 * The SVGAltGlyphElement  interface corresponds to the 'altGlyph' element.
 */
class SVGAltGlyphElement :
                    virtual public SVGTextPositioningElement,
                    public SVGURIReference
{
public:

    /**
     * Get the attribute glyphRef on the given element.
     */
    virtual DOMString getGlyphRef() =0;

    /**
     * Set the attribute glyphRef on the given element.
     */
    virtual void setGlyphRef(const DOMString &val)
                                     throw (DOMException) =0;

    /**
     * Get the attribute format on the given element.
     */
    virtual DOMString getFormat() =0;

    /**
     * Set the attribute format on the given element.
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
 * The SVGAltGlyphDefElement interface corresponds to the 'altGlyphDef' element.
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
 * The SVGAltGlyphItemElement  interface corresponds to the
 *  'altGlyphItem' element.
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
 * The SVGGlyphRefElement  interface corresponds to the 'glyphRef' element.
 */
class SVGGlyphRefElement : virtual public SVGElement,
                           public SVGURIReference,
                           public SVGStylable
{
public:

    /**
     * Get the attribute glyphRef on the given element.
     */
    virtual DOMString getGlyphRef() =0;

    /**
     * Set the attribute glyphRef on the given element.
     */
    virtual void setGlyphRef(const DOMString &val)
                             throw (DOMException) =0;

    /**
     * Get the attribute format on the given element.
     */
    virtual DOMString getFormat() =0;

    /**
     * Set the attribute format on the given element.
     */
    virtual void setFormat(const DOMString &val)
                           throw (DOMException) =0;

    /**
     * Get the attribute x on the given element.
     */
    virtual double getX() = 0;

    /**
     * Set the attribute x on the given element.
     */
    virtual void setX(double val) throw (DOMException) =0;

    /**
     * Get the attribute y on the given element.
     */
    virtual double getY() = 0;

    /**
     * Set the attribute y on the given element.
     */
    virtual void setY(double val) throw (DOMException) =0;

    /**
     * Get the attribute dx on the given element.
     */
    virtual double getDx() = 0;

    /**
     * Set the attribute dx on the given element.
     */
    virtual void setDx(double val) throw (DOMException) =0;

    /**
     * Get the attribute dy on the given element.
     */
    virtual double getDy() = 0;

    /**
     * Set the attribute dy on the given element.
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
 * The SVGMarkerElement  interface corresponds to the 'marker' element.
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
     * Corresponds to attribute refX on the given 'marker' element.
     */
    virtual SVGAnimatedLength getRefX() =0;

    /**
     * Corresponds to attribute refY on the given 'marker' element.
     */
    virtual SVGAnimatedLength getRefY() =0;

    /**
     * Corresponds to attribute markerUnits on the given 'marker' element.
     *      One of the Marker Units Types defined above.
     */
    virtual SVGAnimatedEnumeration getMarkerUnits() =0;

    /**
     * Corresponds to attribute markerWidth on the given 'marker' element.
     */
    virtual SVGAnimatedLength getMarkerWidth() =0;

    /**
     * Corresponds to attribute markerHeight on the given 'marker' element.
     */
    virtual SVGAnimatedLength getMarkerHeight() =0;

    /**
     * Corresponds to attribute orient on the given 'marker' element.
     *      One of the Marker Orientation Types defined above.
     */
    virtual SVGAnimatedEnumeration getOrientType() =0;

    /**
     * Corresponds to attribute orient on the given 'marker' element.
     * If markerUnits is SVG_MARKER_ORIENT_ANGLE, the angle value for
     * attribute orient; otherwise, it will be set to zero.
     */
    virtual SVGAnimatedAngle getOrientAngle() =0;


    /**
     * Sets the value of attribute orient to 'auto'.
     */
    virtual void setOrientToAuto (  ) =0;

    /**
     * Sets the value of attribute orient to the given angle.
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
 * The SVGColorProfileElement  interface corresponds to the
 *  'color-profile' element.
 */
class SVGColorProfileElement :
                    virtual public SVGElement,
                    public SVGURIReference,
                    public SVGRenderingIntent
{
public:

    /**
     * Get the attribute local on the given element.
     */
    virtual DOMString getLocal() =0;

    /**
     * Set the attribute local on the given element.
     */
    virtual void setLocal(const DOMString &val)
                          throw (DOMException) =0;

    /**
     * Get the attribute name on the given element.
     */
    virtual DOMString getName() =0;

    /**
     * Set the attribute name on the given element.
     */
    virtual void setName(const DOMString &val)
                         throw (DOMException) =0;

    /**
     * Set the attribute rendering-intent on the given element.
     * The type of rendering intent, identified by one of the
     *      SVGRenderingIntent constants.
     */
    virtual unsigned short getRenderingIntent() =0;

    /**
     * Get the attribute rendering-intent on the given element.
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
 * The SVGGradientElement interface is a base interface used by
 * SVGLinearGradientElement and SVGRadialGradientElement.
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
     * Corresponds to attribute gradientUnits on the given element.
     *      Takes one of the constants defined in SVGUnitTypes.
     */
    virtual SVGAnimatedEnumeration getGradientUnits() =0;

    /**
     * Corresponds to attribute gradientTransform on the given element.
     */
    virtual SVGAnimatedTransformList getGradientTransform() =0;

    /**
     * Corresponds to attribute spreadMethod on the given element.
     *      One of the Spread Method Types.
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
 * The SVGLinearGradientElement  interface corresponds to the
 *  'linearGradient' element.
 */
class SVGLinearGradientElement : virtual public SVGGradientElement
{
public:


    /**
     * Corresponds to attribute x1 on the given 'linearGradient'  element.
     */
    virtual SVGAnimatedLength getX1() =0;

    /**
     * Corresponds to attribute y1 on the given 'linearGradient'  element.
     */
    virtual SVGAnimatedLength getY1() =0;

    /**
     * Corresponds to attribute x2 on the given 'linearGradient'  element.
     */
    virtual SVGAnimatedLength getX2() =0;

    /**
     * Corresponds to attribute y2 on the given 'linearGradient'  element.
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
 * The SVGRadialGradientElement  interface corresponds to the
 *  'radialGradient' element.
 */
class SVGRadialGradientElement : virtual public SVGGradientElement
{

public:

    /**
     * Corresponds to attribute cx on the given 'radialGradient'  element.
     */
    virtual SVGAnimatedLength getCx() =0;


    /**
     * Corresponds to attribute cy on the given 'radialGradient'  element.
     */
    virtual SVGAnimatedLength getCy() =0;


    /**
     * Corresponds to attribute r on the given 'radialGradient'  element.
     */
    virtual SVGAnimatedLength getR() =0;


    /**
     * Corresponds to attribute fx on the given 'radialGradient'  element.
     */
    virtual SVGAnimatedLength getFx() =0;


    /**
     * Corresponds to attribute fy on the given 'radialGradient'  element.
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
 * The SVGStopElement  interface corresponds to the 'stop' element.
 */
class SVGStopElement :
                    virtual public SVGElement,
                    public SVGStylable
{
public:

    /**
     * Corresponds to attribute offset on the given 'stop' element.
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
 * The SVGPatternElement  interface corresponds to the 'pattern' element.
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
     * Corresponds to attribute patternUnits on the given 'pattern' element. Takes
     * one of the constants defined in SVGUnitTypes.
     */
    virtual SVGAnimatedEnumeration getPatternUnits() =0;

    /**
     * Corresponds to attribute patternContentUnits on the given 'pattern'
     *      element. Takes one of the constants defined in SVGUnitTypes.
     */
    virtual SVGAnimatedEnumeration getPatternContentUnits() =0;

    /**
     * Corresponds to attribute patternTransform on the given 'pattern' element.
     */
    virtual SVGAnimatedTransformList getPatternTransform() =0;

    /**
     * Corresponds to attribute x on the given 'pattern' element.
     */
    virtual SVGAnimatedLength getX() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getY() =0;

    /**
     * Corresponds to attribute width on the given 'pattern' element.
     */
    virtual SVGAnimatedLength getWidth() =0;

    /**
     * Corresponds to attribute height on the given 'pattern' element.
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
 * The SVGClipPathElement  interface corresponds to the 'clipPath' element.
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
     * Corresponds to attribute clipPathUnits on the given 'clipPath' element.
     *      Takes one of the constants defined in SVGUnitTypes.
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
 * The SVGMaskElement  interface corresponds to the 'mask' element.
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
     * Corresponds to attribute maskUnits on the given 'mask' element. Takes one of
     * the constants defined in SVGUnitTypes.
     */
    virtual SVGAnimatedEnumeration getMaskUnits() =0;

    /**
     * Corresponds to attribute maskContentUnits on the given 'mask' element. Takes
     * one of the constants defined in SVGUnitTypes.
     */
    virtual SVGAnimatedEnumeration getMaskContentUnits() =0;

    /**
     * Corresponds to attribute x on the given 'mask' element.
     */
    virtual SVGAnimatedLength getX() =0;

    /**
     * Corresponds to attribute y on the given 'mask' element.
     */
    virtual SVGAnimatedLength getY() =0;

    /**
     * Corresponds to attribute width on the given 'mask' element.
     */
    virtual SVGAnimatedLength getWidth() =0;

    /**
     * Corresponds to attribute height on the given 'mask' element.
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
 * The SVGFilterElement  interface corresponds to the 'filter' element.
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
     * Corresponds to attribute filterUnits on the given 'filter' element. Takes one
     * of the constants defined in SVGUnitTypes.
     */
    virtual SVGAnimatedEnumeration getFilterUnits() =0;

    /**
     * Corresponds to attribute primitiveUnits on the given 'filter' element. Takes
     * one of the constants defined in SVGUnitTypes.
     */
    virtual SVGAnimatedEnumeration getPrimitiveUnits() =0;

    /**
     *
     */
    virtual SVGAnimatedLength getX() =0;

    /**
     * Corresponds to attribute x on the given 'filter' element.
     */
    virtual SVGAnimatedLength getY() =0;

    /**
     * Corresponds to attribute y on the given 'filter' element.
     */
    virtual SVGAnimatedLength getWidth() =0;

    /**
     * Corresponds to attribute height on the given 'filter' element.
     */
    virtual SVGAnimatedLength getHeight() =0;


    /**
     * Corresponds to attribute filterRes on the given 'filter' element.
     *      Contains the X component of attribute filterRes.
     */
    virtual SVGAnimatedInteger getFilterResX() =0;

    /**
     * Corresponds to attribute filterRes on the given 'filter' element.
     * Contains the Y component (possibly computed automatically)
     *      of attribute filterRes.
     */
    virtual SVGAnimatedInteger getFilterResY() =0;

    /**
     * Sets the values for attribute filterRes.
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
 * The SVGFEBlendElement  interface corresponds to the 'feBlend' element.
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
     * Corresponds to attribute in on the given 'feBlend' element.
     */
    virtual SVGAnimatedString getIn1() =0;

    /**
     * Corresponds to attribute in2 on the given 'feBlend' element.
     */
    virtual SVGAnimatedString getIn2() =0;

    /**
     * Corresponds to attribute mode on the given 'feBlend' element.
     *      Takes one of the Blend Mode Types.
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
 * The SVGFEColorMatrixElement  interface corresponds to the
 *  'feColorMatrix' element.
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
     * Corresponds to attribute in on the given 'feColorMatrix' element.
     */
    virtual SVGAnimatedString getIn1() =0;

    /**
     * Corresponds to attribute type on the given 'feColorMatrix' element.
     *      Takes one of the Color Matrix Types.
     */
    virtual SVGAnimatedEnumeration getType() =0;

    /**
     * Corresponds to attribute values on the given 'feColorMatrix' element.
     * Provides access to the contents of the values attribute.
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
 * The SVGFEComponentTransferElement  interface corresponds to
 *  the 'feComponentTransfer' element.
 */
class SVGFEComponentTransferElement :
                    virtual public SVGElement,
                    public SVGFilterPrimitiveStandardAttributes
{
public:

    /**
     * Corresponds to attribute in on the given 'feComponentTransfer'  element.
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
 * This interface defines a base interface used by the component
 *  transfer function interfaces.
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
     * Corresponds to attribute type on the given element. Takes one\
     *      of the Component Transfer Types.
     */
    virtual SVGAnimatedEnumeration getType() =0;

    /**
     * Corresponds to attribute tableValues on the given element.
     */
    virtual SVGAnimatedNumberList getTableValues() =0;

    /**
     * Corresponds to attribute slope on the given element.
     */
    virtual SVGAnimatedNumber getSlope() =0;

    /**
     * Corresponds to attribute intercept on the given element.
     */
    virtual SVGAnimatedNumber getIntercept() =0;

    /**
     * Corresponds to attribute amplitude on the given element.
     */
    virtual SVGAnimatedNumber getAmplitude() =0;

    /**
     * Corresponds to attribute exponent on the given element.
     */
    virtual SVGAnimatedNumber getExponent() =0;

    /**
     * Corresponds to attribute offset on the given element.
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
 * The SVGFEFuncRElement  interface corresponds to the 'feFuncR' element.
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
 * The SVGFEFuncGElement  interface corresponds to the 'feFuncG' element.
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
 * The SVGFEFuncBElement  interface corresponds to the 'feFuncB' element.
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
 * The SVGFEFuncAElement  interface corresponds to the 'feFuncA' element.
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
 * The SVGFECompositeElement interface corresponds to the 'feComposite' element.
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
     * Corresponds to attribute in on the given 'feComposite' element.
     */
    virtual SVGAnimatedString getIn1() =0;

    /**
     * Corresponds to attribute in2 on the given 'feComposite' element.
     */
    virtual SVGAnimatedString getIn2() =0;

    /**
     * Corresponds to attribute operator on the given 'feComposite' element.
     *      Takes one of the Composite Operators.
     */
    virtual SVGAnimatedEnumeration getOperator() =0;

    /**
     * Corresponds to attribute k1 on the given 'feComposite' element.
     */
    virtual SVGAnimatedNumber getK1() =0;

    /**
     * Corresponds to attribute k2 on the given 'feComposite' element.
     */
    virtual SVGAnimatedNumber getK2() =0;

    /**
     * Corresponds to attribute k3 on the given 'feComposite' element.
     */
    virtual SVGAnimatedNumber getK3() =0;

    /**
     * Corresponds to attribute k4 on the given 'feComposite' element.
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
 * The SVGFEConvolveMatrixElement  interface corresponds to
 *  the 'feConvolveMatrix' element.
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
     * Corresponds to attribute order on the given 'feConvolveMatrix'  element.
     */
    virtual SVGAnimatedInteger getOrderX() =0;

    /**
     * Corresponds to attribute order on the given 'feConvolveMatrix'  element.
     */
    virtual SVGAnimatedInteger getOrderY() =0;

    /**
     * Corresponds to attribute kernelMatrix on the given element.
     */
    virtual SVGAnimatedNumberList getKernelMatrix() =0;

    /**
     * Corresponds to attribute divisor on the given 'feConvolveMatrix' element.
     */
    virtual SVGAnimatedNumber getDivisor() =0;

    /**
     * Corresponds to attribute bias on the given 'feConvolveMatrix'  element.
     */
    virtual SVGAnimatedNumber getBias() =0;

    /**
     * Corresponds to attribute targetX on the given 'feConvolveMatrix'  element.
     */
    virtual SVGAnimatedInteger getTargetX() =0;

    /**
     * Corresponds to attribute targetY on the given 'feConvolveMatrix'  element.
     */
    virtual SVGAnimatedInteger getTargetY() =0;

    /**
     * Corresponds to attribute edgeMode on the given 'feConvolveMatrix'
     *      element. Takes one of the Edge Mode Types.
     */
    virtual SVGAnimatedEnumeration getEdgeMode() =0;

    /**
     * Corresponds to attribute kernelUnitLength on the
     *      given 'feConvolveMatrix'  element.
     */
    virtual SVGAnimatedLength getKernelUnitLengthX() =0;

    /**
     * Corresponds to attribute kernelUnitLength on the given
     *      'feConvolveMatrix'  element.
     */
    virtual SVGAnimatedLength getKernelUnitLengthY() =0;

    /**
     * Corresponds to attribute preserveAlpha on the
     *      given 'feConvolveMatrix'  element.
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
 * The SVGFEDiffuseLightingElement  interface corresponds to the
 *  'feDiffuseLighting' element.
 */
class SVGFEDiffuseLightingElement :
                    virtual public SVGElement,
                    public SVGFilterPrimitiveStandardAttributes
{
public:

    /**
     * Corresponds to attribute in on the given 'feDiffuseLighting'  element.
     */
    virtual SVGAnimatedString getIn1() =0;

    /**
     * Corresponds to attribute surfaceScale on the given
     *      'feDiffuseLighting'  element.
     */
    virtual SVGAnimatedNumber getSurfaceScale() =0;

    /**
     * Corresponds to attribute diffuseConstant on the given
     *      'feDiffuseLighting'  element.
     */
    virtual SVGAnimatedNumber getDiffuseConstant() =0;

    /**
     * Corresponds to attribute kernelUnitLength on the given
     *      'feDiffuseLighting'  element.
     */
    virtual SVGAnimatedNumber getKernelUnitLengthX() =0;

    /**
     * Corresponds to attribute kernelUnitLength on the given
     *      'feDiffuseLighting'  element.
     */
    virtual SVGAnimatedNumber getKernelUnitLengthY() =0;



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
 * The SVGFEDistantLightElement interface corresponds to the
 *  'feDistantLight' element.
 */
class SVGFEDistantLightElement : virtual public SVGElement
{
public:

    /**
     * Corresponds to attribute azimuth on the given 'feDistantLight'  element.
     */
    virtual SVGAnimatedNumber getAzimuth() =0;


    /**
     * Corresponds to attribute elevation on the given 'feDistantLight'
     *    element
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
 * The SVGFEPointLightElement interface corresponds to the 'fePointLight' element.
 */
class SVGFEPointLightElement : virtual public SVGElement
{
public:

    /**
     * Corresponds to attribute x on the given 'fePointLight' element.
     */
    virtual SVGAnimatedNumber getX() =0;

    /**
     * Corresponds to attribute y on the given 'fePointLight' element.
     */
    virtual SVGAnimatedNumber getY() =0;

    /**
     * Corresponds to attribute z on the given 'fePointLight' element.
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
 * The SVGFESpotLightElement interface corresponds to the 'feSpotLight' element.
 */
class SVGFESpotLightElement : virtual public SVGElement
{
public:

    /**
     * Corresponds to attribute x on the given 'feSpotLight' element.
     */
    virtual SVGAnimatedNumber getX() =0;

    /**
     * Corresponds to attribute y on the given 'feSpotLight' element.
     */
    virtual SVGAnimatedNumber getY() =0;

    /**
     * Corresponds to attribute z on the given 'feSpotLight' element.
     */
    virtual SVGAnimatedNumber getZ() =0;

    /**
     * Corresponds to attribute pointsAtX on the given 'feSpotLight' element.
     */
    virtual SVGAnimatedNumber getPointsAtX() =0;

    /**
     * Corresponds to attribute pointsAtY on the given 'feSpotLight' element.
     */
    virtual SVGAnimatedNumber getPointsAtY() =0;

    /**
     * Corresponds to attribute pointsAtZ on the given 'feSpotLight' element.
     */
    virtual SVGAnimatedNumber getPointsAtZ() =0;

    /**
     * Corresponds to attribute specularExponent on the
     *      given 'feSpotLight'  element.
     */
    virtual SVGAnimatedNumber getSpecularExponent() =0;

    /**
     * Corresponds to attribute limitingConeAngle on the
     *      given 'feSpotLight'  element.
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

