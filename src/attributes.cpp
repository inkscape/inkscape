/**
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2006 Johan Engelen <johan@shouraizou.nl>
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib.h> // g_assert()
#include "attributes.h"

typedef struct {
    gint code;
    gchar const *name;
} SPStyleProp;

/**
 * Lookup dictionary for attributes/properties.
 */

static SPStyleProp const props[] = {
    {SP_ATTR_INVALID, NULL},
    /* SPObject */
    {SP_ATTR_ID, "id"},
    {SP_ATTR_INKSCAPE_COLLECT, "inkscape:collect"},
    {SP_ATTR_INKSCAPE_LABEL, "inkscape:label"},
    /* SPItem */
    {SP_ATTR_TRANSFORM, "transform"},
    {SP_ATTR_SODIPODI_INSENSITIVE, "sodipodi:insensitive"},
    {SP_ATTR_SODIPODI_NONPRINTABLE, "sodipodi:nonprintable"},
    {SP_ATTR_CONNECTOR_AVOID, "inkscape:connector-avoid"},
    {SP_ATTR_CONNECTION_POINTS, "inkscape:connection-points"},
    {SP_ATTR_STYLE, "style"},
    {SP_ATTR_TRANSFORM_CENTER_X, "inkscape:transform-center-x"},
    {SP_ATTR_TRANSFORM_CENTER_Y, "inkscape:transform-center-y"},
    {SP_ATTR_INKSCAPE_PATH_EFFECT, "inkscape:path-effect"},
    {SP_ATTR_INKSCAPE_HIGHLIGHT_COLOR, "inkscape:highlight-color"},
    /* SPAnchor */
    {SP_ATTR_XLINK_HREF, "xlink:href"},
    {SP_ATTR_XLINK_TYPE, "xlink:type"},
    {SP_ATTR_XLINK_ROLE, "xlink:role"},
    {SP_ATTR_XLINK_ARCROLE, "xlink:arcrole"},
    {SP_ATTR_XLINK_TITLE, "xlink:title"},
    {SP_ATTR_XLINK_SHOW, "xlink:show"},
    {SP_ATTR_XLINK_ACTUATE, "xlink:actuate"},
    {SP_ATTR_TARGET, "target"},
    {SP_ATTR_INKSCAPE_GROUPMODE, "inkscape:groupmode"},
    {SP_ATTR_INKSCAPE_EXPANDED, "inkscape:expanded"},
    /* SPRoot */
    {SP_ATTR_VERSION, "version"},
    {SP_ATTR_WIDTH, "width"},
    {SP_ATTR_HEIGHT, "height"},
    {SP_ATTR_VIEWBOX, "viewBox"},
    {SP_ATTR_PRESERVEASPECTRATIO, "preserveAspectRatio"},
    {SP_ATTR_INKSCAPE_VERSION, "inkscape:version"},
    {SP_ATTR_ONLOAD, "onload"},
    /* SPNamedView */
    {SP_ATTR_VIEWONLY, "viewonly"},
    {SP_ATTR_SHOWGUIDES, "showguides"},
    {SP_ATTR_SHOWGRIDS, "showgrid"},
    {SP_ATTR_GRIDTOLERANCE, "gridtolerance"},
    {SP_ATTR_GUIDETOLERANCE, "guidetolerance"},
    {SP_ATTR_OBJECTTOLERANCE, "objecttolerance"},
    {SP_ATTR_GUIDECOLOR, "guidecolor"},
    {SP_ATTR_GUIDEOPACITY, "guideopacity"},
    {SP_ATTR_GUIDEHICOLOR, "guidehicolor"},
    {SP_ATTR_GUIDEHIOPACITY, "guidehiopacity"},
    {SP_ATTR_SHOWBORDER, "showborder"},
    {SP_ATTR_SHOWPAGESHADOW, "inkscape:showpageshadow"},
    {SP_ATTR_BORDERLAYER, "borderlayer"},
    {SP_ATTR_BORDERCOLOR, "bordercolor"},
    {SP_ATTR_BORDEROPACITY, "borderopacity"},
    {SP_ATTR_PAGECOLOR, "pagecolor"},
    {SP_ATTR_FIT_MARGIN_TOP, "fit-margin-top"},
    {SP_ATTR_FIT_MARGIN_LEFT, "fit-margin-left"},
    {SP_ATTR_FIT_MARGIN_RIGHT, "fit-margin-right"},
    {SP_ATTR_FIT_MARGIN_BOTTOM, "fit-margin-bottom"},
    {SP_ATTR_INKSCAPE_PAGEOPACITY, "inkscape:pageopacity"},
    {SP_ATTR_INKSCAPE_PAGESHADOW, "inkscape:pageshadow"},
    {SP_ATTR_INKSCAPE_ZOOM, "inkscape:zoom"},
    {SP_ATTR_INKSCAPE_CX, "inkscape:cx"},
    {SP_ATTR_INKSCAPE_CY, "inkscape:cy"},
    {SP_ATTR_INKSCAPE_WINDOW_WIDTH, "inkscape:window-width"},
    {SP_ATTR_INKSCAPE_WINDOW_HEIGHT, "inkscape:window-height"},
    {SP_ATTR_INKSCAPE_WINDOW_X, "inkscape:window-x"},
    {SP_ATTR_INKSCAPE_WINDOW_Y, "inkscape:window-y"},
    {SP_ATTR_INKSCAPE_WINDOW_MAXIMIZED, "inkscape:window-maximized"},
    {SP_ATTR_INKSCAPE_SNAP_GLOBAL, "inkscape:snap-global"},
    {SP_ATTR_INKSCAPE_SNAP_PERP, "inkscape:snap-perpendicular"},
    {SP_ATTR_INKSCAPE_SNAP_TANG, "inkscape:snap-tangential"},
    {SP_ATTR_INKSCAPE_SNAP_BBOX, "inkscape:snap-bbox"},
    {SP_ATTR_INKSCAPE_SNAP_NODE, "inkscape:snap-nodes"},
    {SP_ATTR_INKSCAPE_SNAP_OTHERS, "inkscape:snap-others"},
    {SP_ATTR_INKSCAPE_SNAP_ROTATION_CENTER, "inkscape:snap-center"},
    {SP_ATTR_INKSCAPE_SNAP_GRID, "inkscape:snap-grids"},
    {SP_ATTR_INKSCAPE_SNAP_GUIDE, "inkscape:snap-to-guides"},
    {SP_ATTR_INKSCAPE_SNAP_NODE_SMOOTH, "inkscape:snap-smooth-nodes"},
    {SP_ATTR_INKSCAPE_SNAP_LINE_MIDPOINT, "inkscape:snap-midpoints"},
    {SP_ATTR_INKSCAPE_SNAP_OBJECT_MIDPOINT, "inkscape:snap-object-midpoints"},
    {SP_ATTR_INKSCAPE_SNAP_TEXT_BASELINE, "inkscape:snap-text-baseline"},
    {SP_ATTR_INKSCAPE_SNAP_BBOX_EDGE_MIDPOINT, "inkscape:snap-bbox-edge-midpoints"},
    {SP_ATTR_INKSCAPE_SNAP_BBOX_MIDPOINT, "inkscape:snap-bbox-midpoints"},
    {SP_ATTR_INKSCAPE_SNAP_PATH_INTERSECTION, "inkscape:snap-intersection-paths"},
    {SP_ATTR_INKSCAPE_SNAP_PATH, "inkscape:object-paths"},
    {SP_ATTR_INKSCAPE_SNAP_PATH_CLIP, "inkscape:snap-path-clip"},
    {SP_ATTR_INKSCAPE_SNAP_PATH_MASK, "inkscape:snap-path-mask"},
    {SP_ATTR_INKSCAPE_SNAP_NODE_CUSP, "inkscape:object-nodes"},
    {SP_ATTR_INKSCAPE_SNAP_BBOX_EDGE, "inkscape:bbox-paths"},
    {SP_ATTR_INKSCAPE_SNAP_BBOX_CORNER, "inkscape:bbox-nodes"},
    {SP_ATTR_INKSCAPE_SNAP_PAGE_BORDER, "inkscape:snap-page"},
    {SP_ATTR_INKSCAPE_CURRENT_LAYER, "inkscape:current-layer"},
    {SP_ATTR_INKSCAPE_DOCUMENT_UNITS, "inkscape:document-units"},  // This setting sets the Display units, *not* the units used in SVG
    {SP_ATTR_UNITS, "units"},
    {SP_ATTR_INKSCAPE_CONNECTOR_SPACING, "inkscape:connector-spacing"},
    /* SPColorProfile */
    {SP_ATTR_LOCAL, "local"},
    {SP_ATTR_NAME, "name"},
    {SP_ATTR_RENDERING_INTENT, "rendering-intent"},
    /* SPGuide */
    {SP_ATTR_ORIENTATION, "orientation"},
    {SP_ATTR_POSITION, "position"},
    {SP_ATTR_INKSCAPE_COLOR, "inkscape:color"},
    /* SPImage */
    {SP_ATTR_X, "x"},
    {SP_ATTR_Y, "y"},
    /* SPPath */
    {SP_ATTR_D, "d"},
    {SP_ATTR_INKSCAPE_ORIGINAL_D, "inkscape:original-d"},
    /* (Note: XML representation of connectors may change in future.) */
    {SP_ATTR_CONNECTOR_TYPE, "inkscape:connector-type"},
    {SP_ATTR_CONNECTOR_CURVATURE, "inkscape:connector-curvature"},
    {SP_ATTR_CONNECTION_START, "inkscape:connection-start"},
    {SP_ATTR_CONNECTION_END, "inkscape:connection-end"},
    {SP_ATTR_CONNECTION_START_POINT, "inkscape:connection-start-point"},
    {SP_ATTR_CONNECTION_END_POINT, "inkscape:connection-end-point"},
    /* SPRect */
    {SP_ATTR_RX, "rx"},
    {SP_ATTR_RY, "ry"},
    /* Box3D */
    {SP_ATTR_INKSCAPE_BOX3D_PERSPECTIVE_ID, "inkscape:perspectiveID"},
    {SP_ATTR_INKSCAPE_BOX3D_CORNER0, "inkscape:corner0"},
    {SP_ATTR_INKSCAPE_BOX3D_CORNER7, "inkscape:corner7"},
    /* Box3DSide */
    {SP_ATTR_INKSCAPE_BOX3D_SIDE_TYPE, "inkscape:box3dsidetype"}, // XYfront, etc.
    /* Persp3D */
    {SP_ATTR_INKSCAPE_PERSP3D, "inkscape:persp3d"},
    {SP_ATTR_INKSCAPE_PERSP3D_VP_X, "inkscape:vp_x"},
    {SP_ATTR_INKSCAPE_PERSP3D_VP_Y, "inkscape:vp_y"},
    {SP_ATTR_INKSCAPE_PERSP3D_VP_Z, "inkscape:vp_z"},
    {SP_ATTR_INKSCAPE_PERSP3D_ORIGIN, "inkscape:persp3d-origin"},
    /* SPEllipse */
    {SP_ATTR_R, "r"},
    {SP_ATTR_CX, "cx"},
    {SP_ATTR_CY, "cy"},
    {SP_ATTR_SODIPODI_CX, "sodipodi:cx"},
    {SP_ATTR_SODIPODI_CY, "sodipodi:cy"},
    {SP_ATTR_SODIPODI_RX, "sodipodi:rx"},
    {SP_ATTR_SODIPODI_RY, "sodipodi:ry"},
    {SP_ATTR_SODIPODI_START, "sodipodi:start"},
    {SP_ATTR_SODIPODI_END, "sodipodi:end"},
    {SP_ATTR_SODIPODI_OPEN, "sodipodi:open"},
    /* SPStar */
    {SP_ATTR_SODIPODI_SIDES, "sodipodi:sides"},
    {SP_ATTR_SODIPODI_R1, "sodipodi:r1"},
    {SP_ATTR_SODIPODI_R2, "sodipodi:r2"},
    {SP_ATTR_SODIPODI_ARG1, "sodipodi:arg1"},
    {SP_ATTR_SODIPODI_ARG2, "sodipodi:arg2"},
    {SP_ATTR_INKSCAPE_FLATSIDED, "inkscape:flatsided"},
    {SP_ATTR_INKSCAPE_ROUNDED, "inkscape:rounded"},
    {SP_ATTR_INKSCAPE_RANDOMIZED, "inkscape:randomized"},
    /* SPSpiral */
    {SP_ATTR_SODIPODI_EXPANSION, "sodipodi:expansion"},
    {SP_ATTR_SODIPODI_REVOLUTION, "sodipodi:revolution"},
    {SP_ATTR_SODIPODI_RADIUS, "sodipodi:radius"},
    {SP_ATTR_SODIPODI_ARGUMENT, "sodipodi:argument"},
    {SP_ATTR_SODIPODI_T0, "sodipodi:t0"},
    /* SPOffset */
    {SP_ATTR_SODIPODI_ORIGINAL, "sodipodi:original"},
    {SP_ATTR_INKSCAPE_ORIGINAL, "inkscape:original"},
    {SP_ATTR_INKSCAPE_HREF, "inkscape:href"},
    {SP_ATTR_INKSCAPE_RADIUS, "inkscape:radius"},
    /* SPLine */
    {SP_ATTR_X1, "x1"},
    {SP_ATTR_Y1, "y1"},
    {SP_ATTR_X2, "x2"},
    {SP_ATTR_Y2, "y2"},
    /* SPPolyline */
    {SP_ATTR_POINTS, "points"},
    /* SPTSpan */
    {SP_ATTR_DX, "dx"},
    {SP_ATTR_DY, "dy"},
    {SP_ATTR_ROTATE, "rotate"},
    {SP_ATTR_TEXTLENGTH, "textLength"},
    {SP_ATTR_LENGTHADJUST, "lengthAdjust"},
    {SP_ATTR_SODIPODI_ROLE, "sodipodi:role"},
    /* SPText */
    {SP_ATTR_SODIPODI_LINESPACING, "sodipodi:linespacing"},
    /* SPTextPath */
    {SP_ATTR_STARTOFFSET, "startOffset"},
    /* SPStop */
    {SP_ATTR_OFFSET, "offset"},
    /* SPFilter */
    {SP_ATTR_FILTERUNITS, "filterUnits"},
    {SP_ATTR_PRIMITIVEUNITS, "primitiveUnits"},
    {SP_ATTR_FILTERRES, "filterRes"},
    /* Filter primitives common */
    {SP_ATTR_IN, "in"},
    {SP_ATTR_RESULT, "result"},
    /*feBlend*/
    {SP_ATTR_MODE, "mode"},
    {SP_ATTR_IN2, "in2"},
    /*feColorMatrix*/
    {SP_ATTR_TYPE, "type"},
    {SP_ATTR_VALUES, "values"},
    /*feComponentTransfer*/
    //{SP_ATTR_TYPE, "type"},
    {SP_ATTR_TABLEVALUES, "tableValues"},
    {SP_ATTR_SLOPE, "slope"},
    {SP_ATTR_INTERCEPT, "intercept"},
    {SP_ATTR_AMPLITUDE, "amplitude"},
    {SP_ATTR_EXPONENT, "exponent"},
    //{SP_ATTR_OFFSET, "offset"},
    /*feComposite*/
    {SP_ATTR_OPERATOR, "operator"},
    {SP_ATTR_K1, "k1"},
    {SP_ATTR_K2, "k2"},
    {SP_ATTR_K3, "k3"},
    {SP_ATTR_K4, "k4"},
    //{SP_ATTR_IN2, "in2"},
    /*feConvolveMatrix*/
    {SP_ATTR_ORDER, "order"},
    {SP_ATTR_KERNELMATRIX, "kernelMatrix"},
    {SP_ATTR_DIVISOR, "divisor"},
    {SP_ATTR_BIAS, "bias"},
    {SP_ATTR_TARGETX, "targetX"},
    {SP_ATTR_TARGETY, "targetY"},
    {SP_ATTR_EDGEMODE, "edgeMode"},
    {SP_ATTR_KERNELUNITLENGTH, "kernelUnitLength"},
    {SP_ATTR_PRESERVEALPHA, "preserveAlpha"},
    /*feDiffuseLighting*/
    {SP_ATTR_SURFACESCALE, "surfaceScale"},
    {SP_ATTR_DIFFUSECONSTANT, "diffuseConstant"},
    //{SP_ATTR_KERNELUNITLENGTH, "kernelUnitLength"},
    /*feDisplacementMap*/
    {SP_ATTR_SCALE, "scale"},
    {SP_ATTR_XCHANNELSELECTOR, "xChannelSelector"},
    {SP_ATTR_YCHANNELSELECTOR, "yChannelSelector"},
    //{SP_ATTR_IN2, "in2"},
    /*feDistantLight*/
    {SP_ATTR_AZIMUTH, "azimuth"},
    {SP_ATTR_ELEVATION, "elevation"},
    /*fePointLight*/
    {SP_ATTR_Z, "z"},
    /*feSpotLight*/
    {SP_ATTR_POINTSATX, "pointsAtX"},
    {SP_ATTR_POINTSATY, "pointsAtY"},
    {SP_ATTR_POINTSATZ, "pointsAtZ"},
    {SP_ATTR_LIMITINGCONEANGLE, "limitingConeAngle"},
    /* SPGaussianBlur */
    {SP_ATTR_STDDEVIATION, "stdDeviation"},
    /*feImage*/
    /*feMerge*/
    /*feMorphology*/
    //{SP_ATTR_OPERATOR, "operator"},
    {SP_ATTR_RADIUS, "radius"},
    /*feOffset*/
    //{SP_ATTR_DX, "dx"},
    //{SP_ATTR_DY, "dy"},
    /*feSpecularLighting*/
    {SP_ATTR_SPECULARCONSTANT, "specularConstant"},
    {SP_ATTR_SPECULAREXPONENT, "specularExponent"},
    /*feTile*/
    /*feTurbulence*/
    {SP_ATTR_BASEFREQUENCY, "baseFrequency"},
    {SP_ATTR_NUMOCTAVES, "numOctaves"},
    {SP_ATTR_SEED, "seed"},
    {SP_ATTR_STITCHTILES, "stitchTiles"},
    //{SP_ATTR_TYPE, "type"},
    /* SPGradient */
    {SP_ATTR_GRADIENTUNITS, "gradientUnits"},
    {SP_ATTR_GRADIENTTRANSFORM, "gradientTransform"},
    {SP_ATTR_SPREADMETHOD, "spreadMethod"},
    {SP_ATTR_OSB_SWATCH, "osb:paint"},
    /* SPRadialGradient */
    {SP_ATTR_FX, "fx"},
    {SP_ATTR_FY, "fy"},
    /* SPMeshPatch */
    {SP_ATTR_TENSOR, "tensor"},
    //{SP_ATTR_TYPE, "type"},
    /* SPPattern */
    {SP_ATTR_PATTERNUNITS, "patternUnits"},
    {SP_ATTR_PATTERNCONTENTUNITS, "patternContentUnits"},
    {SP_ATTR_PATTERNTRANSFORM, "patternTransform"},
    /* SPHatch */
    {SP_ATTR_HATCHUNITS, "hatchUnits"},
    {SP_ATTR_HATCHCONTENTUNITS, "hatchContentUnits"},
    {SP_ATTR_HATCHTRANSFORM, "hatchTransform"},
    {SP_ATTR_PITCH, "pitch"},
    /* SPClipPath */
    {SP_ATTR_CLIPPATHUNITS, "clipPathUnits"},
    /* SPMask */
    {SP_ATTR_MASKUNITS, "maskUnits"},
    {SP_ATTR_MASKCONTENTUNITS, "maskContentUnits"},
    /* SPMarker */
    {SP_ATTR_MARKERUNITS, "markerUnits"},
    {SP_ATTR_REFX, "refX"},
    {SP_ATTR_REFY, "refY"},
    {SP_ATTR_MARKERWIDTH, "markerWidth"},
    {SP_ATTR_MARKERHEIGHT, "markerHeight"},
    {SP_ATTR_ORIENT, "orient"},
    /* SPStyleElem */
    //{SP_ATTR_TYPE, "type"},
    /* Animations */
    {SP_ATTR_ATTRIBUTENAME, "attributeName"},
    {SP_ATTR_ATTRIBUTETYPE, "attributeType"},
    {SP_ATTR_BEGIN, "begin"},
    {SP_ATTR_DUR, "dur"},
    {SP_ATTR_END, "end"},
    {SP_ATTR_MIN, "min"},
    {SP_ATTR_MAX, "max"},
    {SP_ATTR_RESTART, "restart"},
    {SP_ATTR_REPEATCOUNT, "repeatCount"},
    {SP_ATTR_REPEATDUR, "repeatDur"},
    /* Interpolating animations */
    {SP_ATTR_CALCMODE, "calcMode"},
    //{SP_ATTR_VALUES, "values"},
    {SP_ATTR_KEYTIMES, "keyTimes"},
    {SP_ATTR_KEYSPLINES, "keySplines"},
    {SP_ATTR_FROM, "from"},
    {SP_ATTR_TO, "to"},
    {SP_ATTR_BY, "by"},
    {SP_ATTR_ADDITIVE, "additive"},
    {SP_ATTR_ACCUMULATE, "accumulate"},

    /* SVGFonts */
    /*<font>*/
    {SP_ATTR_HORIZ_ORIGIN_X, "horiz-origin-x"},
    {SP_ATTR_HORIZ_ORIGIN_Y, "horiz-origin-y"},
    {SP_ATTR_HORIZ_ADV_X, "horiz-adv-x"},
    {SP_ATTR_VERT_ORIGIN_X, "vert-origin-x"},
    {SP_ATTR_VERT_ORIGIN_Y, "vert-origin-y"},
    {SP_ATTR_VERT_ADV_Y, "vert-adv-y"},

    /*<glyph>*/
    {SP_ATTR_UNICODE, "unicode"},
    {SP_ATTR_GLYPH_NAME, "glyph-name"},
    //{SP_ATTR_ORIENTATION, "orientation"},
    {SP_ATTR_ARABIC_FORM, "arabic-form"},
    {SP_ATTR_LANG, "lang"},

    /*<hkern> and <vkern>*/
    {SP_ATTR_U1, "u1"},
    {SP_ATTR_G1, "g1"},
    {SP_ATTR_U2, "u2"},
    {SP_ATTR_G2, "g2"},
    {SP_ATTR_K, "k"},

    /*<font-face>*/
    //{SP_ATTR_FONT_FAMILY, "font-family"}, these are already set for CSS2 (SP_PROP_FONT_FAMILY, SP_PROP_FONT_STYLE, SP_PROP_FONT_VARIANT etc...)
    //{SP_ATTR_FONT_STYLE, "font-style"},
    //{SP_ATTR_FONT_VARIANT, "font-variant"},
    //{SP_ATTR_FONT_WEIGHT, "font-weight"},
    //{SP_ATTR_FONT_STRETCH, "font-stretch"},
    //{SP_ATTR_FONT_SIZE, "font-size"},
    {SP_ATTR_UNICODE_RANGE, "unicode-range"},
    {SP_ATTR_UNITS_PER_EM, "units-per-em"},
    {SP_ATTR_PANOSE_1, "panose-1"},
    {SP_ATTR_STEMV, "stemv"},
    {SP_ATTR_STEMH, "stemh"},
    //{SP_ATTR_SLOPE, "slope"},
    {SP_ATTR_CAP_HEIGHT, "cap-height"},
    {SP_ATTR_X_HEIGHT, "x-height"},
    {SP_ATTR_ACCENT_HEIGHT, "accent-height"},
    {SP_ATTR_ASCENT, "ascent"},
    {SP_ATTR_DESCENT, "descent"},
    {SP_ATTR_WIDTHS, "widths"},
    {SP_ATTR_BBOX, "bbox"},
    {SP_ATTR_IDEOGRAPHIC, "ideographic"},
    {SP_ATTR_ALPHABETIC, "alphabetic"},
    {SP_ATTR_MATHEMATICAL, "mathematical"},
    {SP_ATTR_HANGING, "hanging"},
    {SP_ATTR_V_IDEOGRAPHIC, "v-ideographic"},
    {SP_ATTR_V_ALPHABETIC, "v-alphabetic"},
    {SP_ATTR_V_MATHEMATICAL, "v-mathematical"},
    {SP_ATTR_V_HANGING, "v-hanging"},
    {SP_ATTR_UNDERLINE_POSITION, "underline-position"},
    {SP_ATTR_UNDERLINE_THICKNESS, "underline-thickness"},
    {SP_ATTR_STRIKETHROUGH_POSITION, "strikethrough-position"},
    {SP_ATTR_STRIKETHROUGH_THICKNESS, "strikethrough-thickness"},
    {SP_ATTR_OVERLINE_POSITION, "overline-position"},
    {SP_ATTR_OVERLINE_THICKNESS, "overline-thickness"},

    /* XML */
    {SP_ATTR_XML_SPACE, "xml:space"},

    /* typeset */
    {SP_ATTR_TEXT_NOMARKUP, "inkscape:srcNoMarkup"},
    {SP_ATTR_TEXT_PANGOMARKUP, "inkscape:srcPango" },
    {SP_ATTR_TEXT_INSHAPE, "inkscape:dstShape"},
    {SP_ATTR_TEXT_ONPATH, "inkscape:dstPath"},
    {SP_ATTR_TEXT_INBOX,"inkscape:dstBox"},
    {SP_ATTR_TEXT_INCOLUMN,"inkscape:dstColumn"},
    {SP_ATTR_TEXT_EXCLUDE,"inkscape:excludeShape"},
    {SP_ATTR_LAYOUT_OPTIONS,"inkscape:layoutOptions"},

    /* CSS2 */
    {SP_PROP_INKSCAPE_FONT_SPEC, "-inkscape-font-specification"},
    /* Font */
    {SP_PROP_FONT, "font"},
    {SP_PROP_FONT_FAMILY, "font-family"},
    {SP_PROP_FONT_SIZE, "font-size"},
    {SP_PROP_FONT_SIZE_ADJUST, "font-size-adjust"},
    {SP_PROP_FONT_STRETCH, "font-stretch"},
    {SP_PROP_FONT_STYLE, "font-style"},
    {SP_PROP_FONT_VARIANT, "font-variant"},
    {SP_PROP_FONT_WEIGHT, "font-weight"},

    /* Font Variants CSS 3 */
    {SP_PROP_FONT_VARIANT_LIGATURES,  "font-variant-ligatures"},
    {SP_PROP_FONT_VARIANT_POSITION,   "font-variant-position"},
    {SP_PROP_FONT_VARIANT_CAPS,       "font-variant-caps"},
    {SP_PROP_FONT_VARIANT_NUMERIC,    "font-variant-numeric"},
    {SP_PROP_FONT_VARIANT_ALTERNATES, "font-variant-alternates"},
    {SP_PROP_FONT_VARIANT_EAST_ASIAN, "font-variant-east-asian"},
    {SP_PROP_FONT_FEATURE_SETTINGS,   "font-feature-settings"},

    /* Text */
    {SP_PROP_TEXT_INDENT, "text-indent"},
    {SP_PROP_TEXT_ALIGN, "text-align"},
    {SP_PROP_LINE_HEIGHT, "line-height"},
    {SP_PROP_LETTER_SPACING, "letter-spacing"},
    {SP_PROP_WORD_SPACING, "word-spacing"},
    {SP_PROP_TEXT_TRANSFORM, "text-transform"},

    /* Text (css3) */
    {SP_PROP_DIRECTION, "direction"},
    {SP_PROP_BLOCK_PROGRESSION, "block-progression"},
    {SP_PROP_WRITING_MODE, "writing-mode"},
    {SP_PROP_UNICODE_BIDI, "unicode-bidi"},
    {SP_PROP_ALIGNMENT_BASELINE, "alignment-baseline"},
    {SP_PROP_BASELINE_SHIFT, "baseline-shift"},
    {SP_PROP_DOMINANT_BASELINE, "dominant-baseline"},
    {SP_PROP_GLYPH_ORIENTATION_HORIZONTAL, "glyph-orientation-horizontal"},
    {SP_PROP_GLYPH_ORIENTATION_VERTICAL, "glyph-orientation-vertical"},
    {SP_PROP_KERNING, "kerning"},
    {SP_PROP_TEXT_ANCHOR, "text-anchor"},
    {SP_PROP_WHITE_SPACE, "white-space"},

    /* SVG 2 Text Wrapping */
    {SP_PROP_SHAPE_INSIDE,  "shape-inside"},
    {SP_PROP_SHAPE_OUTSIDE, "shape-outside"},
    {SP_PROP_SHAPE_PADDING, "shape-padding"},
    {SP_PROP_SHAPE_MARGIN,  "shape-margin"},

    /* Text Decoration */
    {SP_PROP_TEXT_DECORATION,       "text-decoration"},
    {SP_PROP_TEXT_DECORATION_LINE,  "text-decoration-line"},
    {SP_PROP_TEXT_DECORATION_STYLE, "text-decoration-style"},
    {SP_PROP_TEXT_DECORATION_COLOR, "text-decoration-color"},
    {SP_PROP_TEXT_DECORATION_FILL,  "text-decoration-fill"},
    {SP_PROP_TEXT_DECORATION_STROKE,"text-decoration-stroke"},

    /* Misc */
    {SP_PROP_CLIP, "clip"},
    {SP_PROP_COLOR, "color"},
    {SP_PROP_CURSOR, "cursor"},
    {SP_PROP_DISPLAY, "display"},
    {SP_PROP_OVERFLOW, "overflow"},
    {SP_PROP_VISIBILITY, "visibility"},
    {SP_PROP_MIX_BLEND_MODE, "mix-blend-mode"}, // CSS Blending and Compositing
    {SP_PROP_ISOLATION, "isolation"},
    /* SVG */
    /* Clip/Mask */
    {SP_PROP_CLIP_PATH, "clip-path"},
    {SP_PROP_CLIP_RULE, "clip-rule"},
    {SP_PROP_MASK, "mask"},
    {SP_PROP_OPACITY, "opacity"},
    /* Filter */
    {SP_PROP_ENABLE_BACKGROUND, "enable-background"},
    {SP_PROP_FILTER, "filter"},
    {SP_PROP_FLOOD_COLOR, "flood-color"},
    {SP_PROP_FLOOD_OPACITY, "flood-opacity"},
    {SP_PROP_LIGHTING_COLOR, "lighting-color"},
    /* Gradient */
    {SP_PROP_STOP_COLOR, "stop-color"},
    {SP_PROP_STOP_OPACITY, "stop-opacity"},
    {SP_PROP_STOP_PATH, "path"},
    /* Interactivity */
    {SP_PROP_POINTER_EVENTS, "pointer-events"},
    /* Paint */
    {SP_PROP_COLOR_INTERPOLATION, "color-interpolation"},
    {SP_PROP_COLOR_INTERPOLATION_FILTERS, "color-interpolation-filters"},
    {SP_PROP_COLOR_PROFILE, "color-profile"},
    {SP_PROP_COLOR_RENDERING, "color-rendering"},
    {SP_PROP_FILL, "fill"},
    {SP_PROP_FILL_OPACITY, "fill-opacity"},
    {SP_PROP_FILL_RULE, "fill-rule"},
    {SP_PROP_IMAGE_RENDERING, "image-rendering"},
    {SP_PROP_MARKER, "marker"},
    {SP_PROP_MARKER_END, "marker-end"},
    {SP_PROP_MARKER_MID, "marker-mid"},
    {SP_PROP_MARKER_START, "marker-start"},
    {SP_PROP_PAINT_ORDER, "paint-order" },
    {SP_PROP_SHAPE_RENDERING, "shape-rendering"},
    {SP_PROP_SOLID_COLOR, "solid-color"},
    {SP_PROP_SOLID_OPACITY, "solid-opacity"},
    {SP_PROP_STROKE, "stroke"},
    {SP_PROP_STROKE_DASHARRAY, "stroke-dasharray"},
    {SP_PROP_STROKE_DASHOFFSET, "stroke-dashoffset"},
    {SP_PROP_STROKE_LINECAP, "stroke-linecap"},
    {SP_PROP_STROKE_LINEJOIN, "stroke-linejoin"},
    {SP_PROP_STROKE_MITERLIMIT, "stroke-miterlimit"},
    {SP_PROP_STROKE_OPACITY, "stroke-opacity"},
    {SP_PROP_STROKE_WIDTH, "stroke-width"},
    {SP_PROP_TEXT_RENDERING, "text-rendering"},
    /* Conditional */
    {SP_PROP_SYSTEM_LANGUAGE, "systemLanguage"},
    {SP_PROP_REQUIRED_FEATURES, "requiredFeatures"},
    {SP_PROP_REQUIRED_EXTENSIONS, "requiredExtensions"},
    /* LivePathEffect */
    {SP_PROP_PATH_EFFECT, "effect"},
};

#define n_attrs (sizeof(props) / sizeof(props[0]))

/** Returns an SPAttributeEnum; SP_ATTR_INVALID (of value 0) if key isn't recognized. */
unsigned
sp_attribute_lookup(gchar const *key)
{
    static GHashTable *propdict = NULL;

    if (!propdict) {
        unsigned int i;
        propdict = g_hash_table_new(g_str_hash, g_str_equal);
        for (i = 1; i < n_attrs; i++) {
            g_assert(props[i].code == static_cast< gint >(i) );
            // If this g_assert fails, then the sort order of SPAttributeEnum does not match the order in props[]!
            g_hash_table_insert(propdict,
                                const_cast<void *>(static_cast<void const *>(props[i].name)),
                                GINT_TO_POINTER(props[i].code));
        }
    }

    return GPOINTER_TO_UINT(g_hash_table_lookup(propdict, key));
}

unsigned char const *
sp_attribute_name(unsigned int id)
{
    if (id >= n_attrs) {
        return NULL;
    }

    return (unsigned char*)props[id].name;
}


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :
