#ifndef SEEN_SP_STYLE_ENUMS_H
#define SEEN_SP_STYLE_ENUMS_H

/** \file
 * SPStyle enums: named public enums that correspond to SVG property values.
 */
/* Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2010 Jon A. Cruz
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/* SPFontStyle */

#include "display/canvas-bpath.h" // FIXME those enums belong here!

enum SPCSSFontSize {
    SP_CSS_FONT_SIZE_XX_SMALL,
    SP_CSS_FONT_SIZE_X_SMALL,
    SP_CSS_FONT_SIZE_SMALL,
    SP_CSS_FONT_SIZE_MEDIUM,
    SP_CSS_FONT_SIZE_LARGE,
    SP_CSS_FONT_SIZE_X_LARGE,
    SP_CSS_FONT_SIZE_XX_LARGE,
    SP_CSS_FONT_SIZE_SMALLER,
    SP_CSS_FONT_SIZE_LARGER
};

enum SPCSSFontStyle {
    SP_CSS_FONT_STYLE_NORMAL,
    SP_CSS_FONT_STYLE_ITALIC,
    SP_CSS_FONT_STYLE_OBLIQUE
};

enum SPCSSFontVariant {
    SP_CSS_FONT_VARIANT_NORMAL,
    SP_CSS_FONT_VARIANT_SMALL_CAPS
};

enum SPCSSFontWeight {
    SP_CSS_FONT_WEIGHT_100,
    SP_CSS_FONT_WEIGHT_200,
    SP_CSS_FONT_WEIGHT_300,
    SP_CSS_FONT_WEIGHT_400,
    SP_CSS_FONT_WEIGHT_500,
    SP_CSS_FONT_WEIGHT_600,
    SP_CSS_FONT_WEIGHT_700,
    SP_CSS_FONT_WEIGHT_800,
    SP_CSS_FONT_WEIGHT_900,
    SP_CSS_FONT_WEIGHT_NORMAL,
    SP_CSS_FONT_WEIGHT_BOLD,
    SP_CSS_FONT_WEIGHT_LIGHTER,
    SP_CSS_FONT_WEIGHT_BOLDER
};

enum SPCSSFontStretch {
    SP_CSS_FONT_STRETCH_ULTRA_CONDENSED,
    SP_CSS_FONT_STRETCH_EXTRA_CONDENSED,
    SP_CSS_FONT_STRETCH_CONDENSED,
    SP_CSS_FONT_STRETCH_SEMI_CONDENSED,
    SP_CSS_FONT_STRETCH_NORMAL,
    SP_CSS_FONT_STRETCH_SEMI_EXPANDED,
    SP_CSS_FONT_STRETCH_EXPANDED,
    SP_CSS_FONT_STRETCH_EXTRA_EXPANDED,
    SP_CSS_FONT_STRETCH_ULTRA_EXPANDED,
    SP_CSS_FONT_STRETCH_NARROWER,
    SP_CSS_FONT_STRETCH_WIDER
};

// Can select more than one
enum SPCSSFontVariantLigatures {
    SP_CSS_FONT_VARIANT_LIGATURES_NONE            = 0,
    SP_CSS_FONT_VARIANT_LIGATURES_COMMON          = 1,
    SP_CSS_FONT_VARIANT_LIGATURES_DISCRETIONARY   = 2,
    SP_CSS_FONT_VARIANT_LIGATURES_HISTORICAL      = 4,
    SP_CSS_FONT_VARIANT_LIGATURES_CONTEXTUAL      = 8,
    SP_CSS_FONT_VARIANT_LIGATURES_NOCOMMON        = 16,
    SP_CSS_FONT_VARIANT_LIGATURES_NODISCRETIONARY = 32,
    SP_CSS_FONT_VARIANT_LIGATURES_NOHISTORICAL    = 64,
    SP_CSS_FONT_VARIANT_LIGATURES_NOCONTEXTUAL    = 128
};

enum SPCSSFontVariantPosition {
    SP_CSS_FONT_VARIANT_POSITION_NORMAL,
    SP_CSS_FONT_VARIANT_POSITION_SUB,
    SP_CSS_FONT_VARIANT_POSITION_SUPER
};

enum SPCSSFontVariantCaps {
    SP_CSS_FONT_VARIANT_CAPS_NORMAL,
    SP_CSS_FONT_VARIANT_CAPS_SMALL,
    SP_CSS_FONT_VARIANT_CAPS_ALL_SMALL,
    SP_CSS_FONT_VARIANT_CAPS_PETITE,
    SP_CSS_FONT_VARIANT_CAPS_ALL_PETITE,
    SP_CSS_FONT_VARIANT_CAPS_UNICASE,
    SP_CSS_FONT_VARIANT_CAPS_TITLING,
};

// Can select more than one (see spec)
enum SPCSSFontVariantNumeric {
    SP_CSS_FONT_VARIANT_NUMERIC_NORMAL,
    SP_CSS_FONT_VARIANT_NUMERIC_LINING_NUMS,
    SP_CSS_FONT_VARIANT_NUMERIC_OLDSTYLE_NUMS,
    SP_CSS_FONT_VARIANT_NUMERIC_PROPORTIONAL_NUMS,
    SP_CSS_FONT_VARIANT_NUMERIC_TABULAR_NUMS,
    SP_CSS_FONT_VARIANT_NUMERIC_DIAGONAL_FRACTIONS,
    SP_CSS_FONT_VARIANT_NUMERIC_STACKED_FRACTIONS,
    SP_CSS_FONT_VARIANT_NUMERIC_ORDINAL,
    SP_CSS_FONT_VARIANT_NUMERIC_SLASHED_ZERO
};

enum SPCSSFontVariantAlternates {
    SP_CSS_FONT_VARIANT_ALTERNATES_NORMAL,
    SP_CSS_FONT_VARIANT_ALTERNATES_HISTORICAL_FORMS,
    SP_CSS_FONT_VARIANT_ALTERNATES_STYLISTIC,
    SP_CSS_FONT_VARIANT_ALTERNATES_STYLESET,
    SP_CSS_FONT_VARIANT_ALTERNATES_CHARACTER_VARIANT,
    SP_CSS_FONT_VARIANT_ALTERNATES_SWASH,
    SP_CSS_FONT_VARIANT_ALTERNATES_ORNAMENTS,
    SP_CSS_FONT_VARIANT_ALTERNATES_ANNOTATION
};

enum SPCSSFontVariantEastAsian {
    SP_CSS_FONT_VARIANT_EAST_ASIAN_NORMAL,
    SP_CSS_FONT_VARIANT_EAST_ASIAN_JIS78,
    SP_CSS_FONT_VARIANT_EAST_ASIAN_JIS83,
    SP_CSS_FONT_VARIANT_EAST_ASIAN_JIS90,
    SP_CSS_FONT_VARIANT_EAST_ASIAN_JIS04,
    SP_CSS_FONT_VARIANT_EAST_ASIAN_SIMPLIFIED,
    SP_CSS_FONT_VARIANT_EAST_ASIAN_TRADITIONAL,
    SP_CSS_FONT_VARIANT_EAST_ASIAN_FULL_WIDTH,
    SP_CSS_FONT_VARIANT_EAST_ASIAN_PROPORTIONAL_WIDTH,
    SP_CSS_FONT_VARIANT_EAST_ASIAN_RUBY
};

enum SPCSSTextAlign {
    SP_CSS_TEXT_ALIGN_START,
    SP_CSS_TEXT_ALIGN_END,
    SP_CSS_TEXT_ALIGN_LEFT,
    SP_CSS_TEXT_ALIGN_RIGHT,
    SP_CSS_TEXT_ALIGN_CENTER,
    SP_CSS_TEXT_ALIGN_JUSTIFY
    // also <string> is allowed, but only within table calls
};

enum SPCSSTextTransform {
    SP_CSS_TEXT_TRANSFORM_CAPITALIZE,
    SP_CSS_TEXT_TRANSFORM_UPPERCASE,
    SP_CSS_TEXT_TRANSFORM_LOWERCASE,
    SP_CSS_TEXT_TRANSFORM_NONE
};

enum SPCSSDirection {
    SP_CSS_DIRECTION_LTR,
    SP_CSS_DIRECTION_RTL
};

enum SPCSSBlockProgression {
    SP_CSS_BLOCK_PROGRESSION_TB,
    SP_CSS_BLOCK_PROGRESSION_RL,
    SP_CSS_BLOCK_PROGRESSION_LR
};

enum SPCSSWritingMode {
    SP_CSS_WRITING_MODE_LR_TB,
    SP_CSS_WRITING_MODE_RL_TB,
    SP_CSS_WRITING_MODE_TB_RL,
    SP_CSS_WRITING_MODE_TB_LR
};

enum SPTextAnchor {
    SP_CSS_TEXT_ANCHOR_START,
    SP_CSS_TEXT_ANCHOR_MIDDLE,
    SP_CSS_TEXT_ANCHOR_END
};

enum SPWhiteSpace {
    SP_CSS_WHITE_SPACE_NORMAL,
    SP_CSS_WHITE_SPACE_PRE,
    SP_CSS_WHITE_SPACE_NOWRAP,
    SP_CSS_WHITE_SPACE_PREWRAP,
    SP_CSS_WHITE_SPACE_PRELINE
};

enum SPCSSBaselineShift {
  SP_CSS_BASELINE_SHIFT_BASELINE,
  SP_CSS_BASELINE_SHIFT_SUB,
  SP_CSS_BASELINE_SHIFT_SUPER
};

enum SPVisibility {
    SP_CSS_VISIBILITY_HIDDEN,
    SP_CSS_VISIBILITY_COLLAPSE,
    SP_CSS_VISIBILITY_VISIBLE
};

enum SPOverflow {
    SP_CSS_OVERFLOW_VISIBLE,
    SP_CSS_OVERFLOW_HIDDEN,
    SP_CSS_OVERFLOW_SCROLL,
    SP_CSS_OVERFLOW_AUTO
};

/// \todo more display types
enum SPCSSDisplay {
    SP_CSS_DISPLAY_NONE,
    SP_CSS_DISPLAY_INLINE,
    SP_CSS_DISPLAY_BLOCK,
    SP_CSS_DISPLAY_LIST_ITEM,
    SP_CSS_DISPLAY_RUN_IN,
    SP_CSS_DISPLAY_COMPACT,
    SP_CSS_DISPLAY_MARKER,
    SP_CSS_DISPLAY_TABLE,
    SP_CSS_DISPLAY_INLINE_TABLE,
    SP_CSS_DISPLAY_TABLE_ROW_GROUP,
    SP_CSS_DISPLAY_TABLE_HEADER_GROUP,
    SP_CSS_DISPLAY_TABLE_FOOTER_GROUP,
    SP_CSS_DISPLAY_TABLE_ROW,
    SP_CSS_DISPLAY_TABLE_COLUMN_GROUP,
    SP_CSS_DISPLAY_TABLE_COLUMN,
    SP_CSS_DISPLAY_TABLE_CELL,
    SP_CSS_DISPLAY_TABLE_CAPTION
};

enum SPIsolation {
    SP_CSS_ISOLATION_AUTO,
    SP_CSS_ISOLATION_ISOLATE
};

enum SPBlendMode {
    SP_CSS_BLEND_NORMAL,
    SP_CSS_BLEND_MULTIPLY,
    SP_CSS_BLEND_SCREEN,
    SP_CSS_BLEND_DARKEN,
    SP_CSS_BLEND_LIGHTEN,
    SP_CSS_BLEND_OVERLAY,
    SP_CSS_BLEND_COLORDODGE,
    SP_CSS_BLEND_COLORBURN,
    SP_CSS_BLEND_HARDLIGHT,
    SP_CSS_BLEND_SOFTLIGHT,
    SP_CSS_BLEND_DIFFERENCE,
    SP_CSS_BLEND_EXCLUSION,
    SP_CSS_BLEND_HUE,
    SP_CSS_BLEND_SATURATION,
    SP_CSS_BLEND_COLOR,
    SP_CSS_BLEND_LUMINOSITY
};

enum SPEnableBackground {
    SP_CSS_BACKGROUND_ACCUMULATE,
    SP_CSS_BACKGROUND_NEW
};

enum SPColorInterpolation {
    SP_CSS_COLOR_INTERPOLATION_AUTO,
    SP_CSS_COLOR_INTERPOLATION_SRGB,
    SP_CSS_COLOR_INTERPOLATION_LINEARRGB
};

enum SPColorRendering {
    SP_CSS_COLOR_RENDERING_AUTO,
    SP_CSS_COLOR_RENDERING_OPTIMIZESPEED,
    SP_CSS_COLOR_RENDERING_OPTIMIZEQUALITY
};

/* Last two are CSS4 Image values... for the momement prefaced with -inkscape. */
enum SPImageRendering {
    SP_CSS_IMAGE_RENDERING_AUTO,
    SP_CSS_IMAGE_RENDERING_OPTIMIZESPEED,
    SP_CSS_IMAGE_RENDERING_OPTIMIZEQUALITY,
    SP_CSS_IMAGE_RENDERING_CRISPEDGES,
    SP_CSS_IMAGE_RENDERING_PIXELATED
};

enum SPShapeRendering {
    SP_CSS_SHAPE_RENDERING_AUTO,
    SP_CSS_SHAPE_RENDERING_OPTIMIZESPEED,
    SP_CSS_SHAPE_RENDERING_CRISPEDGES,
    SP_CSS_SHAPE_RENDERING_GEOMETRICPRECISION
};

enum SPTextRendering {
    SP_CSS_TEXT_RENDERING_AUTO,
    SP_CSS_TEXT_RENDERING_OPTIMIZESPEED,
    SP_CSS_TEXT_RENDERING_OPTIMIZELEGIBILITY,
    SP_CSS_TEXT_RENDERING_GEOMETRICPRECISION
};


struct SPStyleEnum {
    char const *key;
    int value;
};

static SPStyleEnum const enum_fill_rule[] = {
    {"nonzero", SP_WIND_RULE_NONZERO},
    {"evenodd", SP_WIND_RULE_EVENODD},
    {NULL, -1}
};

static SPStyleEnum const enum_stroke_linecap[] = {
    {"butt", SP_STROKE_LINECAP_BUTT},
    {"round", SP_STROKE_LINECAP_ROUND},
    {"square", SP_STROKE_LINECAP_SQUARE},
    {NULL, -1}
};

static SPStyleEnum const enum_stroke_linejoin[] = {
    {"miter", SP_STROKE_LINEJOIN_MITER},
    {"round", SP_STROKE_LINEJOIN_ROUND},
    {"bevel", SP_STROKE_LINEJOIN_BEVEL},
    {NULL, -1}
};

static SPStyleEnum const enum_font_style[] = {
    {"normal", SP_CSS_FONT_STYLE_NORMAL},
    {"italic", SP_CSS_FONT_STYLE_ITALIC},
    {"oblique", SP_CSS_FONT_STYLE_OBLIQUE},
    {NULL, -1}
};

static SPStyleEnum const enum_font_size[] = {
    {"xx-small", SP_CSS_FONT_SIZE_XX_SMALL},
    {"x-small", SP_CSS_FONT_SIZE_X_SMALL},
    {"small", SP_CSS_FONT_SIZE_SMALL},
    {"medium", SP_CSS_FONT_SIZE_MEDIUM},
    {"large", SP_CSS_FONT_SIZE_LARGE},
    {"x-large", SP_CSS_FONT_SIZE_X_LARGE},
    {"xx-large", SP_CSS_FONT_SIZE_XX_LARGE},
    {"smaller", SP_CSS_FONT_SIZE_SMALLER},
    {"larger", SP_CSS_FONT_SIZE_LARGER},
    {NULL, -1}
};

static SPStyleEnum const enum_font_variant[] = {
    {"normal", SP_CSS_FONT_VARIANT_NORMAL},
    {"small-caps", SP_CSS_FONT_VARIANT_SMALL_CAPS},
    {NULL, -1}
};

static SPStyleEnum const enum_font_weight[] = {
    {"100", SP_CSS_FONT_WEIGHT_100},
    {"200", SP_CSS_FONT_WEIGHT_200},
    {"300", SP_CSS_FONT_WEIGHT_300},
    {"400", SP_CSS_FONT_WEIGHT_400},
    {"500", SP_CSS_FONT_WEIGHT_500},
    {"600", SP_CSS_FONT_WEIGHT_600},
    {"700", SP_CSS_FONT_WEIGHT_700},
    {"800", SP_CSS_FONT_WEIGHT_800},
    {"900", SP_CSS_FONT_WEIGHT_900},
    {"normal", SP_CSS_FONT_WEIGHT_NORMAL},
    {"bold", SP_CSS_FONT_WEIGHT_BOLD},
    {"lighter", SP_CSS_FONT_WEIGHT_LIGHTER},
    {"bolder", SP_CSS_FONT_WEIGHT_BOLDER},
    {NULL, -1}
};

static SPStyleEnum const enum_font_stretch[] = {
    {"ultra-condensed", SP_CSS_FONT_STRETCH_ULTRA_CONDENSED},
    {"extra-condensed", SP_CSS_FONT_STRETCH_EXTRA_CONDENSED},
    {"condensed", SP_CSS_FONT_STRETCH_CONDENSED},
    {"semi-condensed", SP_CSS_FONT_STRETCH_SEMI_CONDENSED},
    {"normal", SP_CSS_FONT_STRETCH_NORMAL},
    {"semi-expanded", SP_CSS_FONT_STRETCH_SEMI_EXPANDED},
    {"expanded", SP_CSS_FONT_STRETCH_EXPANDED},
    {"extra-expanded", SP_CSS_FONT_STRETCH_EXTRA_EXPANDED},
    {"ultra-expanded", SP_CSS_FONT_STRETCH_ULTRA_EXPANDED},
    {"narrower", SP_CSS_FONT_STRETCH_NARROWER},
    {"wider", SP_CSS_FONT_STRETCH_WIDER},
    {NULL, -1}
};

static SPStyleEnum const enum_font_variant_ligatures[] = {
    {"none",                         SP_CSS_FONT_VARIANT_LIGATURES_NONE},
    {"common-ligatures",             SP_CSS_FONT_VARIANT_LIGATURES_COMMON},
    {"discretionary-ligatures",      SP_CSS_FONT_VARIANT_LIGATURES_DISCRETIONARY},
    {"historical-ligatures",         SP_CSS_FONT_VARIANT_LIGATURES_HISTORICAL},
    {"contextual",                   SP_CSS_FONT_VARIANT_LIGATURES_CONTEXTUAL},
    {"no-common-ligatures",          SP_CSS_FONT_VARIANT_LIGATURES_NOCOMMON},
    {"no-discretionary-ligatures",   SP_CSS_FONT_VARIANT_LIGATURES_NODISCRETIONARY},
    {"no-historical-ligatures",      SP_CSS_FONT_VARIANT_LIGATURES_NOHISTORICAL},
    {"no-contextual",                SP_CSS_FONT_VARIANT_LIGATURES_NOCONTEXTUAL},
    {NULL, -1}
};
    
static SPStyleEnum const enum_font_variant_position[] = {
    {"normal", SP_CSS_FONT_VARIANT_POSITION_NORMAL},
    {"sub", SP_CSS_FONT_VARIANT_POSITION_SUB},
    {"super", SP_CSS_FONT_VARIANT_POSITION_SUPER},
    {NULL, -1}
};
    
static SPStyleEnum const enum_font_variant_caps[] = {
    {"normal", SP_CSS_FONT_VARIANT_CAPS_NORMAL},
    {"small-caps", SP_CSS_FONT_VARIANT_CAPS_SMALL},
    {"all-small-caps", SP_CSS_FONT_VARIANT_CAPS_ALL_SMALL},
    {"petite-caps", SP_CSS_FONT_VARIANT_CAPS_PETITE},
    {"all_petite-caps", SP_CSS_FONT_VARIANT_CAPS_ALL_PETITE},
    {"unicase", SP_CSS_FONT_VARIANT_CAPS_UNICASE},
    {"titling", SP_CSS_FONT_VARIANT_CAPS_TITLING},
    {NULL, -1}
};
    
static SPStyleEnum const enum_font_variant_numeric[] = {
    {"normal", SP_CSS_FONT_VARIANT_NUMERIC_NORMAL},
    {"lining-nums", SP_CSS_FONT_VARIANT_NUMERIC_LINING_NUMS},
    {"oldstyle-nums", SP_CSS_FONT_VARIANT_NUMERIC_OLDSTYLE_NUMS},
    {"proportional-nums", SP_CSS_FONT_VARIANT_NUMERIC_PROPORTIONAL_NUMS},
    {"tabular-nums", SP_CSS_FONT_VARIANT_NUMERIC_TABULAR_NUMS},
    {"diagonal-fractions", SP_CSS_FONT_VARIANT_NUMERIC_DIAGONAL_FRACTIONS},
    {"stacked-fractions", SP_CSS_FONT_VARIANT_NUMERIC_STACKED_FRACTIONS},
    {"ordinal", SP_CSS_FONT_VARIANT_NUMERIC_ORDINAL},
    {"slashed-zero", SP_CSS_FONT_VARIANT_NUMERIC_SLASHED_ZERO},
    {NULL, -1}
};
    
static SPStyleEnum const enum_font_variant_alternates[] = {
    {"normal", SP_CSS_FONT_VARIANT_ALTERNATES_NORMAL},
    {"historical-forms", SP_CSS_FONT_VARIANT_ALTERNATES_HISTORICAL_FORMS},
    {"stylistic", SP_CSS_FONT_VARIANT_ALTERNATES_STYLISTIC},
    {"styleset", SP_CSS_FONT_VARIANT_ALTERNATES_STYLESET},
    {"character_variant", SP_CSS_FONT_VARIANT_ALTERNATES_CHARACTER_VARIANT},
    {"swash", SP_CSS_FONT_VARIANT_ALTERNATES_SWASH},
    {"ornaments", SP_CSS_FONT_VARIANT_ALTERNATES_ORNAMENTS},
    {"annotation", SP_CSS_FONT_VARIANT_ALTERNATES_ANNOTATION},
    {NULL, -1}
};

static SPStyleEnum const enum_font_variant_east_asian[] = {
    {"normal", SP_CSS_FONT_VARIANT_EAST_ASIAN_NORMAL},
    {"jis78", SP_CSS_FONT_VARIANT_EAST_ASIAN_JIS78},
    {"jis83", SP_CSS_FONT_VARIANT_EAST_ASIAN_JIS83},
    {"jis90", SP_CSS_FONT_VARIANT_EAST_ASIAN_JIS90},
    {"jis04", SP_CSS_FONT_VARIANT_EAST_ASIAN_JIS04},
    {"simplified", SP_CSS_FONT_VARIANT_EAST_ASIAN_SIMPLIFIED},
    {"traditional", SP_CSS_FONT_VARIANT_EAST_ASIAN_TRADITIONAL},
    {"full-width", SP_CSS_FONT_VARIANT_EAST_ASIAN_FULL_WIDTH},
    {"proportional-width", SP_CSS_FONT_VARIANT_EAST_ASIAN_PROPORTIONAL_WIDTH},
    {"ruby", SP_CSS_FONT_VARIANT_EAST_ASIAN_RUBY},
    {NULL, -1}
};

static SPStyleEnum const enum_text_align[] = {
    {"start", SP_CSS_TEXT_ALIGN_START},
    {"end", SP_CSS_TEXT_ALIGN_END},
    {"left", SP_CSS_TEXT_ALIGN_LEFT},
    {"right", SP_CSS_TEXT_ALIGN_RIGHT},
    {"center", SP_CSS_TEXT_ALIGN_CENTER},
    {"justify", SP_CSS_TEXT_ALIGN_JUSTIFY},
    {NULL, -1}
};

static SPStyleEnum const enum_text_transform[] = {
    {"capitalize", SP_CSS_TEXT_TRANSFORM_CAPITALIZE},
    {"uppercase", SP_CSS_TEXT_TRANSFORM_UPPERCASE},
    {"lowercase", SP_CSS_TEXT_TRANSFORM_LOWERCASE},
    {"none", SP_CSS_TEXT_TRANSFORM_NONE},
    {NULL, -1}
};

static SPStyleEnum const enum_text_anchor[] = {
    {"start", SP_CSS_TEXT_ANCHOR_START},
    {"middle", SP_CSS_TEXT_ANCHOR_MIDDLE},
    {"end", SP_CSS_TEXT_ANCHOR_END},
    {NULL, -1}
};

static SPStyleEnum const enum_white_space[] = {
    {"normal",   SP_CSS_WHITE_SPACE_NORMAL },
    {"pre",      SP_CSS_WHITE_SPACE_PRE    },
    {"nowrap",   SP_CSS_WHITE_SPACE_NOWRAP },
    {"pre-wrap", SP_CSS_WHITE_SPACE_PREWRAP},
    {"pre-line", SP_CSS_WHITE_SPACE_PRELINE},
    {NULL, -1}
};

static SPStyleEnum const enum_direction[] = {
    {"ltr", SP_CSS_DIRECTION_LTR},
    {"rtl", SP_CSS_DIRECTION_RTL},
    {NULL, -1}
};

static SPStyleEnum const enum_block_progression[] = {
    {"tb", SP_CSS_BLOCK_PROGRESSION_TB},
    {"rl", SP_CSS_BLOCK_PROGRESSION_RL},
    {"lr", SP_CSS_BLOCK_PROGRESSION_LR},
    {NULL, -1}
};

static SPStyleEnum const enum_writing_mode[] = {
    /* Note that using the same enumerator for lr as lr-tb means we write as lr-tb even if the
     * input file said lr.  We prefer writing lr-tb on the grounds that the spec says the initial
     * value is lr-tb rather than lr.
     *
     * ECMA scripts may be surprised to find tb-rl in DOM if they set the attribute to rl, so
     * sharing enumerators for different strings may be a bug (once we support ecma script).
     */
    {"lr-tb", SP_CSS_WRITING_MODE_LR_TB},
    {"rl-tb", SP_CSS_WRITING_MODE_RL_TB},
    {"tb-rl", SP_CSS_WRITING_MODE_TB_RL},
    {"lr", SP_CSS_WRITING_MODE_LR_TB},
    {"rl", SP_CSS_WRITING_MODE_RL_TB},
    {"tb", SP_CSS_WRITING_MODE_TB_RL},
    {NULL, -1}
};

static SPStyleEnum const enum_baseline_shift[] = {
    {"baseline", SP_CSS_BASELINE_SHIFT_BASELINE},
    {"sub",      SP_CSS_BASELINE_SHIFT_SUB},
    {"super",    SP_CSS_BASELINE_SHIFT_SUPER},
    {NULL, -1}
};

static SPStyleEnum const enum_visibility[] = {
    {"hidden", SP_CSS_VISIBILITY_HIDDEN},
    {"collapse", SP_CSS_VISIBILITY_COLLAPSE},
    {"visible", SP_CSS_VISIBILITY_VISIBLE},
    {NULL, -1}
};

static SPStyleEnum const enum_overflow[] = {
    {"visible", SP_CSS_OVERFLOW_VISIBLE},
    {"hidden", SP_CSS_OVERFLOW_HIDDEN},
    {"scroll", SP_CSS_OVERFLOW_SCROLL},
    {"auto", SP_CSS_OVERFLOW_AUTO},
    {NULL, -1}
};

// CSS Compositing and Blending Level 1
static SPStyleEnum const enum_isolation[] = {
    {"auto",             SP_CSS_ISOLATION_AUTO},
    {"isolate",          SP_CSS_ISOLATION_ISOLATE},
    {NULL, -1}
};

static SPStyleEnum const enum_blend_mode[] = {
    {"normal",           SP_CSS_BLEND_NORMAL},
    {"multiply",         SP_CSS_BLEND_MULTIPLY},
    {"screen",           SP_CSS_BLEND_SCREEN},
    {"darken",           SP_CSS_BLEND_DARKEN},
    {"lighten",          SP_CSS_BLEND_LIGHTEN},
    {"overlay",          SP_CSS_BLEND_OVERLAY},
    {"color-dodge",      SP_CSS_BLEND_COLORDODGE},
    {"color-burn",       SP_CSS_BLEND_COLORBURN},
    {"hard-light",       SP_CSS_BLEND_HARDLIGHT},
    {"soft-light",       SP_CSS_BLEND_SOFTLIGHT},
    {"difference",       SP_CSS_BLEND_DIFFERENCE},
    {"exclusion",        SP_CSS_BLEND_EXCLUSION},
    {"hue",              SP_CSS_BLEND_HUE},
    {"saturation",       SP_CSS_BLEND_SATURATION},
    {"color",            SP_CSS_BLEND_COLOR},
    {"luminosity",       SP_CSS_BLEND_LUMINOSITY},
    {NULL, -1}
};

static SPStyleEnum const enum_display[] = {
    {"none",      SP_CSS_DISPLAY_NONE},
    {"inline",    SP_CSS_DISPLAY_INLINE},
    {"block",     SP_CSS_DISPLAY_BLOCK},
    {"list-item", SP_CSS_DISPLAY_LIST_ITEM},
    {"run-in",    SP_CSS_DISPLAY_RUN_IN},
    {"compact",   SP_CSS_DISPLAY_COMPACT},
    {"marker",    SP_CSS_DISPLAY_MARKER},
    {"table",     SP_CSS_DISPLAY_TABLE},
    {"inline-table",  SP_CSS_DISPLAY_INLINE_TABLE},
    {"table-row-group",    SP_CSS_DISPLAY_TABLE_ROW_GROUP},
    {"table-header-group", SP_CSS_DISPLAY_TABLE_HEADER_GROUP},
    {"table-footer-group", SP_CSS_DISPLAY_TABLE_FOOTER_GROUP},
    {"table-row",     SP_CSS_DISPLAY_TABLE_ROW},
    {"table-column-group", SP_CSS_DISPLAY_TABLE_COLUMN_GROUP},
    {"table-column",  SP_CSS_DISPLAY_TABLE_COLUMN},
    {"table-cell",    SP_CSS_DISPLAY_TABLE_CELL},
    {"table-caption", SP_CSS_DISPLAY_TABLE_CAPTION},
    {NULL, -1}
};

static SPStyleEnum const enum_shape_rendering[] = {
    {"auto",                SP_CSS_SHAPE_RENDERING_AUTO},
    {"optimizeSpeed",       SP_CSS_SHAPE_RENDERING_OPTIMIZESPEED},
    {"crispEdges",          SP_CSS_SHAPE_RENDERING_CRISPEDGES},
    {"geometricPrecision",  SP_CSS_SHAPE_RENDERING_GEOMETRICPRECISION},
    {NULL, -1}
};

static SPStyleEnum const enum_color_rendering[] = {
    {"auto",            SP_CSS_COLOR_RENDERING_AUTO},
    {"optimizeSpeed",   SP_CSS_COLOR_RENDERING_OPTIMIZESPEED},
    {"optimizeQuality", SP_CSS_COLOR_RENDERING_OPTIMIZEQUALITY},
    {NULL, -1}
};

static SPStyleEnum const enum_image_rendering[] = {
    {"auto",                  SP_CSS_IMAGE_RENDERING_AUTO},
    {"optimizeSpeed",         SP_CSS_IMAGE_RENDERING_OPTIMIZESPEED},
    {"optimizeQuality",       SP_CSS_IMAGE_RENDERING_OPTIMIZEQUALITY},
    {"-inkscape-crisp-edges", SP_CSS_IMAGE_RENDERING_CRISPEDGES},
    {"-inkscape-pixelated",   SP_CSS_IMAGE_RENDERING_PIXELATED},
    {NULL, -1}
};

static SPStyleEnum const enum_text_rendering[] = {
    {"auto",               SP_CSS_TEXT_RENDERING_AUTO},
    {"optimizeSpeed",      SP_CSS_TEXT_RENDERING_OPTIMIZESPEED},
    {"optimizeLegibility", SP_CSS_TEXT_RENDERING_OPTIMIZELEGIBILITY},
    {"geometricPrecision", SP_CSS_TEXT_RENDERING_GEOMETRICPRECISION},
    {NULL, -1}
};

static SPStyleEnum const enum_enable_background[] = {
    {"accumulate", SP_CSS_BACKGROUND_ACCUMULATE},
    {"new", SP_CSS_BACKGROUND_NEW},
    {NULL, -1}
};

static SPStyleEnum const enum_clip_rule[] = {
    {"nonzero", SP_WIND_RULE_NONZERO},
    {"evenodd", SP_WIND_RULE_EVENODD},
    {NULL, -1}
};

static SPStyleEnum const enum_color_interpolation[] = {
    {"auto", SP_CSS_COLOR_INTERPOLATION_AUTO},
    {"sRGB", SP_CSS_COLOR_INTERPOLATION_SRGB},
    {"linearRGB", SP_CSS_COLOR_INTERPOLATION_LINEARRGB},
    {NULL, -1}
};


#endif // SEEN_SP_STYLE_ENUMS_H


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
