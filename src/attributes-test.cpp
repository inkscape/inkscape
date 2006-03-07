#include <vector>
#include "utest/utest.h"
#include "attributes.h"
#include "streq.h"

/* Extracted mechanically from http://www.w3.org/TR/SVG11/attindex.html:

   tidy -wrap 999 -asxml < attindex.html 2>/dev/null |
     tr -d \\n |
     sed 's,<tr>,@,g' |
     tr @ \\n |
     sed 's,</td>.*,,;s,^<td>,,;1,/^%/d;/^%/d;s,^,    {",;s/$/", false},/' |
     uniq

   attindex.html lacks attributeName, begin, additive, font, marker;
   I've added these manually.
*/
static struct {char const *attr; bool supported;} const all_attrs[] = {
    {"attributeName", true},
    {"begin", true},
    {"additive", true},
    {"font", true},
    {"marker", true},
    {"line-height", true},

    {"accent-height", false},
    {"accumulate", true},
    {"alignment-baseline", true},
    {"alphabetic", false},
    {"amplitude", false},
    {"animate", false},
    {"arabic-form", false},
    {"ascent", false},
    {"attributeType", true},
    {"azimuth", false},
    {"baseFrequency", false},
    {"baseline-shift", true},
    {"baseProfile", false},
    {"bbox", false},
    {"bias", false},
    {"block-progression", true},
    {"by", true},
    {"calcMode", true},
    {"cap-height", false},
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
    {"descent", false},
    {"diffuseConstant", false},
    {"direction", true},
    {"display", true},
    {"divisor", false},
    {"dominant-baseline", true},
    {"dur", true},
    {"dx", true},
    {"dy", true},
    {"edgeMode", false},
    {"elevation", false},
    {"enable-background", true},
    {"end", true},
    {"exponent", false},
    {"externalResourcesRequired", false},
    {"feColorMatrix", false},
    {"feComposite", false},
    {"feGaussianBlur", false},
    {"feMorphology", false},
    {"feTile", false},
    {"fill", true},
    {"fill-opacity", true},
    {"fill-rule", true},
    {"filter", true},
    {"filterRes", false},
    {"filterUnits", false},
    {"flood-color", true},
    {"flood-opacity", true},
    {"font-family", true},
    {"font-size", true},
    {"font-size-adjust", true},
    {"font-stretch", true},
    {"font-style", true},
    {"font-variant", true},
    {"font-weight", true},
    {"format", false},
    {"from", true},
    {"fx", true},
    {"fy", true},
    {"g1", false},
    {"g2", false},
    {"glyph-name", false},
    {"glyph-orientation-horizontal", true},
    {"glyph-orientation-vertical", true},
    {"glyphRef", false},
    {"gradientTransform", true},
    {"gradientUnits", true},
    {"hanging", false},
    {"height", true},
    {"horiz-adv-x", false},
    {"horiz-origin-x", false},
    {"horiz-origin-y", false},
    {"ideographic", false},
    {"image-rendering", true},
    {"in", false},
    {"in2", false},
    {"intercept", false},
    {"k", false},
    {"k1", false},
    {"k2", false},
    {"k3", false},
    {"k4", false},
    {"kernelMatrix", false},
    {"kernelUnitLength", false},
    {"kerning", true},
    {"keyPoints", false},
    {"keySplines", true},
    {"keyTimes", true},
    {"lang", false},
    {"lengthAdjust", false},
    {"letter-spacing", true},
    {"lighting-color", true},
    {"limitingConeAngle", false},
    {"local", false},
    {"marker-end", true},
    {"marker-mid", true},
    {"marker-start", true},
    {"markerHeight", true},
    {"markerUnits", true},
    {"markerWidth", true},
    {"mask", true},
    {"maskContentUnits", true},
    {"maskUnits", true},
    {"mathematical", false},
    {"max", true},
    {"media", false},
    {"method", false},
    {"min", true},
    {"mode", false},
    {"name", false},
    {"numOctaves", false},
    {"offset", true},
    {"onabort", false},
    {"onactivate", false},
    {"onbegin", false},
    {"onclick", false},
    {"onend", false},
    {"onerror", false},
    {"onfocusin", false},
    {"onfocusout", false},
    {"onload", false},
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
    {"operator", false},
    {"order", false},
    {"orient", true},
    {"orientation", true},
    {"origin", false},
    {"overflow", true},
    {"overline-position", false},
    {"overline-thickness", false},
    {"panose-1", false},
    {"path", false},
    {"pathLength", false},
    {"patternContentUnits", true},
    {"patternTransform", true},
    {"patternUnits", true},
    {"pointer-events", true},
    {"points", true},
    {"pointsAtX", false},
    {"pointsAtY", false},
    {"pointsAtZ", false},
    {"preserveAlpha", false},
    {"preserveAspectRatio", true},
    {"primitiveUnits", false},
    {"r", true},
    {"radius", false},
    {"refX", true},
    {"refY", true},
    {"rendering-intent", false},
    {"repeatCount", true},
    {"repeatDur", true},
    {"requiredExtensions", false},
    {"restart", true},
    {"result", false},
    {"rotate", true},
    {"rx", true},
    {"ry", true},
    {"scale", false},
    {"seed", false},
    {"shape-rendering", true},
    {"slope", false},
    {"spacing", false},
    {"specularConstant", false},
    {"specularExponent", false},
    {"spreadMethod", true},
    {"startOffset", true},
    {"stdDeviation", false},
    {"stemh", false},
    {"stemv", false},
    {"stitchTiles", false},
    {"stop-color", true},
    {"stop-opacity", true},
    {"strikethrough-position", false},
    {"strikethrough-thickness", false},
    {"stroke", true},
    {"stroke-dasharray", true},
    {"stroke-dashoffset", true},
    {"stroke-linecap", true},
    {"stroke-linejoin", true},
    {"stroke-miterlimit", true},
    {"stroke-opacity", true},
    {"stroke-width", true},
    {"style", true},
    {"surfaceScale", false},
    {"systemLanguage", false},
    {"tableValues", false},
    {"target", true},
    {"targetX", false},
    {"targetY", false},
    {"text-align", true},
    {"text-anchor", true},
    {"text-decoration", true},
    {"text-indent", true},
    {"text-rendering", true},
    {"text-transform", true},
    {"textLength", false},
    {"title", false},
    {"to", true},
    {"transform", true},
    {"type", true},
    {"u1", false},
    {"u2", false},
    {"underline-position", false},
    {"underline-thickness", false},
    {"unicode", false},
    {"unicode-bidi", true},
    {"unicode-range", false},
    {"units-per-em", false},
    {"v-alphabetic", false},
    {"v-hanging", false},
    {"v-ideographic", false},
    {"v-mathematical", false},
    {"values", true},
    {"version", true},
    {"vert-adv-y", false},
    {"vert-origin-x", false},
    {"vert-origin-y", false},
    {"viewBox", true},
    {"viewTarget", false},
    {"visibility", true},
    {"width", true},
    {"widths", false},
    {"word-spacing", true},
    {"writing-mode", true},
    {"x", true},
    {"x-height", false},
    {"x1", true},
    {"x2", true},
    {"xChannelSelector", false},
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
    {"yChannelSelector", false},
    {"z", false},
    {"zoomAndPan", false},

    /* Extra attributes. */
    {"id", true},
    {"inkscape:collect", true},
    {"inkscape:document-units", true},
    {"inkscape:label", true},
    {"sodipodi:insensitive", true},
    {"sodipodi:nonprintable", true},
    {"inkscape:groupmode", true},
    {"sodipodi:version", true},
    {"inkscape:version", true},
    {"inkscape:object-bbox", true},
    {"inkscape:object-points", true},
    {"inkscape:object-paths", true},
    {"inkscape:object-nodes", true},
    {"inkscape:pageopacity", true},
    {"inkscape:pageshadow", true},
    {"inkscape:transform-center-x", true},
    {"inkscape:transform-center-y", true},
    {"inkscape:zoom", true},
    {"inkscape:cx", true},
    {"inkscape:cy", true},
    {"inkscape:window-width", true},
    {"inkscape:window-height", true},
    {"inkscape:window-x", true},
    {"inkscape:window-y", true},
    {"inkscape:grid-bbox", true},
    {"inkscape:guide-bbox", true},
    {"inkscape:grid-points", true},
    {"inkscape:guide-points", true},
    {"inkscape:current-layer", true},
    {"inkscape:connector-type", true},
    {"inkscape:connection-start", true},
    {"inkscape:connection-end", true},
    {"inkscape:connector-avoid", true},
    {"inkscape:connector-spacing", true},
    {"sodipodi:cx", true},
    {"sodipodi:cy", true},
    {"sodipodi:rx", true},
    {"sodipodi:ry", true},
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

    /* SPNamedView */
    {"viewonly", true},
    {"showgrid", true},
    {"showguides", true},
    {"gridtolerance", true},
    {"guidetolerance", true},
    {"gridoriginx", true},
    {"gridoriginy", true},
    {"gridspacingx", true},
    {"gridspacingy", true},
    {"gridcolor", true},
    {"gridopacity", true},
    {"gridempcolor", true},
    {"gridempopacity", true},
    {"gridempspacing", true},
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

    /* SPGuide */
    {"position", true}

};

static bool
test_attributes()
{
    utest_start("attributes");

    std::vector<bool> ids;
    ids.reserve(256);
    UTEST_TEST("attribute lookup") {
        for (unsigned i = 0; i < G_N_ELEMENTS(all_attrs); ++i) {
            char const *const attr_str = all_attrs[i].attr;
            unsigned const id = sp_attribute_lookup(attr_str);
            bool const recognized(id);
            UTEST_ASSERT(recognized == all_attrs[i].supported);
            if (recognized) {
                if (ids.size() <= id) {
                    ids.resize(id + 1);
                }
                UTEST_ASSERT(!ids[id]);
                ids[id] = true;

                unsigned char const *reverse_ustr = sp_attribute_name(id);
                char const *reverse_str = reinterpret_cast<char const *>(reverse_ustr);
                UTEST_ASSERT(streq(reverse_str, attr_str));
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
                unsigned char const *str = sp_attribute_name(id);
                printf("%s\n", (const char *)str); /* Apparently printf doesn't like unsigned strings -- Ted */
                found = true;
            }
        }
        UTEST_ASSERT(!found);
    }

    return utest_end();
}

int main()
{
    return ( test_attributes()
             ? EXIT_SUCCESS
             : EXIT_FAILURE );
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
