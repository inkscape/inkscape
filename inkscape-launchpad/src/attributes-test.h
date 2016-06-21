
#ifndef SEEN_ATTRIBUTES_TEST_H
#define SEEN_ATTRIBUTES_TEST_H

#include <cxxtest/TestSuite.h>

#include <vector>
#include <glib.h>
#include <glib/gprintf.h>
#include "attributes.h"
#include "streq.h"

class AttributesTest : public CxxTest::TestSuite
{
public:

    AttributesTest()
    {
    }
    virtual ~AttributesTest() {}

// createSuite and destroySuite get us per-suite setup and teardown
// without us having to worry about static initialization order, etc.
    static AttributesTest *createSuite() { return new AttributesTest(); }
    static void destroySuite( AttributesTest *suite ) { delete suite; }


    void testAttributes()
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
   SVG 2: text-decoration-fill, text-decoration-stroke
   SVG 2: solid-color, solid-opacity
   SVG 2: Hatches and Meshes, radial gradient 'fr'
   CSS 3: text-orientation
   CSS 3: font-variant-xxx, font-feature-settings
*/
struct {char const *attr; bool supported;} const all_attrs[] = {
    {"attributeName", true},
    {"begin", true},
    {"additive", true},
    {"font", true},
    {"-inkscape-font-specification", true}, // TODO look into this attribute's name
    {"marker", true},
    {"line-height", true},

    {"accent-height", true},
    {"accumulate", true},
    {"alignment-baseline", true},
    {"alphabetic", true},
    {"amplitude", true},
    {"animate", false},
    {"arabic-form", true},
    {"ascent", true},
    {"attributeType", true},
    {"azimuth", true},
    {"baseFrequency", true},
    {"baseline-shift", true},
    {"baseProfile", false},
    {"bbox", true},
    {"bias", true},
    {"by", true},
    {"calcMode", true},
    {"cap-height", true},
    {"class", false},
    {"clip", true},
    {"clip-path", true},
    {"clip-rule", true},
    {"clipPathUnits", true},
    {"color", true},
    {"color-interpolation", true},
    {"color-interpolation-filters", true},
    {"color-profile", true},
    {"color-rendering", true},
    {"contentScriptType", false},
    {"contentStyleType", false},
    {"cursor", true},
    {"cx", true},
    {"cy", true},
    {"d", true},
    {"descent", true},
    {"diffuseConstant", true},
    {"direction", true},
    {"display", true},
    {"divisor", true},
    {"dominant-baseline", true},
    {"dur", true},
    {"dx", true},
    {"dy", true},
    {"edgeMode", true},
    {"elevation", true},
    {"enable-background", true},
    {"end", true},
    {"exponent", true},
    {"externalResourcesRequired", false},
    {"fill", true},
    {"fill-opacity", true},
    {"fill-rule", true},
    {"filter", true},
    {"filterRes", true},
    {"filterUnits", true},
    {"flood-color", true},
    {"flood-opacity", true},
    {"font-family", true},
    {"font-feature-settings", true},
    {"font-size", true},
    {"font-size-adjust", true},
    {"font-stretch", true},
    {"font-style", true},
    {"font-variant", true},
    {"font-variant-ligatures", true},
    {"font-variant-position", true},
    {"font-variant-caps", true},
    {"font-variant-numeric", true},
    {"font-variant-east-asian", true},
    {"font-variant-alternates", true},
    {"font-weight", true},
    {"format", false},
    {"from", true},
    {"fx", true},
    {"fy", true},
    {"fr", true},
    {"g1", true},
    {"g2", true},
    {"glyph-name", true},
    {"glyph-orientation-horizontal", true},
    {"glyph-orientation-vertical", true},
    {"glyphRef", false},
    {"gradientTransform", true},
    {"gradientUnits", true},
    {"hanging", true},
    {"height", true},
    {"horiz-adv-x", true},
    {"horiz-origin-x", true},
    {"horiz-origin-y", true},
    {"ideographic", true},
    {"image-rendering", true},
    {"in", true},
    {"in2", true},
    {"intercept", true},
    {"isolation", true},
    {"k", true},
    {"k1", true},
    {"k2", true},
    {"k3", true},
    {"k4", true},
    {"kernelMatrix", true},
    {"kernelUnitLength", true},
    {"kerning", true},
    {"keyPoints", false},
    {"keySplines", true},
    {"keyTimes", true},
    {"lang", true},
    {"lengthAdjust", true},
    {"letter-spacing", true},
    {"lighting-color", true},
    {"limitingConeAngle", true},
    {"local", true},
    {"marker-end", true},
    {"marker-mid", true},
    {"marker-start", true},
    {"markerHeight", true},
    {"markerUnits", true},
    {"markerWidth", true},
    {"mask", true},
    {"maskContentUnits", true},
    {"maskUnits", true},
    {"mathematical", true},
    {"max", true},
    {"media", false},
    {"method", false},
    {"min", true},
    {"mix-blend-mode", true},
    {"mode", true},
    {"name", true},
    {"numOctaves", true},
    {"offset", true},
    {"onabort", false},
    {"onactivate", false},
    {"onbegin", false},
    {"onclick", false},
    {"onend", false},
    {"onerror", false},
    {"onfocusin", false},
    {"onfocusout", false},
    {"onload", true},
    {"onmousedown", false},
    {"onmousemove", false},
    {"onmouseout", false},
    {"onmouseover", false},
    {"onmouseup", false},
    {"onrepeat", false},
    {"onresize", false},
    {"onscroll", false},
    {"onunload", false},
    {"onzoom", false},
    {"opacity", true},
    {"operator", true},
    {"order", true},
    {"orient", true},
    {"orientation", true},
    {"origin", false},
    {"overflow", true},
    {"overline-position", true},
    {"overline-thickness", true},
    {"paint-order", true},
    {"panose-1", true},
    {"path", true},
    {"pathLength", false},
    {"patternContentUnits", true},
    {"patternTransform", true},
    {"patternUnits", true},
    {"pointer-events", true},
    {"points", true},
    {"pointsAtX", true},
    {"pointsAtY", true},
    {"pointsAtZ", true},
    {"preserveAlpha", true},
    {"preserveAspectRatio", true},
    {"primitiveUnits", true},
    {"r", true},
    {"radius", true},
    {"refX", true},
    {"refY", true},
    {"rendering-intent", true},
    {"repeatCount", true},
    {"repeatDur", true},
    {"requiredFeatures", true},
    {"requiredExtensions", true},
    {"restart", true},
    {"result", true},
    {"rotate", true},
    {"rx", true},
    {"ry", true},
    {"scale", true},
    {"seed", true},
    {"shape-inside", true},
    {"shape-margin", true},
    {"shape-outside", true},
    {"shape-padding", true},
    {"shape-rendering", true},
    {"slope", true},
    {"spacing", false},
    {"specularConstant", true},
    {"specularExponent", true},
    {"spreadMethod", true},
    {"startOffset", true},
    {"stdDeviation", true},
    {"stemh", true},
    {"stemv", true},
    {"stitchTiles", true},
    {"stop-color", true},
    {"stop-opacity", true},
    {"strikethrough-position", true},
    {"strikethrough-thickness", true},
    {"stroke", true},
    {"stroke-dasharray", true},
    {"stroke-dashoffset", true},
    {"stroke-linecap", true},
    {"stroke-linejoin", true},
    {"stroke-miterlimit", true},
    {"stroke-opacity", true},
    {"stroke-width", true},
    {"style", true},
    {"surfaceScale", true},
    {"systemLanguage", true},
    {"tableValues", true},
    {"target", true},
    {"targetX", true},
    {"targetY", true},
    {"text-align", true},
    {"text-anchor", true},
    {"text-decoration", true},
    {"text-decoration-line", true},
    {"text-decoration-style", true},
    {"text-decoration-color", true},
    {"text-decoration-fill", true},
    {"text-decoration-stroke", true},
    {"text-indent", true},
    {"text-rendering", true},
    {"text-transform", true},
    {"textLength", true},
    {"title", false},
    {"to", true},
    {"transform", true},
    {"type", true},
    {"u1", true},
    {"u2", true},
    {"underline-position", true},
    {"underline-thickness", true},
    {"unicode", true},
    {"unicode-bidi", true},
    {"unicode-range", true},
    {"units-per-em", true},
    {"v-alphabetic", true},
    {"v-hanging", true},
    {"v-ideographic", true},
    {"v-mathematical", true},
    {"values", true},
    {"version", true},
    {"vert-adv-y", true},
    {"vert-origin-x", true},
    {"vert-origin-y", true},
    {"viewBox", true},
    {"viewTarget", false},
    {"visibility", true},
    {"white-space", true},
    {"width", true},
    {"widths", true},
    {"word-spacing", true},
    {"writing-mode", true},
    {"text-orientation", true},
    {"x", true},
    {"x-height", true},
    {"x1", true},
    {"x2", true},
    {"xChannelSelector", true},
    {"xlink:actuate", true},
    {"xlink:arcrole", true},
    {"xlink:href", true},
    {"xlink:role", true},
    {"xlink:show", true},
    {"xlink:title", true},
    {"xlink:type", true},
    {"xml:base", false},
    {"xml:space", true},
    {"xmlns", false},
    {"xmlns:xlink", false},
    {"y", true},
    {"y1", true},
    {"y2", true},
    {"yChannelSelector", true},
    {"z", true},
    {"zoomAndPan", false},

    /* Extra attributes. */
    {"id", true},
    {"sodipodi:docname", true},
    {"sodipodi:insensitive", true},
    {"sodipodi:type", true},
    {"inkscape:collect", true},
    {"inkscape:document-units", true},
    {"inkscape:label", true},
    {"inkscape:groupmode", true},
    {"inkscape:version", true},
    {"inkscape:object-paths", true},

    {"inkscape:original-d", true},
    {"inkscape:pageopacity", true},
    {"inkscape:pageshadow", true},
    {"inkscape:path-effect", true},

    // SPItem
    {"inkscape:transform-center-x", true},
    {"inkscape:transform-center-y", true},
    {"inkscape:highlight-color", true},

    // Measure tool
    {"inkscape:measure-start", true},
    {"inkscape:measure-end", true},

    // Spray tool
    {"inkscape:spray-origin", true},

    // Connector tool
    {"inkscape:connector-type", true},
    {"inkscape:connection-start", true},
    {"inkscape:connection-end", true},
    {"inkscape:connection-points", true},
    {"inkscape:connection-start-point", true},
    {"inkscape:connection-end-point", true},
    {"inkscape:connector-curvature", true},
    {"inkscape:connector-avoid", true},
    {"inkscape:connector-spacing", true},

    // Ellipse, Spiral, Star
    {"sodipodi:cx", true},
    {"sodipodi:cy", true},
    {"sodipodi:rx", true},
    {"sodipodi:ry", true},

    // Box tool
    {"inkscape:perspectiveID", true},
    {"inkscape:corner0", true},
    {"inkscape:corner7", true},
    {"inkscape:box3dsidetype", true},
    {"inkscape:persp3d", true},
    {"inkscape:vp_x", true},
    {"inkscape:vp_y", true},
    {"inkscape:vp_z", true},
    {"inkscape:persp3d-origin", true},

    // Star tool
    {"sodipodi:start", true},
    {"sodipodi:end", true},
    {"sodipodi:open", true},
    {"sodipodi:sides", true},
    {"sodipodi:r1", true},
    {"sodipodi:r2", true},
    {"sodipodi:arg1", true},
    {"sodipodi:arg2", true},
    {"inkscape:flatsided", true},
    {"inkscape:rounded", true},
    {"inkscape:randomized", true},
    {"sodipodi:expansion", true},
    {"sodipodi:revolution", true},
    {"sodipodi:radius", true},
    {"sodipodi:argument", true},
    {"sodipodi:t0", true},
    {"sodipodi:original", true},
    {"inkscape:original", true},
    {"inkscape:href", true},
    {"inkscape:radius", true},
    {"sodipodi:role", true},
    {"sodipodi:linespacing", true},
    {"inkscape:srcNoMarkup", true},
    {"inkscape:srcPango", true},
    {"inkscape:dstShape", true},
    {"inkscape:dstPath", true},
    {"inkscape:dstBox", true},
    {"inkscape:dstColumn", true},
    {"inkscape:excludeShape", true},
    {"inkscape:layoutOptions", true},
    {"osb:paint", true},

    /* SPSolidColor" */
    {"solid-color", true},
    {"solid-opacity", true},

    /* SPMeshPatch */
    {"tensor", true},

    /* SPHash */
    {"hatchUnits", true},
    {"hatchContentUnits", true},
    {"hatchTransform", true},
    {"pitch", true},
    
    /* SPNamedView */
    {"fit-margin-top", true},
    {"fit-margin-left", true},
    {"fit-margin-right", true},
    {"fit-margin-bottom", true},
    {"units", true},
    {"viewonly", true},
    {"showgrid", true},
//    {"gridtype", true},
    {"showguides", true},
//    {"inkscape:lockguides", false}, //not sure about uncomment
    {"gridtolerance", true},
    {"guidetolerance", true},
    {"objecttolerance", true},
/*    {"gridoriginx", true},
    {"gridoriginy", true},
    {"gridspacingx", true},
    {"gridspacingy", true},
    {"gridanglex", true},
    {"gridanglez", true},
    {"gridcolor", true},
    {"gridopacity", true},
    {"gridempcolor", true},
    {"gridempopacity", true},
    {"gridempspacing", true}, */
    {"guidecolor", true},
    {"guideopacity", true},
    {"guidehicolor", true},
    {"guidehiopacity", true},
    {"showborder", true},
    {"inkscape:showpageshadow", true},
    {"borderlayer", true},
    {"bordercolor", true},
    {"borderopacity", true},
    {"pagecolor", true},

    {"inkscape:zoom", true},
    {"inkscape:cx", true},
    {"inkscape:cy", true},
    {"inkscape:window-width", true},
    {"inkscape:window-height", true},
    {"inkscape:window-x", true},
    {"inkscape:window-y", true},
    {"inkscape:window-maximized", true},
    {"inkscape:current-layer", true},
    {"inkscape:pagecheckerboard", true},

    /* SPGuide */
    {"position", true},
    {"inkscape:color", true},
    {"inkscape:lockguides", true},
    {"inkscape:locked", true},

    /* Snapping */
    {"inkscape:snap-perpendicular", true},
    {"inkscape:snap-tangential", true},
    {"inkscape:snap-path-clip", true},
    {"inkscape:snap-path-mask", true},
    {"inkscape:object-nodes", true},
    {"inkscape:bbox-paths", true},
    {"inkscape:bbox-nodes", true},
    {"inkscape:snap-page", true},
    {"inkscape:snap-global", true},
    {"inkscape:snap-bbox", true},
    {"inkscape:snap-nodes", true},
    {"inkscape:snap-others", true},
    {"inkscape:snap-from-guide", true},
    {"inkscape:snap-center", true},
    {"inkscape:snap-smooth-nodes", true},
    {"inkscape:snap-midpoints", true},
    {"inkscape:snap-object-midpoints", true},
    {"inkscape:snap-text-baseline", true},
    {"inkscape:snap-bbox-edge-midpoints", true},
    {"inkscape:snap-bbox-midpoints", true},
    {"inkscape:snap-grids", true},
    {"inkscape:snap-to-guides", true},
    {"inkscape:snap-intersection-paths", true},

    /* SPTag */
    {"inkscape:expanded", true}
};



        std::vector<bool> ids;
        ids.reserve(256);

        for (unsigned i = 0; i < G_N_ELEMENTS(all_attrs); ++i) {
            char const *const attr_str = all_attrs[i].attr;
            unsigned const id = sp_attribute_lookup(attr_str);
            bool const recognized(id);
            TSM_ASSERT_EQUALS( std::string(all_attrs[i].attr), recognized, all_attrs[i].supported );
            if (recognized) {
                if (ids.size() <= id) {
                    ids.resize(id + 1);
                }
                TS_ASSERT(!ids[id]);
                ids[id] = true;

                unsigned char const *reverse_ustr = sp_attribute_name(id);
                char const *reverse_str = reinterpret_cast<char const *>(reverse_ustr);
                TS_ASSERT(streq(reverse_str, attr_str));
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
        bool found = false;
        unsigned const n_ids = ids.size();
        for (unsigned id = 1; id < n_ids; ++id) {
            if (!ids[id]) {
                gchar* tmp = g_strdup_printf( "Attribute string with enum %d {%s} not handled", id, sp_attribute_name(id) );
                TS_WARN( std::string((const char*)tmp) );
                g_free( tmp );
                found = true;
            }
        }
        TS_ASSERT(!found);

        for ( unsigned int index = 1; index < n_ids; index++ ) {
            guchar const* name = sp_attribute_name(index);
            unsigned int postLookup = sp_attribute_lookup( reinterpret_cast<gchar const*>(name) );
            TSM_ASSERT_EQUALS( std::string("Enum round-trip through string {") + (char const*)name + "} failed.", index, postLookup );
        }

    }
};

#endif // SEEN_ATTRIBUTES_TEST_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
