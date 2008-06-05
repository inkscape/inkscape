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

#include "svgimpl.h"
#include <cstring>
using std::strcmp;

/**
 * This is the .cpp side of the SVG implementations classes.  Note that many
 * of the sections for each of the classes is empty.   This is because that class
 * has been implemented totally in svgimpl.h
 */   

namespace org
{
namespace w3c
{
namespace dom
{
namespace svg
{


/*#########################################################################
## U T I L I T Y
#########################################################################*/

static DOMString d2s(double d)
{
   char buf[40];
   snprintf(buf, 40, "%f", d);
   DOMString s(buf);
   return s;
}

static double s2d(const DOMString &s)
{
    const char *begin = s.c_str();
    char *end;
    double val = strtod(begin, &end);
    if (end <= begin)
        return 0.0;
    return val;
}


/*#########################################################################
## Element type lookup table
#########################################################################*/

/**
 * Used for mapping name->enum and enum->name. For SVG element types.
 */
typedef struct
{
    const char *name;
    int         type;
} SVGElementTableEntry;



SVGElementTableEntry svgElementTable[] =
{
  { "a",                     SVG_A_ELEMENT                   },
  { "altGlyph",              SVG_ALTGLYPH_ELEMENT            },
  { "altGlyphDef",           SVG_ALTGLYPHDEF_ELEMENT         },
  { "altGlyphItem",          SVG_ALTGLYPHITEM_ELEMENT        },
  { "animate",               SVG_ANIMATE_ELEMENT             },
  { "animateColor",          SVG_ANIMATECOLOR_ELEMENT        },
  { "animateMotion",         SVG_ANIMATEMOTION_ELEMENT       },
  { "animateTransform",      SVG_ANIMATETRANSFORM_ELEMENT    },
  { "circle",                SVG_CIRCLE_ELEMENT              },
  { "clipPath",              SVG_CLIPPATH_ELEMENT            },
  { "color-profile",         SVG_COLOR_PROFILE_ELEMENT       },
  { "cursor",                SVG_CURSOR_ELEMENT              },
  { "definition-src",        SVG_DEFINITION_SRC_ELEMENT      },
  { "defs",                  SVG_DEFS_ELEMENT                },
  { "desc",                  SVG_DESC_ELEMENT                },
  { "ellipse",               SVG_ELLIPSE_ELEMENT             },
  { "feBlend",               SVG_FEBLEND_ELEMENT             },
  { "feColorMatrix",         SVG_FECOLORMATRIX_ELEMENT       },
  { "feComponentTransfer",   SVG_FECOMPONENTTRANSFER_ELEMENT },
  { "feComposite",           SVG_FECOMPOSITE_ELEMENT         },
  { "feConvolveMatrix",      SVG_FECONVOLVEMATRIX_ELEMENT    },
  { "feDiffuseLighting",     SVG_FEDIFFUSELIGHTING_ELEMENT   },
  { "feDisplacementMap",     SVG_FEDISPLACEMENTMAP_ELEMENT   },
  { "feDistantLight",        SVG_FEDISTANTLIGHT_ELEMENT      },
  { "feFlood",               SVG_FEFLOOD_ELEMENT             },
  { "feFuncA",               SVG_FEFUNCA_ELEMENT             },
  { "feFuncB",               SVG_FEFUNCB_ELEMENT             },
  { "feFuncG",               SVG_FEFUNCG_ELEMENT             },
  { "feFuncR",               SVG_FEFUNCR_ELEMENT             },
  { "feGaussianBlur",        SVG_FEGAUSSIANBLUR_ELEMENT      },
  { "feImage",               SVG_FEIMAGE_ELEMENT             },
  { "feMerge",               SVG_FEMERGE_ELEMENT             },
  { "feMergeNode",           SVG_FEMERGENODE_ELEMENT         },
  { "feMorphology",          SVG_FEMORPHOLOGY_ELEMENT        },
  { "feOffset",              SVG_FEOFFSET_ELEMENT            },
  { "fePointLight",          SVG_FEPOINTLIGHT_ELEMENT        },
  { "feSpecularLighting",    SVG_FESPECULARLIGHTING_ELEMENT  },
  { "feSpotLight",           SVG_FESPOTLIGHT_ELEMENT         },
  { "feTile",                SVG_FETILE_ELEMENT              },
  { "feTurbulence",          SVG_FETURBULENCE_ELEMENT        },
  { "filter",                SVG_FILTER_ELEMENT              },
  { "font",                  SVG_FONT_ELEMENT                },
  { "font-face",             SVG_FONT_FACE_ELEMENT           },
  { "font-face-format",      SVG_FONT_FACE_FORMAT_ELEMENT    },
  { "font-face-name",        SVG_FONT_FACE_NAME_ELEMENT      },
  { "font-face-src",         SVG_FONT_FACE_SRC_ELEMENT       },
  { "font-face-uri",         SVG_FONT_FACE_URI_ELEMENT       },
  { "foreignObject",         SVG_FOREIGNOBJECT_ELEMENT       },
  { "g",                     SVG_G_ELEMENT                   },
  { "glyph",                 SVG_GLYPH_ELEMENT               },
  { "glyphRef",              SVG_GLYPHREF_ELEMENT            },
  { "hkern",                 SVG_HKERN_ELEMENT               },
  { "image",                 SVG_IMAGE_ELEMENT               },
  { "line",                  SVG_LINE_ELEMENT                },
  { "linearGradient",        SVG_LINEARGRADIENT_ELEMENT      },
  { "marker",                SVG_MARKER_ELEMENT              },
  { "mask",                  SVG_MASK_ELEMENT                },
  { "metadata",              SVG_METADATA_ELEMENT            },
  { "missing-glyph",         SVG_MISSING_GLYPH_ELEMENT       },
  { "mpath",                 SVG_MPATH_ELEMENT               },
  { "path",                  SVG_PATH_ELEMENT                },
  { "pattern",               SVG_PATTERN_ELEMENT             },
  { "polygon",               SVG_POLYGON_ELEMENT             },
  { "polyline",              SVG_POLYLINE_ELEMENT            },
  { "radialGradient",        SVG_RADIALGRADIENT_ELEMENT      },
  { "rect",                  SVG_RECT_ELEMENT                },
  { "script",                SVG_SCRIPT_ELEMENT              },
  { "set",                   SVG_SET_ELEMENT                 },
  { "stop",                  SVG_STOP_ELEMENT                },
  { "style",                 SVG_STYLE_ELEMENT               },
  { "svg",                   SVG_SVG_ELEMENT                 },
  { "switch",                SVG_SWITCH_ELEMENT              },
  { "symbol",                SVG_SYMBOL_ELEMENT              },
  { "text",                  SVG_TEXT_ELEMENT                },
  { "textPath",              SVG_TEXTPATH_ELEMENT            },
  { "title",                 SVG_TITLE_ELEMENT               },
  { "tref",                  SVG_TREF_ELEMENT                },
  { "tspan",                 SVG_TSPAN_ELEMENT               },
  { "use",                   SVG_USE_ELEMENT                 },
  { "view",                  SVG_VIEW_ELEMENT                },
  { "vkern",                 SVG_VKERN_ELEMENT               }
  };


static int _entryComparison(const void *vkey, const void *ventry)
{
    const char *key = (const char *)vkey;
    const SVGElementTableEntry *entry = (const SVGElementTableEntry *)ventry;
    return strcmp(key, entry->name);
}

/**
 * Look up the SVG Element type enum for a given string
 * Return -1 if not found
 */
int svgElementStrToEnum(const char *str)
{
    if (!str)
        return -1;
    SVGElementTableEntry *entry = 
           (SVGElementTableEntry *)bsearch(str, svgElementTable,
            SVG_MAX_ELEMENT, sizeof(SVGElementTableEntry), _entryComparison);
    if (!entry)
        return -1;
    return entry->type;
}


/**
 * Return the string corresponding to a given SVG element type enum
 * Return "unknown" if not found
 */
const char *svgElementEnumToStr(int type)
{
    if (type < 0 || type >= SVG_MAX_ELEMENT)
        return "unknown";
    return svgElementTable[type].name;
}


/*#########################################################################
## SVGElementImpl
#########################################################################*/


//##################
//# Non-API methods
//##################




/*#########################################################################
## SVGDocumentImpl
#########################################################################*/



//####################################################
//# Overload some createXXX() methods from DocumentImpl,
//# To create our SVG-DOM types
//####################################################

/**
 *
 */
ElementPtr SVGDocumentImpl::createElement(const DOMString& tagName)
                           throw(DOMException)
{
    ElementPtr elem;
    int elementType = svgElementStrToEnum(tagName.c_str());
    switch (elementType)
        {
        case SVG_A_ELEMENT:
            {
            elem = new SVGAElementImpl();
            break;
            }                  
        case SVG_ALTGLYPH_ELEMENT:
            {
            elem = new SVGAltGlyphElementImpl();
            break;
            }                  
        case SVG_ALTGLYPHDEF_ELEMENT:
            {
            elem = new SVGAltGlyphDefElementImpl();
            break;
            }                  
        case SVG_ALTGLYPHITEM_ELEMENT:
            {
            elem = new SVGAltGlyphItemElementImpl();
            break;
            }                  
        case SVG_ANIMATE_ELEMENT:
            {
            elem = new SVGAnimateElementImpl();
            break;
            }                  
        case SVG_ANIMATECOLOR_ELEMENT:
            {
            elem = new SVGAnimateColorElementImpl();
            break;
            }                  
        case SVG_ANIMATEMOTION_ELEMENT:
            {
            elem = new SVGAnimateMotionElementImpl();
            break;
            }                  
        case SVG_ANIMATETRANSFORM_ELEMENT:
            {
            elem = new SVGAnimateTransformElementImpl();
            break;
            }                  
        case SVG_CIRCLE_ELEMENT:
            {
            elem = new SVGCircleElementImpl();
            break;
            }                  
        case SVG_CLIPPATH_ELEMENT:
            {
            elem = new SVGClipPathElementImpl();
            break;
            }                  
        case SVG_COLOR_PROFILE_ELEMENT:
            {
            elem = new SVGColorProfileElementImpl();
            break;
            }                  
        case SVG_CURSOR_ELEMENT:
            {
            elem = new SVGCursorElementImpl();
            break;
            }                  
        case SVG_DEFINITION_SRC_ELEMENT:
            {
            elem = new SVGDefinitionSrcElementImpl();
            break;
            }                  
        case SVG_DEFS_ELEMENT:
            {
            elem = new SVGDefsElementImpl();
            break;
            }                  
        case SVG_DESC_ELEMENT:
            {
            elem = new SVGDescElementImpl();
            break;
            }                  
        case SVG_ELLIPSE_ELEMENT:
            {
            elem = new SVGEllipseElementImpl();
            break;
            }                  
        case SVG_FEBLEND_ELEMENT:
            {
            elem = new SVGFEBlendElementImpl();
            break;
            }                  
        case SVG_FECOLORMATRIX_ELEMENT:
            {
            elem = new SVGFEColorMatrixElementImpl();
            break;
            }                  
        case SVG_FECOMPONENTTRANSFER_ELEMENT:
            {
            elem = new SVGFEComponentTransferElementImpl();
            break;
            }                  
        case SVG_FECOMPOSITE_ELEMENT:
            {
            elem = new SVGFECompositeElementImpl();
            break;
            }                  
        case SVG_FECONVOLVEMATRIX_ELEMENT:
            {
            elem = new SVGFEConvolveMatrixElementImpl();
            break;
            }                  
        case SVG_FEDIFFUSELIGHTING_ELEMENT:
            {
            elem = new SVGFEDiffuseLightingElementImpl();
            break;
            }                  
        case SVG_FEDISPLACEMENTMAP_ELEMENT:
            {
            elem = new SVGFEDisplacementMapElementImpl();
            break;
            }                  
        case SVG_FEDISTANTLIGHT_ELEMENT:
            {
            elem = new SVGFEDistantLightElementImpl();
            break;
            }                  
        case SVG_FEFLOOD_ELEMENT:
            {
            elem = new SVGFEFloodElementImpl();
            break;
            }                  
        case SVG_FEFUNCA_ELEMENT:
            {
            elem = new SVGFEFuncAElementImpl();
            break;
            }                  
        case SVG_FEFUNCB_ELEMENT:
            {
            elem = new SVGFEFuncBElementImpl();
            break;
            }                  
        case SVG_FEFUNCG_ELEMENT:
            {
            elem = new SVGFEFuncGElementImpl();
            break;
            }                  
        case SVG_FEFUNCR_ELEMENT:
            {
            elem = new SVGFEFuncRElementImpl();
            break;
            }                  
        case SVG_FEGAUSSIANBLUR_ELEMENT:
            {
            elem = new SVGFEGaussianBlurElementImpl();
            break;
            }                  
        case SVG_FEIMAGE_ELEMENT:
            {
            elem = new SVGFEImageElementImpl();
            break;
            }                  
        case SVG_FEMERGE_ELEMENT:
            {
            elem = new SVGFEMergeElementImpl();
            break;
            }                  
        case SVG_FEMERGENODE_ELEMENT:
            {
            elem = new SVGFEMergeNodeElementImpl();
            break;
            }                  
        case SVG_FEMORPHOLOGY_ELEMENT:
            {
            elem = new SVGFEMorphologyElementImpl();
            break;
            }                  
        case SVG_FEOFFSET_ELEMENT:
            {
            elem = new SVGFEOffsetElementImpl();
            break;
            }                  
        case SVG_FEPOINTLIGHT_ELEMENT:
            {
            elem = new SVGFEPointLightElementImpl();
            break;
            }                  
        case SVG_FESPECULARLIGHTING_ELEMENT:
            {
            elem = new SVGFESpecularLightingElementImpl();
            break;
            }                  
        case SVG_FESPOTLIGHT_ELEMENT:
            {
            elem = new SVGFESpotLightElementImpl();
            break;
            }                  
        case SVG_FETILE_ELEMENT:
            {
            elem = new SVGFETileElementImpl();
            break;
            }                  
        case SVG_FETURBULENCE_ELEMENT:
            {
            elem = new SVGFETurbulenceElementImpl();
            break;
            }                  
        case SVG_FILTER_ELEMENT:
            {
            elem = new SVGFilterElementImpl();
            break;
            }                  
        case SVG_FONT_ELEMENT:
            {
            elem = new SVGAElementImpl();
            break;
            }                  
        case SVG_FONT_FACE_ELEMENT:
            {
            elem = new SVGFontElementImpl();
            break;
            }                  
        case SVG_FONT_FACE_FORMAT_ELEMENT:
            {
            elem = new SVGFontFaceFormatElementImpl();
            break;
            }                  
        case SVG_FONT_FACE_NAME_ELEMENT:
            {
            elem = new SVGFontFaceNameElementImpl();
            break;
            }                  
        case SVG_FONT_FACE_SRC_ELEMENT:
            {
            elem = new SVGFontFaceSrcElementImpl();
            break;
            }                  
        case SVG_FONT_FACE_URI_ELEMENT:
            {
            elem = new SVGFontFaceUriElementImpl();
            break;
            }                  
        case SVG_FOREIGNOBJECT_ELEMENT:
            {
            elem = new SVGForeignObjectElementImpl();
            break;
            }                  
        case SVG_G_ELEMENT:
            {
            elem = new SVGGElementImpl();
            break;
            }                  
        case SVG_GLYPH_ELEMENT:
            {
            elem = new SVGGlyphElementImpl();
            break;
            }                  
        case SVG_GLYPHREF_ELEMENT:
            {
            elem = new SVGGlyphRefElementImpl();
            break;
            }                  
        case SVG_HKERN_ELEMENT:
            {
            elem = new SVGHKernElementImpl();
            break;
            }                  
        case SVG_IMAGE_ELEMENT:
            {
            elem = new SVGImageElementImpl();
            break;
            }                  
        case SVG_LINE_ELEMENT:
            {
            elem = new SVGLineElementImpl();
            break;
            }                  
        case SVG_LINEARGRADIENT_ELEMENT:
            {
            elem = new SVGLinearGradientElementImpl();
            break;
            }                  
        case SVG_MARKER_ELEMENT:
            {
            elem = new SVGMarkerElementImpl();
            break;
            }                  
        case SVG_MASK_ELEMENT:
            {
            elem = new SVGMaskElementImpl();
            break;
            }                  
        case SVG_METADATA_ELEMENT:
            {
            elem = new SVGMetadataElementImpl();
            break;
            }                  
        case SVG_MISSING_GLYPH_ELEMENT:
            {
            elem = new SVGMissingGlyphElementImpl();
            break;
            }                  
        case SVG_MPATH_ELEMENT:
            {
            elem = new SVGMPathElementImpl();
            break;
            }                  
        case SVG_PATH_ELEMENT:
            {
            elem = new SVGPathElementImpl();
            break;
            }                  
        case SVG_PATTERN_ELEMENT:
            {
            elem = new SVGPatternElementImpl();
            break;
            }                  
        case SVG_POLYGON_ELEMENT:
            {
            elem = new SVGPolygonElementImpl();
            break;
            }                  
        case SVG_POLYLINE_ELEMENT:
            {
            elem = new SVGPolylineElementImpl();
            break;
            }                  
        case SVG_RADIALGRADIENT_ELEMENT:
            {
            elem = new SVGRadialGradientElementImpl();
            break;
            }                  
        case SVG_RECT_ELEMENT:
            {
            elem = new SVGRectElementImpl();
            break;
            }                  
        case SVG_SCRIPT_ELEMENT:
            {
            elem = new SVGScriptElementImpl();
            break;
            }                  
        case SVG_SET_ELEMENT:
            {
            elem = new SVGSetElementImpl();
            break;
            }                  
        case SVG_STOP_ELEMENT:
            {
            elem = new SVGStopElementImpl();
            break;
            }                  
        case SVG_STYLE_ELEMENT:
            {
            elem = new SVGStyleElementImpl();
            break;
            }                  
        case SVG_SVG_ELEMENT:
            {
            elem = new SVGSVGElementImpl();
            break;
            }                  
        case SVG_SWITCH_ELEMENT:
            {
            elem = new SVGSwitchElementImpl();
            break;
            }                  
        case SVG_SYMBOL_ELEMENT:
            {
            elem = new SVGSymbolElementImpl();
            break;
            }                  
        case SVG_TEXT_ELEMENT:
            {
            elem = new SVGTextElementImpl();
            break;
            }                  
        case SVG_TEXTPATH_ELEMENT:
            {
            elem = new SVGTextPathElementImpl();
            break;
            }                  
        case SVG_TITLE_ELEMENT:
            {
            elem = new SVGTitleElementImpl();
            break;
            }                  
        case SVG_TREF_ELEMENT:
            {
            elem = new SVGTRefElementImpl();
            break;
            }                  
        case SVG_TSPAN_ELEMENT:
            {
            elem = new SVGTSpanElementImpl();
            break;
            }                  
        case SVG_USE_ELEMENT:
            {
            elem = new SVGUseElementImpl();
            break;
            }                  
        case SVG_VIEW_ELEMENT:
            {
            elem = new SVGViewElementImpl();
            break;
            }                  
        case SVG_VKERN_ELEMENT:
            {
            elem = new SVGVKernElementImpl();
            break;
            }                  
        default:
            {
            }
        }
    return elem;
}


/**
 *
 */
ElementPtr SVGDocumentImpl::createElementNS(const DOMString& namespaceURI,
                             const DOMString& qualifiedName)
                             throw(DOMException)
{
    ElementPtr elem;
    if (namespaceURI == SVG_NAMESPACE)
        elem = createElement(qualifiedName);
    else
        elem = new SVGElementImpl(this, namespaceURI, qualifiedName);
    return elem;
}



//##################
//# Non-API methods
//##################

void SVGDocumentImpl::init()
{
    title       = "";
    referrer    = "";
    domain      = "";
    rootElement = new SVGSVGElementImpl();
}






/*#########################################################################
## SVGSVGElementImpl
#########################################################################*/


/**
 * Retrieves an attribute value by name.
 */
DOMString SVGSVGElementImpl::getAttribute(const DOMString& name)
{
    DOMString s;
    if (name == "x")
        s = d2s(x.getAnimVal().getValue());
    else if (name == "y")
        s = d2s(y.getAnimVal().getValue());
    else 
	    s = SVGElement::getAttribute(name);
	return s;
}

/**
 * Sets an attribute value
 */
void SVGSVGElementImpl::setAttribute(const DOMString& name,
                  const DOMString& value)
                  throw(DOMException)
{
    if (name == "x")
        x.getAnimVal().setValue(s2d(value));
    else if (name == "y")
        y.getAnimVal().setValue(s2d(value));
    SVGElement::setAttribute(name, value);
}


/**
 *
 */
unsigned long SVGSVGElementImpl::suspendRedraw(unsigned long /*max_wait_milliseconds*/ )
{
    return 0L;
}

/**
 *
 */
void SVGSVGElementImpl::unsuspendRedraw(unsigned long /*suspend_handle_id*/ )
                                  throw ( DOMException )
{
}


/**
 *
 */
void SVGSVGElementImpl::unsuspendRedrawAll(  )
{
}

/**
 *
 */
void SVGSVGElementImpl::forceRedraw(  )
{
}

/**
 *
 */
void SVGSVGElementImpl::pauseAnimations(  )
{
}

/**
 *
 */
void SVGSVGElementImpl::unpauseAnimations(  )
{
}

/**
 *
 */
bool SVGSVGElementImpl::animationsPaused(  )
{
    return false;
}


/**
 *
 */
NodeList SVGSVGElementImpl::getIntersectionList(const SVGRect &/*rect*/,
                        const SVGElementPtr /*referenceElement*/ )
{
    NodeList list;
    return list;
}

/**
 *
 */
NodeList SVGSVGElementImpl::getEnclosureList(const SVGRect &/*rect*/,
                         const SVGElementPtr /*referenceElement*/ )
{
    NodeList list;
    return list;
}

/**
 *
 */
bool SVGSVGElementImpl::checkIntersection(const SVGElementPtr /*element*/,
                                          const SVGRect &/*rect*/ )
{
    return false;
}

/**
 *
 */
bool SVGSVGElementImpl::checkEnclosure(const SVGElementPtr /*element*/,
                                       const SVGRect &/*rect*/ )
{
    return false;
}

/**
 *
 */
void SVGSVGElementImpl::deselectAll(  )
{
}

/**
 *
 */
ElementPtr SVGSVGElementImpl::getElementById(const DOMString& /*elementId*/ )
{
    return NULL;
}



//##################
//# Non-API methods
//##################





/*#########################################################################
## SVGGElementImpl
#########################################################################*/


//##################
//# Non-API methods
//##################






/*#########################################################################
## SVGDefsElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################






/*#########################################################################
## SVGDescElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################






/*#########################################################################
## SVGTitleElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################




/*#########################################################################
## SVGSymbolElementImpl
#########################################################################*/




//##################
//# Non-API methods
//##################






/*#########################################################################
## SVGUseElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################









/*#########################################################################
## SVGImageElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################








/*#########################################################################
## SVGSwitchElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################






/*#########################################################################
## GetSVGDocumentImpl
#########################################################################*/

/**
 *
 */
SVGDocumentPtr GetSVGDocumentImpl::getSVGDocument(  )
                    throw ( DOMException )
{
    return NULL;
}

//##################
//# Non-API methods
//##################








/*#########################################################################
## SVGStyleElementImpl
#########################################################################*/





//##################
//# Non-API methods
//##################




/*#########################################################################
## SVGPathElementImpl
#########################################################################*/

/**
 *
 */
SVGAnimatedNumber SVGPathElementImpl::getPathLength()
{
    SVGAnimatedNumber ret;
    return ret;
}

/**
 *
 */
double SVGPathElementImpl::getTotalLength(  )
{
    return 0.0;
}

/**
 *
 */
SVGPoint SVGPathElementImpl::getPointAtLength(double /*distance*/ )
{
    SVGPoint ret;
    return ret;
}

/**
 *
 */
unsigned long SVGPathElementImpl::getPathSegAtLength(double /*distance*/ )
{
    return 0L;
}


//##################
//# Non-API methods
//##################








/*#########################################################################
## SVGRectElementImpl
#########################################################################*/





//##################
//# Non-API methods
//##################








/*#########################################################################
## SVGCircleElementImpl
#########################################################################*/




//##################
//# Non-API methods
//##################








/*#########################################################################
## SVGEllipseElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################








/*#########################################################################
## SVGLineElementImpl
#########################################################################*/


//##################
//# Non-API methods
//##################








/*#########################################################################
## SVGPolylineElementImpl
#########################################################################*/





//##################
//# Non-API methods
//##################






/*#########################################################################
## SVGPolygonElementImpl
#########################################################################*/






//##################
//# Non-API methods
//##################







/*#########################################################################
## SVGTextContentElementImpl
#########################################################################*/

/**
 *
 */
SVGAnimatedLength SVGTextContentElementImpl::getTextLength()
{
    SVGAnimatedLength ret;
    return ret;
}


/**
 *
 */
SVGAnimatedEnumeration SVGTextContentElementImpl::getLengthAdjust()
{
    SVGAnimatedEnumeration ret;
    return ret;
}


/**
 *
 */
long SVGTextContentElementImpl::getNumberOfChars(  )
{
    return 0L;
}

/**
 *
 */
double SVGTextContentElementImpl::getComputedTextLength(  )
{
    return 0.0;
}

/**
 *
 */
double SVGTextContentElementImpl::getSubStringLength(unsigned long /*charnum*/, unsigned long /*nchars*/ )
                                     throw ( DOMException )
{
    return 0.0;
}

/**
 *
 */
SVGPoint SVGTextContentElementImpl::getStartPositionOfChar(unsigned long /*charnum*/ )
                                              throw ( DOMException )
{
    SVGPoint ret;
    return ret;
}

/**
 *
 */
SVGPoint SVGTextContentElementImpl::getEndPositionOfChar(unsigned long /*charnum*/ )
                                           throw ( DOMException )
{
    SVGPoint ret;
    return ret;
}

/**
 *
 */
SVGRect SVGTextContentElementImpl::getExtentOfChar(unsigned long /*charnum*/ )
                                      throw ( DOMException )
{
    SVGRect ret;
    return ret;
}

/**
 *
 */
double SVGTextContentElementImpl::getRotationOfChar(unsigned long /*charnum*/ )
                                     throw ( DOMException )
{
    return 0.0;
}

/**
 *
 */
long SVGTextContentElementImpl::getCharNumAtPosition(const SVGPoint &/*point*/ )
{
    return 0L;
}

/**
 *
 */
void SVGTextContentElementImpl::selectSubString(unsigned long /*charnum*/,
                                                unsigned long /*nchars*/ )
                                                throw ( DOMException )
{
}



//##################
//# Non-API methods
//##################









/*#########################################################################
## SVGTextPositioningElementImpl
#########################################################################*/




//##################
//# Non-API methods
//##################








/*#########################################################################
## SVGTextElementImpl
#########################################################################*/





//##################
//# Non-API methods
//##################







/*#########################################################################
## SVGTSpanElementImpl
#########################################################################*/





//##################
//# Non-API methods
//##################






/*#########################################################################
## SVGTRefElementImpl
#########################################################################*/





//##################
//# Non-API methods
//##################







/*#########################################################################
## SVGTextPathElementImpl
#########################################################################*/




//##################
//# Non-API methods
//##################









/*#########################################################################
## SVGAltGlyphElementImpl
#########################################################################*/






//##################
//# Non-API methods
//##################







/*#########################################################################
## SVGAltGlyphDefElementImpl
#########################################################################*/





//##################
//# Non-API methods
//##################







/*#########################################################################
## SVGAltGlyphItemElementImpl
#########################################################################*/





//##################
//# Non-API methods
//##################







/*#########################################################################
## SVGGlyphRefElementImpl
#########################################################################*/




//##################
//# Non-API methods
//##################








/*#########################################################################
## SVGMarkerElementImpl
#########################################################################*/




//##################
//# Non-API methods
//##################








/*#########################################################################
## SVGColorProfileElementImpl
#########################################################################*/




//##################
//# Non-API methods
//##################









/*#########################################################################
## SVGGradientElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################







/*#########################################################################
## SVGLinearGradientElementImpl
#########################################################################*/




//##################
//# Non-API methods
//##################








/*#########################################################################
## SVGRadialGradientElementImpl
#########################################################################*/





//##################
//# Non-API methods
//##################








/*#########################################################################
## SVGStopElementImpl
#########################################################################*/





//##################
//# Non-API methods
//##################









/*#########################################################################
## SVGPatternElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################








/*#########################################################################
## SVGClipPathElementImpl
#########################################################################*/






//##################
//# Non-API methods
//##################









/*#########################################################################
## SVGMaskElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################









/*#########################################################################
## SVGFilterElementImpl
#########################################################################*/





//##################
//# Non-API methods
//##################









/*#########################################################################
## SVGFEBlendElementImpl
#########################################################################*/




//##################
//# Non-API methods
//##################








/*#########################################################################
## SVGFEColorMatrixElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################









/*#########################################################################
## SVGFEComponentTransferElementImpl
#########################################################################*/




//##################
//# Non-API methods
//##################










/*#########################################################################
## SVGComponentTransferFunctionElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################









/*#########################################################################
## SVGFEFuncRElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################







/*#########################################################################
## SVGFEFuncGElementImpl
#########################################################################*/





//##################
//# Non-API methods
//##################







/*#########################################################################
## SVGFEFuncBElementImpl
#########################################################################*/





//##################
//# Non-API methods
//##################







/*#########################################################################
## SVGFEFuncAElementImpl
#########################################################################*/





//##################
//# Non-API methods
//##################







/*#########################################################################
## SVGFECompositeElementImpl
#########################################################################*/




//##################
//# Non-API methods
//##################








/*#########################################################################
## SVGFEConvolveMatrixElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################








/*#########################################################################
## SVGFEDiffuseLightingElementImpl
#########################################################################*/




//##################
//# Non-API methods
//##################





/*#########################################################################
## SVGFEDistantLightElementImpl
#########################################################################*/





//##################
//# Non-API methods
//##################









/*#########################################################################
## SVGFEPointLightElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################









/*#########################################################################
## SVGFESpotLightElementImpl
#########################################################################*/






//##################
//# Non-API methods
//##################









/*#########################################################################
## SVGFEDisplacementMapElementImpl
#########################################################################*/




//##################
//# Non-API methods
//##################









/*#########################################################################
## SVGFEFloodElementImpl
#########################################################################*/





//##################
//# Non-API methods
//##################









/*#########################################################################
## SVGFEGaussianBlurElementImpl
#########################################################################*/





//##################
//# Non-API methods
//##################









/*#########################################################################
## SVGFEImageElementImpl
#########################################################################*/




//##################
//# Non-API methods
//##################







/*#########################################################################
## SVGFEMergeElementImpl
#########################################################################*/




//##################
//# Non-API methods
//##################







/*#########################################################################
## SVGFEMergeNodeElementImpl
#########################################################################*/





//##################
//# Non-API methods
//##################








/*#########################################################################
## SVGFEMorphologyElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################








/*#########################################################################
## SVGFEOffsetElementImpl
#########################################################################*/




//##################
//# Non-API methods
//##################






/*#########################################################################
## SVGFESpecularLightingElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################









/*#########################################################################
## SVGFETileElementImpl
#########################################################################*/





//##################
//# Non-API methods
//##################









/*#########################################################################
## SVGFETurbulenceElementImpl
#########################################################################*/






//##################
//# Non-API methods
//##################









/*#########################################################################
## SVGCursorElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################









/*#########################################################################
## SVGAElementImpl
#########################################################################*/





//##################
//# Non-API methods
//##################








/*#########################################################################
## SVGViewElementImpl
#########################################################################*/






//##################
//# Non-API methods
//##################








/*#########################################################################
## SVGScriptElementImpl
#########################################################################*/




//##################
//# Non-API methods
//##################







/*#########################################################################
## SVGAnimationElementImpl
#########################################################################*/





//##################
//# Non-API methods
//##################









/*#########################################################################
## SVGAnimateElementImpl
#########################################################################*/


//##################
//# Non-API methods
//##################







/*#########################################################################
## SVGSetElementImpl
#########################################################################*/




//##################
//# Non-API methods
//##################







/*#########################################################################
## SVGAnimateMotionElementImpl
#########################################################################*/


//##################
//# Non-API methods
//##################







/*#########################################################################
## SVGMPathElementImpl
#########################################################################*/


//##################
//# Non-API methods
//##################







/*#########################################################################
## SVGAnimateColorElementImpl
#########################################################################*/


//##################
//# Non-API methods
//##################







/*#########################################################################
## SVGAnimateTransformElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################







/*#########################################################################
## SVGFontElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################







/*#########################################################################
## SVGGlyphElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################







/*#########################################################################
## SVGMissingGlyphElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################







/*#########################################################################
## SVGHKernElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################







/*#########################################################################
## SVGVKernElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################







/*#########################################################################
## SVGFontFaceElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################







/*#########################################################################
## SVGFontFaceSrcElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################







/*#########################################################################
## SVGFontFaceUriElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################





/*#########################################################################
## SVGFontFaceFormatElementImpl
#########################################################################*/




//##################
//# Non-API methods
//##################






/*#########################################################################
## SVGFontFaceNameElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################







/*#########################################################################
## SVGDefinitionSrcElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################







/*#########################################################################
## SVGMetadataElementImpl
#########################################################################*/


//##################
//# Non-API methods
//##################





/*#########################################################################
## SVGForeignObjectElementImpl
#########################################################################*/



//##################
//# Non-API methods
//##################












}  //namespace svg
}  //namespace dom
}  //namespace w3c
}  //namespace org


/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/

