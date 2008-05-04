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
#include <stdio.h>
#include <string.h>

typedef struct CssProp_def CssProp;

typedef bool (*parsefunc)(CssProp *prop, char *propName, char *propVal);


struct CssProp_def
{
    parsefunc func;
    const char *name;
    const char *values;
    const char *defaultValue;
    const char *appliesTo;
    bool inherited;
    const char *percentages;
    const char *mediaGroups;
};


bool parseDefault(CssProp */*prop*/, char *propName, char *propVal)
{
    printf("######## '%s:%s'\n", propName, propVal);
    return true;
}



static CssProp cssProps[] =
{

{
parseDefault,
"azimuth",
"<angle> | [[ left-side | far-left | left | center-left | center | center-right | right | far-right | right-side ] || behind ] | leftwards | rightwards | inherit",
"center",
"",
true,
"",
"aural"
},


{
parseDefault,
"background-attachment",
"scroll | fixed | inherit",
"scroll",
"",
false,
"",
"visual"
},


{
parseDefault,
"background-color",
"<color> | transparent | inherit",
"transparent",
"",
false,
"",
"visual"
},


{
parseDefault,
"background-image",
"<uri> | none | inherit",
"none",
"",
false,
"",
"visual"
},


{
parseDefault,
"background-position",
"[ [ <percentage> | <length> | left | center | right ] [ <percentage> | <length> | top | center | bottom ]? ] | [ [ left | center | right ] || [ top | center | bottom ] ] | inherit",
"0% 0%",
"",
false,
"refer to the size of the box itself",
"visual"
},


{
parseDefault,
"background-repeat",
"repeat | repeat-x | repeat-y | no-repeat | inherit",
"repeat",
"",
false,
"",
"visual"
},


{
parseDefault,
"background",
"['background-color' || 'background-image' || 'background-repeat' || 'background-attachment' || 'background-position'] | inherit",
"see individual properties",
"",
false,
"allowed on 'background-position",
"visual"
},


{
parseDefault,
"border-collapse",
"collapse | separate | inherit",
"separate",
"table' and 'inline-table' elements",
true,
"",
"visual"
},


{
parseDefault,
"border-color",
"[ <color> | transparent ]{1,4} | inherit",
"see individual properties",
"",
false,
"",
"visual"
},


{
parseDefault,
"border-spacing",
"<length> <length>? | inherit",
"0",
"table' and 'inline-table' elements",
true,
"",
"visual"
},


{
parseDefault,
"border-style",
"<border-style>{1,4} | inherit",
"see individual properties",
"",
false,
"",
"visual"
},


{
parseDefault,
"border-top' 'border-right' 'border-bottom' 'border-left",
"[ <border-width> || <border-style> || 'border-top-color' ] | inherit",
"see individual properties",
"",
false,
"",
"visual"
},


{
parseDefault,
"border-top-color' 'border-right-color' 'border-bottom-color' 'border-left-color",
"<color> | transparent | inherit",
"the value of the 'color' property",
"",
false,
"",
"visual"
},


{
parseDefault,
"border-top-style' 'border-right-style' 'border-bottom-style' 'border-left-style",
"<border-style> | inherit",
"none",
"",
false,
"",
"visual"
},


{
parseDefault,
"border-top-width' 'border-right-width' 'border-bottom-width' 'border-left-width",
"<border-width> | inherit",
"medium",
"",
false,
"",
"visual"
},


{
parseDefault,
"border-width",
"<border-width>{1,4} | inherit",
"see individual properties",
"",
false,
"",
"visual"
},


{
parseDefault,
"border",
"[ <border-width> || <border-style> || 'border-top-color' ] | inherit",
"see individual properties",
"",
false,
"",
"visual"
},


{
parseDefault,
"bottom",
"<length> | <percentage> | auto | inherit",
"auto",
"positioned elements",
false,
"refer to height of containing block",
"visual"
},


{
parseDefault,
"caption-side",
"top | bottom | inherit",
"top",
"table-caption' elements",
true,
"",
"visual"
},


{
parseDefault,
"clear",
"none | left | right | both | inherit",
"none",
"block-level elements",
false,
"",
"visual"
},


{
parseDefault,
"clip",
"<shape> | auto | inherit",
"auto",
"absolutely positioned elements",
false,
"",
"visual"
},


{
parseDefault,
"color",
"<color> | inherit",
"depends on user agent",
"",
true,
"",
"visual"
},


{
parseDefault,
"content",
"normal | [ <string> | <uri> | <counter> | attr(<identifier>) | open-quote | close-quote | no-open-quote | no-close-quote ]+ | inherit",
"normal",
":before and :after pseudo-elements",
false,
"",
"all "
},


{
parseDefault,
"counter-increment",
"[ <identifier> <integer>? ]+ | none | inherit",
"none",
"",
false,
"",
"all "
},


{
parseDefault,
"counter-reset",
"[ <identifier> <integer>? ]+ | none | inherit",
"none",
"",
false,
"",
"all "
},


{
parseDefault,
"cue-after",
"<uri> | none | inherit",
"none",
"",
false,
"",
"aural"
},


{
parseDefault,
"cue-before",
"<uri> | none | inherit",
"none",
"",
false,
"",
"aural"
},


{
parseDefault,
"cue",
"[ 'cue-before' || 'cue-after' ] | inherit",
"see individual properties",
"",
false,
"",
"aural"
},


{
parseDefault,
"cursor",
"[ [<uri> ,]* [ auto | crosshair | default | pointer | move | e-resize | ne-resize | nw-resize | n-resize | se-resize | sw-resize | s-resize | w-resize | text | wait | help | progress ] ] | inherit",
"auto",
"",
true,
"",
"visual, interactive "
},


{
parseDefault,
"direction",
"ltr | rtl | inherit",
"ltr",
"all elements, but see prose",
true,
"",
"visual"
},


{
parseDefault,
"display",
"inline | block | list-item | run-in | inline-block | table | inline-table | table-row-group | table-header-group | table-footer-group | table-row | table-column-group | table-column | table-cell | table-caption | none | inherit",
"inline",
"",
false,
"",
"all "
},


{
parseDefault,
"elevation",
"<angle> | below | level | above | higher | lower | inherit",
"level",
"",
true,
"",
"aural"
},


{
parseDefault,
"empty-cells",
"show | hide | inherit",
"show",
"table-cell' elements",
true,
"",
"visual"
},


{
parseDefault,
"float",
"left | right | none | inherit",
"none",
"all, but see 9.7",
false,
"",
"visual"
},


{
parseDefault,
"font-family",
"[[ <family-name> | <generic-family> ] [, <family-name>| <generic-family>]* ] | inherit",
"depends on user agent",
"",
true,
"",
"visual"
},


{
parseDefault,
"font-size",
"<absolute-size> | <relative-size> | <length> | <percentage> | inherit",
"medium",
"",
true,
"refer to parent element's font size",
"visual"
},


{
parseDefault,
"font-style",
"normal | italic | oblique | inherit",
"normal",
"",
true,
"",
"visual"
},


{
parseDefault,
"font-variant",
"normal | small-caps | inherit",
"normal",
"",
true,
"",
"visual"
},


{
parseDefault,
"font-weight",
"normal | bold | bolder | lighter | 100 | 200 | 300 | 400 | 500 | 600 | 700 | 800 | 900 | inherit",
"normal",
"",
true,
"",
"visual"
},


{
parseDefault,
"font",
"[ [ 'font-style' || 'font-variant' || 'font-weight' ]? 'font-size' [ / 'line-height' ]? 'font-family' ] | caption | icon | menu | message-box | small-caption | status-bar | inherit",
"see individual properties",
"",
true,
"see individual properties",
"visual"
},


{
parseDefault,
"height",
"<length> | <percentage> | auto | inherit",
"auto",
"all elements but non-replaced inline elements, table columns, and column groups",
false,
"see prose",
"visual"
},


{
parseDefault,
"left",
"<length> | <percentage> | auto | inherit",
"auto",
"positioned elements",
false,
"refer to width of containing block",
"visual"
},


{
parseDefault,
"letter-spacing",
"normal | <length> | inherit",
"normal",
"",
true,
"",
"visual"
},


{
parseDefault,
"line-height",
"normal | <number> | <length> | <percentage> | inherit",
"normal",
"",
true,
"refer to the font size of the element itself",
"visual"
},


{
parseDefault,
"list-style-image",
"<uri> | none | inherit",
"none",
"elements with 'display: list-item",
true,
"",
"visual"
},


{
parseDefault,
"list-style-position",
"inside | outside | inherit",
"outside",
"elements with 'display: list-item",
true,
"",
"visual"
},


{
parseDefault,
"list-style-type",
"disc | circle | square | decimal | decimal-leading-zero | lower-roman | upper-roman | lower-greek | lower-latin | upper-latin | armenian | georgian | none | inherit",
"disc",
"elements with 'display: list-item",
true,
"",
"visual"
},


{
parseDefault,
"list-style",
"[ 'list-style-type' || 'list-style-position' || 'list-style-image' ] | inherit",
"see individual properties",
"elements with 'display: list-item",
true,
"",
"visual"
},


{
parseDefault,
"margin-right' 'margin-left",
"<margin-width> | inherit",
"0",
"all elements except elements with table display types other than table and inline-table",
false,
"refer to width of containing block",
"visual"
},


{
parseDefault,
"margin-top' 'margin-bottom",
"<margin-width> | inherit",
"0",
"all elements except elements with table display types other than table and inline-table",
false,
"refer to width of containing block",
"visual"
},


{
parseDefault,
"margin",
"<margin-width>{1,4} | inherit",
"see individual properties",
"all elements except elements with table display types other than table and inline-table",
false,
"refer to width of containing block",
"visual"
},


{
parseDefault,
"max-height",
"<length> | <percentage> | none | inherit",
"none",
"all elements except non-replaced inline elements and table elements",
false,
"see prose",
"visual"
},


{
parseDefault,
"max-width",
"<length> | <percentage> | none | inherit",
"none",
"all elements except non-replaced inline elements and table elements",
false,
"refer to width of containing block",
"visual"
},


{
parseDefault,
"min-height",
"<length> | <percentage> | inherit",
"0",
"all elements except non-replaced inline elements and table elements",
false,
"see prose",
"visual"
},


{
parseDefault,
"min-width",
"<length> | <percentage> | inherit",
"0",
"all elements except non-replaced inline elements and table elements",
false,
"refer to width of containing block",
"visual"
},


{
parseDefault,
"orphans",
"<integer> | inherit",
"2",
"block-level elements",
true,
"",
"visual, paged "
},


{
parseDefault,
"outline-color",
"<color> | invert | inherit",
"invert",
"",
false,
"",
"visual, interactive "
},


{
parseDefault,
"outline-style",
"<border-style> | inherit",
"none",
"",
false,
"",
"visual, interactive "
},


{
parseDefault,
"outline-width",
"<border-width> | inherit",
"medium",
"",
false,
"",
"visual, interactive "
},


{
parseDefault,
"outline",
"[ 'outline-color' || 'outline-style' || 'outline-width' ] | inherit",
"see individual properties",
"",
false,
"",
"visual, interactive "
},


{
parseDefault,
"overflow",
"visible | hidden | scroll | auto | inherit",
"visible",
"block-level and replaced elements, table cells, inline blocks",
false,
"",
"visual"
},


{
parseDefault,
"padding-top' 'padding-right' 'padding-bottom' 'padding-left",
"<padding-width> | inherit",
"0",
"all elements except elements with table display types other than table, inline-table, and table-cell",
false,
"refer to width of containing block",
"visual"
},


{
parseDefault,
"padding",
"<padding-width>{1,4} | inherit",
"see individual properties",
"all elements except elements with table display types other than table, inline-table, and table-cell",
false,
"refer to width of containing block",
"visual"
},


{
parseDefault,
"page-break-after",
"auto | always | avoid | left | right | inherit",
"auto",
"block-level elements",
false,
"",
"visual, paged "
},


{
parseDefault,
"page-break-before",
"auto | always | avoid | left | right | inherit",
"auto",
"block-level elements",
false,
"",
"visual, paged "
},


{
parseDefault,
"page-break-inside",
"avoid | auto | inherit",
"auto",
"block-level elements",
true,
"",
"visual, paged "
},


{
parseDefault,
"pause-after",
"<time> | <percentage> | inherit",
"0",
"",
false,
"see prose",
"aural"
},


{
parseDefault,
"pause-before",
"<time> | <percentage> | inherit",
"0",
"",
false,
"see prose",
"aural"
},


{
parseDefault,
"pause",
"[ [<time> | <percentage>]{1,2} ] | inherit",
"see individual properties",
"",
false,
"see descriptions of 'pause-before' and 'pause-after",
"aural"
},


{
parseDefault,
"pitch-range",
"<number> | inherit",
"50",
"",
true,
"",
"aural"
},


{
parseDefault,
"pitch",
"<frequency> | x-low | low | medium | high | x-high | inherit",
"medium",
"",
true,
"",
"aural"
},


{
parseDefault,
"play-during",
"<uri> [ mix || repeat ]? | auto | none | inherit",
"auto",
"",
false,
"",
"aural"
},


{
parseDefault,
"position",
"static | relative | absolute | fixed | inherit",
"static",
"",
false,
"",
"visual"
},


{
parseDefault,
"quotes",
"[<string> <string>]+ | none | inherit",
"depends on user agent",
"",
true,
"",
"visual"
},


{
parseDefault,
"richness",
"<number> | inherit",
"50",
"",
true,
"",
"aural"
},


{
parseDefault,
"right",
"<length> | <percentage> | auto | inherit",
"auto",
"positioned elements",
false,
"refer to width of containing block",
"visual"
},


{
parseDefault,
"speak-header",
"once | always | inherit",
"once",
"elements that have table header information",
true,
"",
"aural"
},


{
parseDefault,
"speak-numeral",
"digits | continuous | inherit",
"continuous",
"",
true,
"",
"aural"
},


{
parseDefault,
"speak-punctuation",
"code | none | inherit",
"none",
"",
true,
"",
"aural"
},


{
parseDefault,
"speak",
"normal | none | spell-out | inherit",
"normal",
"",
true,
"",
"aural"
},


{
parseDefault,
"speech-rate",
"<number> | x-slow | slow | medium | fast | x-fast | faster | slower | inherit",
"medium",
"",
true,
"",
"aural"
},


{
parseDefault,
"stress",
"<number> | inherit",
"50",
"",
true,
"",
"aural"
},


{
parseDefault,
"table-layout",
"auto | fixed | inherit",
"auto",
"table' and 'inline-table' elements",
false,
"",
"visual"
},


{
parseDefault,
"text-align",
"left | right | center | justify | inherit",
"left' if 'direction' is 'ltr'; 'right' if 'direction' is 'rtl",
"block-level elements, table cells and inline blocks",
true,
"",
"visual"
},


{
parseDefault,
"text-decoration",
"none | [ underline || overline || line-through || blink ] | inherit",
"none",
"",
"no (see prose)",
"",
"visual"
},


{
parseDefault,
"text-indent",
"<length> | <percentage> | inherit",
"0",
"block-level elements, table cells and inline blocks",
true,
"refer to width of containing block",
"visual"
},


{
parseDefault,
"text-transform",
"capitalize | uppercase | lowercase | none | inherit",
"none",
"",
true,
"",
"visual"
},


{
parseDefault,
"top",
"<length> | <percentage> | auto | inherit",
"auto",
"positioned elements",
false,
"refer to height of containing block",
"visual"
},


{
parseDefault,
"unicode-bidi",
"normal | embed | bidi-override | inherit",
"normal",
"all elements, but see prose",
false,
"",
"visual"
},


{
parseDefault,
"vertical-align",
"baseline | sub | super | top | text-top | middle | bottom | text-bottom | <percentage> | <length> | inherit",
"baseline",
"inline-level and 'table-cell' elements",
false,
"refer to the 'line-height' of the element itself",
"visual"
},


{
parseDefault,
"visibility",
"visible | hidden | collapse | inherit",
"visible",
"",
true,
"",
"visual"
},


{
parseDefault,
"voice-family",
"[[<specific-voice> | <generic-voice> ],]* [<specific-voice> | <generic-voice> ] | inherit",
"depends on user agent",
"",
true,
"",
"aural"
},


{
parseDefault,
"volume",
"<number> | <percentage> | silent | x-soft | soft | medium | loud | x-loud | inherit",
"medium",
"",
true,
"refer to inherited value",
"aural"
},


{
parseDefault,
"white-space",
"normal | pre | nowrap | pre-wrap | pre-line | inherit",
"normal",
"",
true,
"",
"visual"
},


{
parseDefault,
"widows",
"<integer> | inherit",
"2",
"block-level elements",
true,
"",
"visual, paged"
},


{
parseDefault,
"width",
"<length> | <percentage> | auto | inherit",
"auto",
"all elements but non-replaced inline elements, table rows, and row groups",
false,
"refer to width of containing block",
"visual"
},


{
parseDefault,
"word-spacing",
"normal | <length> | inherit",
"normal",
"",
true,
"",
"visual"
},


{
parseDefault,
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
NULL,
false,
NULL,
NULL
}

};



bool parseProperty(char *name, char *value)
{
    for (CssProp *prop=cssProps; prop->name ; prop++)
        {
        if (strcmp(name, prop->name)==0)
            {
            parsefunc func = prop->func;
            if (func)
                {
                if (!(*func)(prop, name, value))
                    {
                    printf("...\n");
                    return false;
                    }
                else
                    {
                    return true;
                    }
                }
            else
                {
                printf("null parsing function specified\n");
                return false;
                }
            }
        }
    printf("Property '%s' not found\n",  name);
    return false;
}


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
    //printTable();
    parseProperty((char *)"visibility", (char *)"hidden");
    return 0;
}
