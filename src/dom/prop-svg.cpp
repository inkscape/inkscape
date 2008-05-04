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
 * Copyright (C) 2005 Bob Jamison
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
#include <stdio.h>
#include <stdlib.h>


struct SvgProp_def
{
    char const *name;
    char const *values;
    char const *defaultValue;
    char const *appliesTo;
    bool inherited;
    char const *percentages;
    char const *mediaGroups;
    bool animatable;
};

typedef struct SvgProp_def SvgProp;

static SvgProp svgProps[] =
{

{
"alignment-baseline",
"auto | baseline | before-edge | text-before-edge | middle | central | after-edge | text-after-edge | ideographic | alphabetic | hanging | mathematical | inherit",
"see property description",
"'tspan', 'tref', 'altGlyph', 'textPath' elements",
false,
"",
"visual",
true
},

{
"baseline-shift",
"baseline | sub | super | <percentage> | <length> | inherit",
"baseline",
"tspan', 'tref', 'altGlyph', 'textPath' elements",
false,
"refers to the 'line-height' of the 'text' element, which in the case of SVG is defined to be equal to the 'font-size",
"visual",
"yes (non-additive, 'set' and 'animate' elements only)"
},

{
"clip",
"<shape> | auto | inherit",
"auto",
"elements which establish a new viewport, 'pattern' elements and 'marker' elements",
false,
"",
"visual",
true
},

{
"clip-path",
"<uri> | none | inherit",
"none",
"container elements and graphics elements",
false,
"",
"visual",
true
},

{
"clip-rule",
"nonzero | evenodd | inherit",
"nonzero",
"graphics elements within a 'clipPath' element",
true,
"",
"visual",
true
},

{
"color",
"<color> | inherit",
"depends on user agent",
"elements to which properties 'fill', 'stroke', 'stop-color', 'flood-color', 'lighting-color' apply",
true,
"",
"visual",
true
},

{
"color-interpolation",
"auto | sRGB | linearRGB | inherit",
"sRGB",
"container elements, graphics elements and 'animateColor",
true,
"",
"visual",
true
},

{
"color-interpolation-filters",
"auto | sRGB | linearRGB | inherit",
"linearRGB",
"filter primitives",
true,
"",
"visual",
true
},

{
"color-profile",
"auto | sRGB | <name> | <uri> | inherit",
"auto",
"'image' elements that refer to raster images",
true,
"",
"visual",
true
},

{
"color-rendering",
"auto | optimizeSpeed | optimizeQuality | inherit",
"auto",
"container elements, graphics elements and 'animateColor",
true,
"",
"visual",
true
},

{
"cursor",
"[ [<uri> ,]* [ auto | crosshair | default | pointer | move | e-resize | ne-resize | nw-resize | n-resize | se-resize | sw-resize | s-resize | w-resize| text | wait | help ] ] | inherit",
"auto",
"container elements and graphics elements",
true,
"",
"visual, interactive",
true
},

{
"direction",
"ltr | rtl | inherit",
"ltr",
"text content elements",
true,
"",
"visual",
false
},

{
"display",
"inline | block | list-item | run-in | compact | marker | table | inline-table | table-row-group | table-header-group | table-footer-group | table-row | table-column-group | table-column | table-cell | table-caption | none | inherit",
"inline",
"'svg', 'g', 'switch', 'a', 'foreignObject', graphics elements (including the 'text' element) and text sub-elements (i.e., 'tspan', 'tref', 'altGlyph', 'textPath')",
false,
"",
"all",
true
},

{
"dominant-baseline",
"auto | use-script | no-change | reset-size | ideographic | alphabetic | hanging | mathematical | central | middle | text-after-edge | text-before-edge | inherit",
"auto",
"text content elements",
false,
"",
"visual",
true
},

{
"enable-background",
"accumulate | new [ <x> <y> <width> <height> ] | inherit",
"accumulate",
"container elements",
false,
"",
"visual",
false
},

{
"fill",
"<paint> (See Specifying paint)",
"black",
"shapes and text content elements",
true,
"",
"visual",
true
},

{
"fill-opacity",
"<opacity-value> | inherit",
"1",
"shapes and text content elements",
true,
"",
"visual",
true
},

{
"fill-rule",
"nonzero | evenodd | inherit",
"nonzero",
"shapes and text content elements",
true,
"",
"visual",
true
},

{
"filter",
"<uri> | none | inherit",
"none",
"container elements and graphics elements",
false,
"",
"visual",
true
},

{
"flood-color",
"currentColor | <color> [icc-color(<name>[,<icccolorvalue>]*)] | inherit",
"black",
"'feFlood' elements",
false,
"",
"visual",
true
},

{
"flood-opacity",
"<opacity-value> | inherit",
"1",
"'feFlood' elements",
false,
"",
"visual",
true
},

{
"font",
"[ [ 'font-style' || 'font-variant' || 'font-weight' ]? 'font-size' [ / 'line-height' ]? 'font-family' ] | caption | icon | menu | message-box | small-caption | status-bar | inherit",
"see individual properties",
"text content elements",
true,
"allowed on 'font-size' and 'line-height' ('line-height' same as 'font-size' in SVG)",
"visual",
"yes (non-additive, 'set' and 'animate' elements only)"
},

{
"font-family",
"[[ <family-name> | <generic-family> ],]* [ <family-name> | <generic-family>] | inherit",
"depends on user agent",
"text content elements",
true,
"",
"visual",
true
},

{
"font-size",
"<absolute-size> | <relative-size> | <length> | <percentage> | inherit",
"medium",
"text content elements",
"yes, the computed value is inherited",
"refer to parent element's font size",
"visual",
true
},

{
"font-size-adjust",
"<number> | none | inherit",
"none",
"text content elements",
true,
"",
"visual",
true
},

{
"font-stretch",
"normal | wider | narrower | ultra-condensed | extra-condensed | condensed | semi-condensed | semi-expanded | expanded | extra-expanded | ultra-expanded | inherit",
"normal",
"text content elements",
true,
"",
"visual",
true
},

{
"font-style",
"normal | italic | oblique | inherit",
"normal",
"text content elements",
true,
"",
"visual",
true
},

{
"font-variant",
"normal | small-caps | inherit",
"normal",
"text content elements",
true,
"",
"visual",
true
},

{
"font-weight",
"normal | bold | bolder | lighter | 100 | 200 | 300 | 400 | 500 | 600 | 700 | 800 | 900 | inherit",
"normal",
"text content elements",
true,
"",
"visual",
true
},

{
"glyph-orientation-horizontal",
"<angle> | inherit",
"0deg",
"text content elements",
true,
"",
"visual",
false
},

{
"glyph-orientation-vertical",
"auto | <angle> | inherit",
"auto",
"text content elements",
true,
"",
"visual",
false
},

{
"image-rendering",
"auto | optimizeSpeed | optimizeQuality | inherit",
"auto",
"images",
true,
"",
"visual",
true
},

{
"kerning",
"auto | <length> | inherit",
"auto",
"text content elements",
true,
"",
"visual",
true
},

{
"letter-spacing",
"normal | <length> | inherit",
"normal",
"text content elements",
true,
"",
"visual",
true
},

{
"lighting-color",
"currentColor | <color> [icc-color(<name>[,<icccolorvalue>]*)] | inherit",
"white",
"feDiffuseLighting' and 'feSpecularLighting' elements",
false,
"",
"visual",
true
},

{
"marker",
"see individual properties",
"see individual properties",
"path', 'line', 'polyline' and 'polygon' elements",
true,
"",
"visual",
true
},

{
"marker-end' 'marker-mid' 'marker-start",
"none | inherit | <uri>",
"none",
"path', 'line', 'polyline' and 'polygon' elements",
true,
"",
"visual",
true
},

{
"mask",
"<uri> | none | inherit",
"none",
"container elements and graphics elements",
false,
"",
"visual",
true
},

{
"opacity",
"<opacity-value> | inherit",
"1",
"container elements and graphics elements",
false,
"",
"visual",
true
},

{
"overflow",
"visible | hidden | scroll | auto | inherit",
"see prose",
"elements which establish a new viewport, 'pattern' elements and 'marker' elements",
false,
"",
"visual",
true
},

{
"pointer-events",
"visiblePainted | visibleFill | visibleStroke | visible | painted | fill | stroke | all | none | inherit",
"visiblePainted",
"graphics elements",
true,
"",
"visual",
true
},

{
"shape-rendering",
"auto | optimizeSpeed | crispEdges | geometricPrecision | inherit",
"auto",
"shapes",
true,
"",
"visual",
true
},

{
"stop-color",
"currentColor | <color> [icc-color(<name>[,<icccolorvalue>]*)] | inherit",
"black",
"stop' elements",
false,
"",
"visual",
true
},

{
"stop-opacity",
"<opacity-value> | inherit",
"1",
"stop' elements",
false,
"",
"visual",
true
},

{
"stroke",
"<paint> (See Specifying paint)",
"none",
"shapes and text content elements",
true,
"",
"visual",
true
},

{
"stroke-dasharray",
"none | <dasharray> | inherit",
"none",
"shapes and text content elements",
true,
"",
"visual",
""
},

{
"stroke-dashoffset",
"<length> | inherit",
"0",
"shapes and text content elements",
true,
"see prose",
"visual",
true
},

{
"stroke-linecap",
"butt | round | square | inherit",
"butt",
"shapes and text content elements",
true,
"",
"visual",
true
},

{
"stroke-linejoin",
"miter | round | bevel | inherit",
"miter",
"shapes and text content elements",
true,
"",
"visual",
true
},

{
"stroke-miterlimit",
"<miterlimit> | inherit",
"4",
"shapes and text content elements",
true,
"",
"visual",
true
},

{
"stroke-opacity",
"<opacity-value> | inherit",
"1",
"shapes and text content elements",
true,
"",
"visual",
true
},

{
"stroke-width",
"<length> | inherit",
"1",
"shapes and text content elements",
true,
"",
"visual",
true
},

{
"text-anchor",
"start | middle | end | inherit",
"start",
"text content elements",
true,
"",
"visual",
true
},

{
"text-decoration",
"none | [ underline || overline || line-through || blink ] | inherit",
"none",
"text content elements",
"no (see prose)",
"",
"visual",
true
},

{
"text-rendering",
"auto | optimizeSpeed | optimizeLegibility | geometricPrecision | inherit",
"auto",
"'text' elements",
true,
"",
"visual",
true
},

{
"unicode-bidi",
"normal | embed | bidi-override | inherit",
"normal",
"text content elements",
false,
"",
"visual",
false
},

{
"visibility",
"visible | hidden | collapse | inherit",
"visible",
"graphics elements (including the 'text' element) and text sub-elements (i.e., 'tspan', 'tref', 'altGlyph', 'textPath' and 'a')",
true,
"",
"visual",
true
},

{
"word-spacing",
"normal | <length> | inherit",
"normal",
"text content elements",
true,
"",
"visual",
true
},

{
"writing-mode",
"lr-tb | rl-tb | tb-rl | lr | rl | tb | inherit",
"lr-tb",
"'text' elements",
true,
"",
"visual",
false
},

{
NULL,
NULL,
NULL,
NULL,
false,
NULL,
NULL,
false
}

};


static void
printTable()
{
    for (SvgProp const *prop = svgProps; prop->name; prop++) {
        printf("#### Prop: %s ####\n", prop->name);
        printf("values      : %s\n", prop->values);
        printf("defaultValue: %s\n", prop->defaultValue);
        printf("appliesTo   : %s\n", prop->appliesTo);
        printf("inherited   : %s\n", ( prop->inherited ? "true" : "false" ));
        printf("percentages : %s\n", prop->percentages);
        printf("groups      : %s\n", prop->mediaGroups);
        printf("animatable  : %s\n", ( prop->animatable ? "true" : "false" ));
        printf("\n");
    }
}


int main(int /*argc*/, char **/*argv*/)
{
    printTable();
    return ( ferror(stdout) ? EXIT_FAILURE : EXIT_SUCCESS );
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
