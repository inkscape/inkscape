/*
 * Unit tests for attributes.
 *
 * Author:
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2015 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "attributes.h"

namespace {

static const unsigned int FIRST_VALID_ID = 1;

class AttributeInfo
{
public:
    AttributeInfo(std::string const &attr, bool supported) :
        attr(attr),
        supported(supported)
    {
    }

    std::string attr;
    bool supported;
};

typedef std::vector<AttributeInfo>::iterator AttrItr;

std::vector<AttributeInfo> getKnownAttrs()
{
/* Extracted mechanically from http://www.w3.org/TR/SVG11/attindex.html:

   tidy -wrap 999 -asxml < attindex.html 2>/dev/null |
   tr -d \\n |
   sed 's,<tr>,@,g' |
   tr @ \\n |
   sed 's,</td>.*,,;s,^<td>,,;1,/^%/d;/^%/d;s,^,    {",;s/$/", false},/' |
   uniq

   attindex.html lacks attributeName, begin, additive, font, marker;
   I've added these manually.

   SVG 2: white-space, shape-inside, shape-outside, shape-padding, shape-margin
*/
    AttributeInfo all_attrs[] = {
        AttributeInfo("attributeName", true),
        AttributeInfo("begin", true),
        AttributeInfo("additive", true),
        AttributeInfo("font", true),
        AttributeInfo("-inkscape-font-specification", true), // TODO look into this attribute's name
        AttributeInfo("marker", true),
        AttributeInfo("line-height", true),

        AttributeInfo("accent-height", true),
        AttributeInfo("accumulate", true),
        AttributeInfo("alignment-baseline", true),
        AttributeInfo("alphabetic", true),
        AttributeInfo("amplitude", true),
        AttributeInfo("animate", false),
        AttributeInfo("arabic-form", true),
        AttributeInfo("ascent", true),
        AttributeInfo("attributeType", true),
        AttributeInfo("azimuth", true),
        AttributeInfo("baseFrequency", true),
        AttributeInfo("baseline-shift", true),
        AttributeInfo("baseProfile", false),
        AttributeInfo("bbox", true),
        AttributeInfo("bias", true),
        AttributeInfo("block-progression", true),
        AttributeInfo("by", true),
        AttributeInfo("calcMode", true),
        AttributeInfo("cap-height", true),
        AttributeInfo("class", false),
        AttributeInfo("clip", true),
        AttributeInfo("clip-path", true),
        AttributeInfo("clip-rule", true),
        AttributeInfo("clipPathUnits", true),
        AttributeInfo("color", true),
        AttributeInfo("color-interpolation", true),
        AttributeInfo("color-interpolation-filters", true),
        AttributeInfo("color-profile", true),
        AttributeInfo("color-rendering", true),
        AttributeInfo("contentScriptType", false),
        AttributeInfo("contentStyleType", false),
        AttributeInfo("cursor", true),
        AttributeInfo("cx", true),
        AttributeInfo("cy", true),
        AttributeInfo("d", true),
        AttributeInfo("descent", true),
        AttributeInfo("diffuseConstant", true),
        AttributeInfo("direction", true),
        AttributeInfo("display", true),
        AttributeInfo("divisor", true),
        AttributeInfo("dominant-baseline", true),
        AttributeInfo("dur", true),
        AttributeInfo("dx", true),
        AttributeInfo("dy", true),
        AttributeInfo("edgeMode", true),
        AttributeInfo("elevation", true),
        AttributeInfo("enable-background", true),
        AttributeInfo("end", true),
        AttributeInfo("exponent", true),
        AttributeInfo("externalResourcesRequired", false),
        AttributeInfo("feBlend", false),
        AttributeInfo("feColorMatrix", false),
        AttributeInfo("feComponentTransfer", false),
        AttributeInfo("feComposite", false),
        AttributeInfo("feConvolveMatrix", false),
        AttributeInfo("feDiffuseLighting", false),
        AttributeInfo("feDisplacementMap", false),
        AttributeInfo("feFlood", false),
        AttributeInfo("feGaussianBlur", false),
        AttributeInfo("feImage", false),
        AttributeInfo("feMerge", false),
        AttributeInfo("feMorphology", false),
        AttributeInfo("feOffset", false),
        AttributeInfo("feSpecularLighting", false),
        AttributeInfo("feTile", false),
        AttributeInfo("fill", true),
        AttributeInfo("fill-opacity", true),
        AttributeInfo("fill-rule", true),
        AttributeInfo("filter", true),
        AttributeInfo("filterRes", true),
        AttributeInfo("filterUnits", true),
        AttributeInfo("flood-color", true),
        AttributeInfo("flood-opacity", true),
        AttributeInfo("font-family", true),
        AttributeInfo("font-size", true),
        AttributeInfo("font-size-adjust", true),
        AttributeInfo("font-stretch", true),
        AttributeInfo("font-style", true),
        AttributeInfo("font-variant", true),
        AttributeInfo("font-weight", true),
        AttributeInfo("format", false),
        AttributeInfo("from", true),
        AttributeInfo("fx", true),
        AttributeInfo("fy", true),
        AttributeInfo("g1", true),
        AttributeInfo("g2", true),
        AttributeInfo("glyph-name", true),
        AttributeInfo("glyph-orientation-horizontal", true),
        AttributeInfo("glyph-orientation-vertical", true),
        AttributeInfo("glyphRef", false),
        AttributeInfo("gradientTransform", true),
        AttributeInfo("gradientUnits", true),
        AttributeInfo("hanging", true),
        AttributeInfo("hatchContentUnits", true), // SVG 2.0
        AttributeInfo("hatchTransform", true), // SVG 2.0  TODO renamed to transform
        AttributeInfo("hatchUnits", true), // SVG 2.0
        AttributeInfo("height", true),
        AttributeInfo("horiz-adv-x", true),
        AttributeInfo("horiz-origin-x", true),
        AttributeInfo("horiz-origin-y", true),
        AttributeInfo("ideographic", true),
        AttributeInfo("image-rendering", true),
        AttributeInfo("in", true),
        AttributeInfo("in2", true),
        AttributeInfo("intercept", true),
        AttributeInfo("isolation", true),
        AttributeInfo("k", true),
        AttributeInfo("k1", true),
        AttributeInfo("k2", true),
        AttributeInfo("k3", true),
        AttributeInfo("k4", true),
        AttributeInfo("kernelMatrix", true),
        AttributeInfo("kernelUnitLength", true),
        AttributeInfo("kerning", true),
        AttributeInfo("keyPoints", false),
        AttributeInfo("keySplines", true),
        AttributeInfo("keyTimes", true),
        AttributeInfo("lang", true),
        AttributeInfo("lengthAdjust", true),
        AttributeInfo("letter-spacing", true),
        AttributeInfo("lighting-color", true),
        AttributeInfo("limitingConeAngle", true),
        AttributeInfo("local", true),
        AttributeInfo("marker-end", true),
        AttributeInfo("marker-mid", true),
        AttributeInfo("marker-start", true),
        AttributeInfo("markerHeight", true),
        AttributeInfo("markerUnits", true),
        AttributeInfo("markerWidth", true),
        AttributeInfo("mask", true),
        AttributeInfo("maskContentUnits", true),
        AttributeInfo("maskUnits", true),
        AttributeInfo("mathematical", true),
        AttributeInfo("max", true),
        AttributeInfo("media", false),
        AttributeInfo("method", false),
        AttributeInfo("min", true),
        AttributeInfo("mix-blend-mode", true),
        AttributeInfo("mode", true),
        AttributeInfo("name", true),
        AttributeInfo("numOctaves", true),
        AttributeInfo("offset", true),
        AttributeInfo("onabort", false),
        AttributeInfo("onactivate", false),
        AttributeInfo("onbegin", false),
        AttributeInfo("onclick", false),
        AttributeInfo("onend", false),
        AttributeInfo("onerror", false),
        AttributeInfo("onfocusin", false),
        AttributeInfo("onfocusout", false),
        AttributeInfo("onload", true),
        AttributeInfo("onmousedown", false),
        AttributeInfo("onmousemove", false),
        AttributeInfo("onmouseout", false),
        AttributeInfo("onmouseover", false),
        AttributeInfo("onmouseup", false),
        AttributeInfo("onrepeat", false),
        AttributeInfo("onresize", false),
        AttributeInfo("onscroll", false),
        AttributeInfo("onunload", false),
        AttributeInfo("onzoom", false),
        AttributeInfo("opacity", true),
        AttributeInfo("operator", true),
        AttributeInfo("order", true),
        AttributeInfo("orient", true),
        AttributeInfo("orientation", true),
        AttributeInfo("origin", false),
        AttributeInfo("overflow", true),
        AttributeInfo("overline-position", true),
        AttributeInfo("overline-thickness", true),
        AttributeInfo("paint-order", true),
        AttributeInfo("panose-1", true),
        AttributeInfo("path", true),
        AttributeInfo("pathLength", false),
        AttributeInfo("patternContentUnits", true),
        AttributeInfo("patternTransform", true),
        AttributeInfo("patternUnits", true),
        AttributeInfo("pitch", true), // SVG 2.-
        AttributeInfo("pointer-events", true),
        AttributeInfo("points", true),
        AttributeInfo("pointsAtX", true),
        AttributeInfo("pointsAtY", true),
        AttributeInfo("pointsAtZ", true),
        AttributeInfo("preserveAlpha", true),
        AttributeInfo("preserveAspectRatio", true),
        AttributeInfo("primitiveUnits", true),
        AttributeInfo("r", true),
        AttributeInfo("radius", true),
        AttributeInfo("refX", true),
        AttributeInfo("refY", true),
        AttributeInfo("rendering-intent", true),
        AttributeInfo("repeatCount", true),
        AttributeInfo("repeatDur", true),
        AttributeInfo("requiredFeatures", true),
        AttributeInfo("requiredExtensions", true),
        AttributeInfo("restart", true),
        AttributeInfo("result", true),
        AttributeInfo("rotate", true),
        AttributeInfo("rx", true),
        AttributeInfo("ry", true),
        AttributeInfo("scale", true),
        AttributeInfo("seed", true),
        AttributeInfo("shape-inside", true),
        AttributeInfo("shape-margin", true),
        AttributeInfo("shape-outside", true),
        AttributeInfo("shape-padding", true),
        AttributeInfo("shape-rendering", true),
        AttributeInfo("slope", true),
        AttributeInfo("solid-color", true), // SVG 2.0
        AttributeInfo("solid-opacity", true), // SVG 2.0
        AttributeInfo("spacing", false),
        AttributeInfo("specularConstant", true),
        AttributeInfo("specularExponent", true),
        AttributeInfo("spreadMethod", true),
        AttributeInfo("startOffset", true),
        AttributeInfo("stdDeviation", true),
        AttributeInfo("stemh", true),
        AttributeInfo("stemv", true),
        AttributeInfo("stitchTiles", true),
        AttributeInfo("stop-color", true),
        AttributeInfo("stop-opacity", true),
        AttributeInfo("strikethrough-position", true),
        AttributeInfo("strikethrough-thickness", true),
        AttributeInfo("stroke", true),
        AttributeInfo("stroke-dasharray", true),
        AttributeInfo("stroke-dashoffset", true),
        AttributeInfo("stroke-linecap", true),
        AttributeInfo("stroke-linejoin", true),
        AttributeInfo("stroke-miterlimit", true),
        AttributeInfo("stroke-opacity", true),
        AttributeInfo("stroke-width", true),
        AttributeInfo("style", true),
        AttributeInfo("surfaceScale", true),
        AttributeInfo("systemLanguage", true),
        AttributeInfo("tableValues", true),
        AttributeInfo("target", true),
        AttributeInfo("targetX", true),
        AttributeInfo("targetY", true),
        AttributeInfo("text-align", true),
        AttributeInfo("text-anchor", true),
        AttributeInfo("text-decoration", true),
        AttributeInfo("text-decoration-color", true),
        AttributeInfo("text-decoration-line", true),
        AttributeInfo("text-decoration-style", true),
        AttributeInfo("text-indent", true),
        AttributeInfo("text-rendering", true),
        AttributeInfo("text-transform", true),
        AttributeInfo("textLength", true),
        AttributeInfo("title", false),
        AttributeInfo("to", true),
        AttributeInfo("transform", true),
        AttributeInfo("type", true),
        AttributeInfo("u1", true),
        AttributeInfo("u2", true),
        AttributeInfo("underline-position", true),
        AttributeInfo("underline-thickness", true),
        AttributeInfo("unicode", true),
        AttributeInfo("unicode-bidi", true),
        AttributeInfo("unicode-range", true),
        AttributeInfo("units-per-em", true),
        AttributeInfo("v-alphabetic", true),
        AttributeInfo("v-hanging", true),
        AttributeInfo("v-ideographic", true),
        AttributeInfo("v-mathematical", true),
        AttributeInfo("values", true),
        AttributeInfo("version", true),
        AttributeInfo("vert-adv-y", true),
        AttributeInfo("vert-origin-x", true),
        AttributeInfo("vert-origin-y", true),
        AttributeInfo("viewBox", true),
        AttributeInfo("viewTarget", false),
        AttributeInfo("visibility", true),
        AttributeInfo("white-space", true),
        AttributeInfo("width", true),
        AttributeInfo("widths", true),
        AttributeInfo("word-spacing", true),
        AttributeInfo("writing-mode", true),
        AttributeInfo("x", true),
        AttributeInfo("x-height", true),
        AttributeInfo("x1", true),
        AttributeInfo("x2", true),
        AttributeInfo("xChannelSelector", true),
        AttributeInfo("xlink:actuate", true),
        AttributeInfo("xlink:arcrole", true),
        AttributeInfo("xlink:href", true),
        AttributeInfo("xlink:role", true),
        AttributeInfo("xlink:show", true),
        AttributeInfo("xlink:title", true),
        AttributeInfo("xlink:type", true),
        AttributeInfo("xml:base", false),
        AttributeInfo("xml:space", true),
        AttributeInfo("xmlns", false),
        AttributeInfo("xmlns:xlink", false),
        AttributeInfo("y", true),
        AttributeInfo("y1", true),
        AttributeInfo("y2", true),
        AttributeInfo("yChannelSelector", true),
        AttributeInfo("z", true),
        AttributeInfo("zoomAndPan", false),

        // Extra attributes.
        AttributeInfo("id", true),
        AttributeInfo("inkscape:bbox-nodes", true),
        AttributeInfo("inkscape:bbox-paths", true),
        AttributeInfo("inkscape:box3dsidetype", true),
        AttributeInfo("inkscape:collect", true),
        AttributeInfo("inkscape:connection-end", true),
        AttributeInfo("inkscape:connection-end-point", true),
        AttributeInfo("inkscape:connection-points", true),
        AttributeInfo("inkscape:connection-start", true),
        AttributeInfo("inkscape:connection-start-point", true),
        AttributeInfo("inkscape:connector-avoid", true),
        AttributeInfo("inkscape:connector-curvature", true),
        AttributeInfo("inkscape:connector-spacing", true),
        AttributeInfo("inkscape:connector-type", true),
        AttributeInfo("inkscape:corner0", true),
        AttributeInfo("inkscape:corner7", true),
        AttributeInfo("inkscape:current-layer", true),
        AttributeInfo("inkscape:cx", true),
        AttributeInfo("inkscape:cy", true),
        AttributeInfo("inkscape:document-units", true),
        AttributeInfo("inkscape:dstBox", true),
        AttributeInfo("inkscape:dstColumn", true),
        AttributeInfo("inkscape:dstPath", true),
        AttributeInfo("inkscape:dstShape", true),
        AttributeInfo("inkscape:excludeShape", true),
        AttributeInfo("inkscape:expanded", true),
        AttributeInfo("inkscape:flatsided", true),
        AttributeInfo("inkscape:groupmode", true),
        AttributeInfo("inkscape:highlight-color", true),
        AttributeInfo("inkscape:href", true),
        AttributeInfo("inkscape:label", true),
        AttributeInfo("inkscape:layoutOptions", true),
        AttributeInfo("inkscape:object-nodes", true),
        AttributeInfo("inkscape:object-paths", true),
        AttributeInfo("inkscape:original", true),
        AttributeInfo("inkscape:original-d", true),
        AttributeInfo("inkscape:pageopacity", true),
        AttributeInfo("inkscape:pageshadow", true),
        AttributeInfo("inkscape:path-effect", true),
        AttributeInfo("inkscape:persp3d", true),
        AttributeInfo("inkscape:persp3d-origin", true),
        AttributeInfo("inkscape:perspectiveID", true),
        AttributeInfo("inkscape:radius", true),
        AttributeInfo("inkscape:randomized", true),
        AttributeInfo("inkscape:rounded", true),
        AttributeInfo("inkscape:snap-bbox", true),
        AttributeInfo("inkscape:snap-bbox-edge-midpoints", true),
        AttributeInfo("inkscape:snap-bbox-midpoints", true),
        AttributeInfo("inkscape:snap-center", true),
        AttributeInfo("inkscape:snap-global", true),
        AttributeInfo("inkscape:snap-grids", true),
        AttributeInfo("inkscape:snap-intersection-paths", true),
        AttributeInfo("inkscape:snap-midpoints", true),
        AttributeInfo("inkscape:snap-nodes", true),
        AttributeInfo("inkscape:snap-object-midpoints", true),
        AttributeInfo("inkscape:snap-others", true),
        AttributeInfo("inkscape:snap-page", true),
        AttributeInfo("inkscape:snap-path-clip", true),
        AttributeInfo("inkscape:snap-path-mask", true),
        AttributeInfo("inkscape:snap-perpendicular", true),
        AttributeInfo("inkscape:snap-smooth-nodes", true),
        AttributeInfo("inkscape:snap-tangential", true),
        AttributeInfo("inkscape:snap-text-baseline", true),
        AttributeInfo("inkscape:snap-to-guides", true),
        AttributeInfo("inkscape:srcNoMarkup", true),
        AttributeInfo("inkscape:srcPango", true),
        AttributeInfo("inkscape:transform-center-x", true),
        AttributeInfo("inkscape:transform-center-y", true),
        AttributeInfo("inkscape:version", true),
        AttributeInfo("inkscape:vp_x", true),
        AttributeInfo("inkscape:vp_y", true),
        AttributeInfo("inkscape:vp_z", true),
        AttributeInfo("inkscape:window-height", true),
        AttributeInfo("inkscape:window-maximized", true),
        AttributeInfo("inkscape:window-width", true),
        AttributeInfo("inkscape:window-x", true),
        AttributeInfo("inkscape:window-y", true),
        AttributeInfo("inkscape:zoom", true),
        AttributeInfo("osb:paint", true),
        AttributeInfo("sodipodi:arg1", true),
        AttributeInfo("sodipodi:arg2", true),
        AttributeInfo("sodipodi:argument", true),
        AttributeInfo("sodipodi:cx", true),
        AttributeInfo("sodipodi:cy", true),
        AttributeInfo("sodipodi:end", true),
        AttributeInfo("sodipodi:expansion", true),
        AttributeInfo("sodipodi:insensitive", true),
        AttributeInfo("sodipodi:linespacing", true),
        AttributeInfo("sodipodi:nonprintable", true),
        AttributeInfo("sodipodi:open", true),
        AttributeInfo("sodipodi:original", true),
        AttributeInfo("sodipodi:r1", true),
        AttributeInfo("sodipodi:r2", true),
        AttributeInfo("sodipodi:radius", true),
        AttributeInfo("sodipodi:revolution", true),
        AttributeInfo("sodipodi:role", true),
        AttributeInfo("sodipodi:rx", true),
        AttributeInfo("sodipodi:ry", true),
        AttributeInfo("sodipodi:sides", true),
        AttributeInfo("sodipodi:start", true),
        AttributeInfo("sodipodi:t0", true),
        AttributeInfo("sodipodi:version", false),

        // SPMeshPatch
        AttributeInfo("tensor", true),

        // SPNamedView
        AttributeInfo("fit-margin-top", true),
        AttributeInfo("fit-margin-left", true),
        AttributeInfo("fit-margin-right", true),
        AttributeInfo("fit-margin-bottom", true),
        AttributeInfo("units", true),
        AttributeInfo("viewonly", true),
        AttributeInfo("showgrid", true),
//    AttributeInfo("gridtype", true),
        AttributeInfo("showguides", true),
        AttributeInfo("gridtolerance", true),
        AttributeInfo("guidetolerance", true),
        AttributeInfo("objecttolerance", true),
/*    AttributeInfo("gridoriginx", true),
      AttributeInfo("gridoriginy", true),
      AttributeInfo("gridspacingx", true),
      AttributeInfo("gridspacingy", true),
      AttributeInfo("gridanglex", true),
      AttributeInfo("gridanglez", true),
      AttributeInfo("gridcolor", true),
      AttributeInfo("gridopacity", true),
      AttributeInfo("gridempcolor", true),
      AttributeInfo("gridempopacity", true),
      AttributeInfo("gridempspacing", true), */
        AttributeInfo("guidecolor", true),
        AttributeInfo("guideopacity", true),
        AttributeInfo("guidehicolor", true),
        AttributeInfo("guidehiopacity", true),
        AttributeInfo("showborder", true),
        AttributeInfo("inkscape:showpageshadow", true),
        AttributeInfo("borderlayer", true),
        AttributeInfo("bordercolor", true),
        AttributeInfo("borderopacity", true),
        AttributeInfo("pagecolor", true),

        // SPGuide
        AttributeInfo("position", true)
    };

    size_t count = sizeof(all_attrs) / sizeof(all_attrs[0]);
    std::vector<AttributeInfo> vect(all_attrs, all_attrs + count);
    EXPECT_GT(vect.size(), size_t(100)); // should be more than
    return vect;
}

/**
 * Returns a vector with counts for all IDs up to the highest known value.
 *
 * The index is the ID, and the value is the number of times that ID is seen.
 */
std::vector<size_t> getIdIds()
{
    std::vector<size_t> ids;
    std::vector<AttributeInfo> all_attrs = getKnownAttrs();
    ids.reserve(all_attrs.size()); // minimize memory thrashing
    for (AttrItr it(all_attrs.begin()); it != all_attrs.end(); ++it) {
        unsigned int id = sp_attribute_lookup(it->attr.c_str());
        if (id >= ids.size()) {
            ids.resize(id + 1);
        }
        ids[id]++;
    }

    return ids;
}

// Ensure 'supported' value for each known attribute is correct.
TEST(AttributesTest, SupportedKnown)
{
    std::vector<AttributeInfo> all_attrs = getKnownAttrs();
    for (AttrItr it(all_attrs.begin()); it != all_attrs.end(); ++it) {
        unsigned int id = sp_attribute_lookup(it->attr.c_str());
        EXPECT_EQ(it->supported, id != 0u) << "Matching for attribute '" << it->attr << "'";
    }
}

// Ensure names of known attributes are preserved when converted to id and back.
TEST(AttributesTest, NameRoundTrip)
{
    std::vector<AttributeInfo> all_attrs = getKnownAttrs();
    for (AttrItr it(all_attrs.begin()); it != all_attrs.end(); ++it) {
        if (it->supported) {
            unsigned int id = sp_attribute_lookup(it->attr.c_str());
            char const *redoneName = reinterpret_cast<char const *>(sp_attribute_name(id));
            EXPECT_TRUE(redoneName != NULL) << "For attribute '" << it->attr << "'";
            if (redoneName) {
                EXPECT_EQ(it->attr, redoneName);
            }
        }
    }
}

/* Test for any attributes that this test program doesn't know about.
 *
 * If any are found, then:
 *
 *   If it is in the `inkscape:' namespace then simply add it to all_attrs with
 *   `true' as the second field (`supported').
 *
 *   If it is in the `sodipodi:' namespace then check the spelling against sodipodi
 *   sources.  If you don't have sodipodi sources, then don't add it: leave to someone
 *   else.
 *
 *   Otherwise, it's probably a bug: ~all SVG 1.1 attributes should already be
 *   in the all_attrs table.  However, the comment above all_attrs does mention
 *   some things missing from attindex.html, so there may be more.  Check the SVG
 *   spec.  Another possibility is that the attribute is new in SVG 1.2.  In this case,
 *   check the spelling against the [draft] SVG 1.2 spec before adding to all_attrs.
 *   (If you can't be bothered checking the spec, then don't update all_attrs.)
 *
 *   If the attribute isn't in either SVG 1.1 or 1.2 then it's probably a mistake
 *   for it not to be in the inkscape namespace.  (Not sure about attributes used only
 *   on elements in the inkscape namespace though.)
 *
 *   In any case, make sure that the attribute's source is documented accordingly.
 */
TEST(AttributesTest, ValuesAreKnown)
{
    std::vector<size_t> ids = getIdIds();
    for (size_t i = FIRST_VALID_ID; i < ids.size(); ++i) {
        if (!ids[i]) {
            unsigned char const *name = sp_attribute_name(i);
            EXPECT_TRUE(ids[i] > 0) << "Attribute string with enum " << i << " {" << name << "} not handled";
        }
    }
}

// Ensure two different names aren't mapped to the same enum value.
TEST(AttributesTest, ValuesUnique)
{
    std::vector<size_t> ids = getIdIds();
    for (size_t i = FIRST_VALID_ID; i < ids.size(); ++i) {
        EXPECT_LE(ids[i], size_t(1)) << "Attribute enum " << i << " used for multiple strings"
                                     << " including {" << sp_attribute_name(i) << "}";
    }
}

} // namespace

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
