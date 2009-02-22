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


struct CssProp_def
{
    const char *name;
    const char *values;
    const char *defaultValue;
    const char *appliesTo;
    bool inherited;
    const char *percentages;
    const char *mediaGroups;
};

typedef struct CssProp_def CssProp;

static CssProp cssProps[] =
{

{
"azimuth",
"<angle> | [[ left-side | far-left | left | center-left | center | center-right | right | far-right | right-side ] || behind ] | leftwards | rightwards | inherit",
"center",
"",
true,
"",
"aural"
},


{
"background-attachment",
"scroll | fixed | inherit",
"scroll",
"",
false,
"",
"visual"
},


{
"background-color",
"<color> | transparent | inherit",
"transparent",
"",
false,
"",
"visual"
},


{
"background-image",
"<uri> | none | inherit",
"none",
"",
false,
"",
"visual"
},


{
"background-position",
"[ [ <percentage> | <length> | left | center | right ] [ <percentage> | <length> | top | center | bottom ]? ] | [ [ left | center | right ] || [ top | center | bottom ] ] | inherit",
"0% 0%",
"",
false,
"refer to the size of the box itself",
"visual"
},


{
"background-repeat",
"repeat | repeat-x | repeat-y | no-repeat | inherit",
"repeat",
"",
false,
"",
"visual"
},


{
"background",
"['background-color' || 'background-image' || 'background-repeat' || 'background-attachment' || 'background-position'] | inherit",
"see individual properties",
"",
false,
"allowed on 'background-position",
"visual"
},


{
"border-collapse",
"collapse | separate | inherit",
"separate",
"table' and 'inline-table' elements",
true,
"",
"visual"
},


{
"border-color",
"[ <color> | transparent ]{1,4} | inherit",
"see individual properties",
"",
false,
"",
"visual"
},


{
"border-spacing",
"<length> <length>? | inherit",
"0",
"table' and 'inline-table' elements",
true,
"",
"visual"
},


{
"border-style",
"<border-style>{1,4} | inherit",
"see individual properties",
"",
false,
"",
"visual"
},


{
"border-top' 'border-right' 'border-bottom' 'border-left",
"[ <border-width> || <border-style> || 'border-top-color' ] | inherit",
"see individual properties",
"",
false,
"",
"visual"
},


{
"border-top-color' 'border-right-color' 'border-bottom-color' 'border-left-color",
"<color> | transparent | inherit",
"the value of the 'color' property",
"",
false,
"",
"visual"
},


{
"border-top-style' 'border-right-style' 'border-bottom-style' 'border-left-style",
"<border-style> | inherit",
"none",
"",
false,
"",
"visual"
},


{
"border-top-width' 'border-right-width' 'border-bottom-width' 'border-left-width",
"<border-width> | inherit",
"medium",
"",
false,
"",
"visual"
},


{
"border-width",
"<border-width>{1,4} | inherit",
"see individual properties",
"",
false,
"",
"visual"
},


{
"border",
"[ <border-width> || <border-style> || 'border-top-color' ] | inherit",
"see individual properties",
"",
false,
"",
"visual"
},


{
"bottom",
"<length> | <percentage> | auto | inherit",
"auto",
"positioned elements",
false,
"refer to height of containing block",
"visual"
},


{
"caption-side",
"top | bottom | inherit",
"top",
"table-caption' elements",
true,
"",
"visual"
},


{
"clear",
"none | left | right | both | inherit",
"none",
"block-level elements",
false,
"",
"visual"
},


{
"clip",
"<shape> | auto | inherit",
"auto",
"absolutely positioned elements",
false,
"",
"visual"
},


{
"color",
"<color> | inherit",
"depends on user agent",
"",
true,
"",
"visual"
},


{
"content",
"normal | [ <string> | <uri> | <counter> | attr(<identifier>) | open-quote | close-quote | no-open-quote | no-close-quote ]+ | inherit",
"normal",
":before and :after pseudo-elements",
false,
"",
"all "
},


{
"counter-increment",
"[ <identifier> <integer>? ]+ | none | inherit",
"none",
"",
false,
"",
"all "
},


{
"counter-reset",
"[ <identifier> <integer>? ]+ | none | inherit",
"none",
"",
false,
"",
"all "
},


{
"cue-after",
"<uri> | none | inherit",
"none",
"",
false,
"",
"aural"
},


{
"cue-before",
"<uri> | none | inherit",
"none",
"",
false,
"",
"aural"
},


{
"cue",
"[ 'cue-before' || 'cue-after' ] | inherit",
"see individual properties",
"",
false,
"",
"aural"
},


{
"cursor",
"[ [<uri> ,]* [ auto | crosshair | default | pointer | move | e-resize | ne-resize | nw-resize | n-resize | se-resize | sw-resize | s-resize | w-resize | text | wait | help | progress ] ] | inherit",
"auto",
"",
true,
"",
"visual, interactive "
},


{
"direction",
"ltr | rtl | inherit",
"ltr",
"all elements, but see prose",
true,
"",
"visual"
},


{
"display",
"inline | block | list-item | run-in | inline-block | table | inline-table | table-row-group | table-header-group | table-footer-group | table-row | table-column-group | table-column | table-cell | table-caption | none | inherit",
"inline",
"",
false,
"",
"all "
},


{
"elevation",
"<angle> | below | level | above | higher | lower | inherit",
"level",
"",
true,
"",
"aural"
},


{
"empty-cells",
"show | hide | inherit",
"show",
"table-cell' elements",
true,
"",
"visual"
},


{
"float",
"left | right | none | inherit",
"none",
"all, but see 9.7",
false,
"",
"visual"
},


{
"font-family",
"[[ <family-name> | <generic-family> ] [, <family-name>| <generic-family>]* ] | inherit",
"depends on user agent",
"",
true,
"",
"visual"
},


{
"font-size",
"<absolute-size> | <relative-size> | <length> | <percentage> | inherit",
"medium",
"",
true,
"refer to parent element's font size",
"visual"
},


{
"font-style",
"normal | italic | oblique | inherit",
"normal",
"",
true,
"",
"visual"
},


{
"font-variant",
"normal | small-caps | inherit",
"normal",
"",
true,
"",
"visual"
},


{
"font-weight",
"normal | bold | bolder | lighter | 100 | 200 | 300 | 400 | 500 | 600 | 700 | 800 | 900 | inherit",
"normal",
"",
true,
"",
"visual"
},


{
"font",
"[ [ 'font-style' || 'font-variant' || 'font-weight' ]? 'font-size' [ / 'line-height' ]? 'font-family' ] | caption | icon | menu | message-box | small-caption | status-bar | inherit",
"see individual properties",
"",
true,
"see individual properties",
"visual"
},


{
"height",
"<length> | <percentage> | auto | inherit",
"auto",
"all elements but non-replaced inline elements, table columns, and column groups",
false,
"see prose",
"visual"
},


{
"left",
"<length> | <percentage> | auto | inherit",
"auto",
"positioned elements",
false,
"refer to width of containing block",
"visual"
},


{
"letter-spacing",
"normal | <length> | inherit",
"normal",
"",
true,
"",
"visual"
},


{
"line-height",
"normal | <number> | <length> | <percentage> | inherit",
"normal",
"",
true,
"refer to the font size of the element itself",
"visual"
},


{
"list-style-image",
"<uri> | none | inherit",
"none",
"elements with 'display: list-item",
true,
"",
"visual"
},


{
"list-style-position",
"inside | outside | inherit",
"outside",
"elements with 'display: list-item",
true,
"",
"visual"
},


{
"list-style-type",
"disc | circle | square | decimal | decimal-leading-zero | lower-roman | upper-roman | lower-greek | lower-latin | upper-latin | armenian | georgian | none | inherit",
"disc",
"elements with 'display: list-item",
true,
"",
"visual"
},


{
"list-style",
"[ 'list-style-type' || 'list-style-position' || 'list-style-image' ] | inherit",
"see individual properties",
"elements with 'display: list-item",
true,
"",
"visual"
},


{
"margin-right' 'margin-left",
"<margin-width> | inherit",
"0",
"all elements except elements with table display types other than table and inline-table",
false,
"refer to width of containing block",
"visual"
},


{
"margin-top' 'margin-bottom",
"<margin-width> | inherit",
"0",
"all elements except elements with table display types other than table and inline-table",
false,
"refer to width of containing block",
"visual"
},


{
"margin",
"<margin-width>{1,4} | inherit",
"see individual properties",
"all elements except elements with table display types other than table and inline-table",
false,
"refer to width of containing block",
"visual"
},


{
"max-height",
"<length> | <percentage> | none | inherit",
"none",
"all elements except non-replaced inline elements and table elements",
false,
"see prose",
"visual"
},


{
"max-width",
"<length> | <percentage> | none | inherit",
"none",
"all elements except non-replaced inline elements and table elements",
false,
"refer to width of containing block",
"visual"
},


{
"min-height",
"<length> | <percentage> | inherit",
"0",
"all elements except non-replaced inline elements and table elements",
false,
"see prose",
"visual"
},


{
"min-width",
"<length> | <percentage> | inherit",
"0",
"all elements except non-replaced inline elements and table elements",
false,
"refer to width of containing block",
"visual"
},


{
"orphans",
"<integer> | inherit",
"2",
"block-level elements",
true,
"",
"visual, paged "
},


{
"outline-color",
"<color> | invert | inherit",
"invert",
"",
false,
"",
"visual, interactive "
},


{
"outline-style",
"<border-style> | inherit",
"none",
"",
false,
"",
"visual, interactive "
},


{
"outline-width",
"<border-width> | inherit",
"medium",
"",
false,
"",
"visual, interactive "
},


{
"outline",
"[ 'outline-color' || 'outline-style' || 'outline-width' ] | inherit",
"see individual properties",
"",
false,
"",
"visual, interactive "
},


{
"overflow",
"visible | hidden | scroll | auto | inherit",
"visible",
"block-level and replaced elements, table cells, inline blocks",
false,
"",
"visual"
},


{
"padding-top' 'padding-right' 'padding-bottom' 'padding-left",
"<padding-width> | inherit",
"0",
"all elements except elements with table display types other than table, inline-table, and table-cell",
false,
"refer to width of containing block",
"visual"
},


{
"padding",
"<padding-width>{1,4} | inherit",
"see individual properties",
"all elements except elements with table display types other than table, inline-table, and table-cell",
false,
"refer to width of containing block",
"visual"
},


{
"page-break-after",
"auto | always | avoid | left | right | inherit",
"auto",
"block-level elements",
false,
"",
"visual, paged "
},


{
"page-break-before",
"auto | always | avoid | left | right | inherit",
"auto",
"block-level elements",
false,
"",
"visual, paged "
},


{
"page-break-inside",
"avoid | auto | inherit",
"auto",
"block-level elements",
true,
"",
"visual, paged "
},


{
"pause-after",
"<time> | <percentage> | inherit",
"0",
"",
false,
"see prose",
"aural"
},


{
"pause-before",
"<time> | <percentage> | inherit",
"0",
"",
false,
"see prose",
"aural"
},


{
"pause",
"[ [<time> | <percentage>]{1,2} ] | inherit",
"see individual properties",
"",
false,
"see descriptions of 'pause-before' and 'pause-after",
"aural"
},


{
"pitch-range",
"<number> | inherit",
"50",
"",
true,
"",
"aural"
},


{
"pitch",
"<frequency> | x-low | low | medium | high | x-high | inherit",
"medium",
"",
true,
"",
"aural"
},


{
"play-during",
"<uri> [ mix || repeat ]? | auto | none | inherit",
"auto",
"",
false,
"",
"aural"
},


{
"position",
"static | relative | absolute | fixed | inherit",
"static",
"",
false,
"",
"visual"
},


{
"quotes",
"[<string> <string>]+ | none | inherit",
"depends on user agent",
"",
true,
"",
"visual"
},


{
"richness",
"<number> | inherit",
"50",
"",
true,
"",
"aural"
},


{
"right",
"<length> | <percentage> | auto | inherit",
"auto",
"positioned elements",
false,
"refer to width of containing block",
"visual"
},


{
"speak-header",
"once | always | inherit",
"once",
"elements that have table header information",
true,
"",
"aural"
},


{
"speak-numeral",
"digits | continuous | inherit",
"continuous",
"",
true,
"",
"aural"
},


{
"speak-punctuation",
"code | none | inherit",
"none",
"",
true,
"",
"aural"
},


{
"speak",
"normal | none | spell-out | inherit",
"normal",
"",
true,
"",
"aural"
},


{
"speech-rate",
"<number> | x-slow | slow | medium | fast | x-fast | faster | slower | inherit",
"medium",
"",
true,
"",
"aural"
},


{
"stress",
"<number> | inherit",
"50",
"",
true,
"",
"aural"
},


{
"table-layout",
"auto | fixed | inherit",
"auto",
"table' and 'inline-table' elements",
false,
"",
"visual"
},


{
"text-align",
"left | right | center | justify | inherit",
"left' if 'direction' is 'ltr'; 'right' if 'direction' is 'rtl",
"block-level elements, table cells and inline blocks",
true,
"",
"visual"
},


{
"text-decoration",
"none | [ underline || overline || line-through || blink ] | inherit",
"none",
"",
"no (see prose)",
"",
"visual"
},


{
"text-indent",
"<length> | <percentage> | inherit",
"0",
"block-level elements, table cells and inline blocks",
true,
"refer to width of containing block",
"visual"
},


{
"text-transform",
"capitalize | uppercase | lowercase | none | inherit",
"none",
"",
true,
"",
"visual"
},


{
"top",
"<length> | <percentage> | auto | inherit",
"auto",
"positioned elements",
false,
"refer to height of containing block",
"visual"
},


{
"unicode-bidi",
"normal | embed | bidi-override | inherit",
"normal",
"all elements, but see prose",
false,
"",
"visual"
},


{
"vertical-align",
"baseline | sub | super | top | text-top | middle | bottom | text-bottom | <percentage> | <length> | inherit",
"baseline",
"inline-level and 'table-cell' elements",
false,
"refer to the 'line-height' of the element itself",
"visual"
},


{
"visibility",
"visible | hidden | collapse | inherit",
"visible",
"",
true,
"",
"visual"
},


{
"voice-family",
"[[<specific-voice> | <generic-voice> ],]* [<specific-voice> | <generic-voice> ] | inherit",
"depends on user agent",
"",
true,
"",
"aural"
},


{
"volume",
"<number> | <percentage> | silent | x-soft | soft | medium | loud | x-loud | inherit",
"medium",
"",
true,
"refer to inherited value",
"aural"
},


{
"white-space",
"normal | pre | nowrap | pre-wrap | pre-line | inherit",
"normal",
"",
true,
"",
"visual"
},


{
"widows",
"<integer> | inherit",
"2",
"block-level elements",
true,
"",
"visual, paged"
},


{
"width",
"<length> | <percentage> | auto | inherit",
"auto",
"all elements but non-replaced inline elements, table rows, and row groups",
false,
"refer to width of containing block",
"visual"
},


{
"word-spacing",
"normal | <length> | inherit",
"normal",
"",
true,
"",
"visual"
},


{
"z-index",
"auto | <integer> | inherit",
"auto",
"positioned elements",
false,
"",
"visual"
},

{
NULL,
NULL,
NULL,
NULL,
false,
NULL,
NULL
}

};




bool printTable()
{
    for (CssProp *prop=cssProps; prop->name ; prop++)
        {
        printf("#### Prop: %s ####\n", prop->name);
        printf("values      : %s\n", prop->values);
        printf("defaultValue: %s\n", prop->defaultValue);
        printf("appliesTo   : %s\n", prop->appliesTo);
        printf("inherited   : %s\n", ( prop->inherited ? "true" : "false" ));
        printf("percentages : %s\n", prop->percentages);
        printf("groups      : %s\n", prop->mediaGroups);
        printf("\n");
        }
    return true;
}


int main(int /*argc*/, char **/*argv*/)
{
    printTable();
    return 0;
}
